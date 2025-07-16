use std::ffi::{CStr, c_char};
use std::fs::OpenOptions;
use std::io::{Read, Write};
use std::path::Path;

use memmap2::{MmapMut, MmapOptions};
use xz2::read::XzDecoder;
use xz2::write::XzEncoder;

const TILE_SIZE: usize = 1024;

#[repr(C)]
pub struct FeatherCore {
    _private: u8,
}

impl FeatherCore {
    pub fn new() -> Self {
        FeatherCore { _private: 0 }
    }
}

#[no_mangle]
pub extern "C" fn fc_create() -> *mut FeatherCore {
    Box::into_raw(Box::new(FeatherCore::new()))
}

#[no_mangle]
pub extern "C" fn fc_destroy(ptr: *mut FeatherCore) {
    if !ptr.is_null() {
        unsafe { Box::from_raw(ptr); }
    }
}

struct Patch {
    offset: usize,
    old: Vec<u8>,
    new: Vec<u8>,
}

#[repr(C)]
pub struct ChronoPatchTree {
    mmap: MmapMut,
    patches: Vec<Vec<u8>>, // compressed patches
    cursor: usize,
}

impl ChronoPatchTree {
    pub fn open(path: &Path, size: usize) -> std::io::Result<Self> {
        let file = OpenOptions::new()
            .read(true)
            .write(true)
            .create(true)
            .open(path)?;
        file.set_len(size as u64)?;
        let mmap = unsafe { MmapOptions::new().len(size).map_mut(&file)? };
        Ok(ChronoPatchTree { mmap, patches: Vec::new(), cursor: 0 })
    }

    fn decode_patch(data: &[u8]) -> Patch {
        let mut decoder = XzDecoder::new(data);
        let mut buf = Vec::new();
        decoder.read_to_end(&mut buf).unwrap();
        let offset = usize::from_le_bytes(buf[0..8].try_into().unwrap());
        let old_len = usize::from_le_bytes(buf[8..16].try_into().unwrap());
        let new_len = usize::from_le_bytes(buf[16..24].try_into().unwrap());
        let old = buf[24..24 + old_len].to_vec();
        let new = buf[24 + old_len..24 + old_len + new_len].to_vec();
        Patch { offset, old, new }
    }

    fn encode_patch(patch: &Patch) -> Vec<u8> {
        let mut raw = Vec::with_capacity(24 + patch.old.len() + patch.new.len());
        raw.extend_from_slice(&(patch.offset as u64).to_le_bytes());
        raw.extend_from_slice(&(patch.old.len() as u64).to_le_bytes());
        raw.extend_from_slice(&(patch.new.len() as u64).to_le_bytes());
        raw.extend_from_slice(&patch.old);
        raw.extend_from_slice(&patch.new);
        let mut encoder = XzEncoder::new(Vec::new(), 6);
        encoder.write_all(&raw).unwrap();
        encoder.finish().unwrap()
    }

    pub fn write(&mut self, offset: usize, data: &[u8]) {
        if offset + data.len() > self.mmap.len() {
            return;
        }
        let old = self.mmap[offset..offset + data.len()].to_vec();
        self.mmap[offset..offset + data.len()].copy_from_slice(data);
        let patch = Patch { offset, old, new: data.to_vec() };
        if self.cursor < self.patches.len() {
            self.patches.truncate(self.cursor);
        }
        self.patches.push(Self::encode_patch(&patch));
        self.cursor += 1;
    }

    pub fn read(&self, offset: usize, buf: &mut [u8]) {
        if offset + buf.len() > self.mmap.len() {
            return;
        }
        buf.copy_from_slice(&self.mmap[offset..offset + buf.len()]);
    }

    pub fn apply_patch(&mut self) {
        if self.cursor >= self.patches.len() {
            return;
        }
        let patch = Self::decode_patch(&self.patches[self.cursor]);
        self.mmap[patch.offset..patch.offset + patch.new.len()].copy_from_slice(&patch.new);
        self.cursor += 1;
    }

    pub fn revert_patch(&mut self) {
        if self.cursor == 0 {
            return;
        }
        self.cursor -= 1;
        let patch = Self::decode_patch(&self.patches[self.cursor]);
        self.mmap[patch.offset..patch.offset + patch.old.len()].copy_from_slice(&patch.old);
    }
}

#[no_mangle]
pub extern "C" fn cpt_new(path: *const c_char, size: usize) -> *mut ChronoPatchTree {
    if path.is_null() {
        return std::ptr::null_mut();
    }
    let c_str = unsafe { CStr::from_ptr(path) };
    let path = Path::new(c_str.to_str().unwrap());
    match ChronoPatchTree::open(path, size) {
        Ok(tree) => Box::into_raw(Box::new(tree)),
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn cpt_write(ptr: *mut ChronoPatchTree, offset: usize, data: *const u8, len: usize) -> bool {
    if ptr.is_null() || data.is_null() {
        return false;
    }
    let slice = unsafe { std::slice::from_raw_parts(data, len) };
    unsafe { &mut *ptr }.write(offset, slice);
    true
}

#[no_mangle]
pub extern "C" fn cpt_read(ptr: *const ChronoPatchTree, offset: usize, data: *mut u8, len: usize) -> bool {
    if ptr.is_null() || data.is_null() {
        return false;
    }
    let slice = unsafe { std::slice::from_raw_parts_mut(data, len) };
    unsafe { &*ptr }.read(offset, slice);
    true
}

#[no_mangle]
pub extern "C" fn cpt_apply(ptr: *mut ChronoPatchTree) {
    if let Some(tree) = unsafe { ptr.as_mut() } {
        tree.apply_patch();
    }
}

#[no_mangle]
pub extern "C" fn cpt_revert(ptr: *mut ChronoPatchTree) {
    if let Some(tree) = unsafe { ptr.as_mut() } {
        tree.revert_patch();
    }
}

#[no_mangle]
pub extern "C" fn cpt_free(ptr: *mut ChronoPatchTree) {
    if !ptr.is_null() {
        unsafe { Box::from_raw(ptr); }
    }
}

