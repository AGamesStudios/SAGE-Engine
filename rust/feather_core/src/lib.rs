use std::ffi::{CStr, c_char};
use std::fs::OpenOptions;
use std::io::{Read, Write};
use std::path::Path;

use memmap2::{MmapMut, MmapOptions};
use xz2::read::XzDecoder;
use xz2::write::XzEncoder;
use pyo3::prelude::*;

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

// --- MicroPython integration -----------------------------------------------

#[repr(C)]
pub struct MicroPy {
    _private: u8,
}

#[no_mangle]
pub extern "C" fn mp_new() -> *mut MicroPy {
    Box::into_raw(Box::new(MicroPy { _private: 0 }))
}

#[no_mangle]
pub extern "C" fn mp_free(ptr: *mut MicroPy) {
    if !ptr.is_null() {
        unsafe { Box::from_raw(ptr); }
    }
}

#[no_mangle]
pub extern "C" fn mp_exec(_ptr: *mut MicroPy, script: *const c_char) -> bool {
    if script.is_null() {
        return false;
    }
    let c_str = unsafe { CStr::from_ptr(script) };
    let code = match c_str.to_str() {
        Ok(s) => s,
        Err(_) => return false,
    };
    Python::with_gil(|py| py.run(code, None, None).is_ok())
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


// --- DAG Scheduler -------------------------------------------------------
use std::collections::{HashMap, VecDeque};
use std::ffi::c_void;
use std::thread;

type TaskFn = unsafe extern "C" fn(*mut c_void);

struct DagTask {
    func: TaskFn,
    data: *mut c_void,
    deps: Vec<usize>,
}

#[repr(C)]
pub struct DagScheduler {
    tasks: HashMap<usize, DagTask>,
}

impl DagScheduler {
    fn new() -> Self {
        DagScheduler { tasks: HashMap::new() }
    }

    fn add_task(&mut self, id: usize, func: TaskFn, data: *mut c_void) {
        self.tasks.insert(id, DagTask { func, data, deps: Vec::new() });
    }

    fn add_dep(&mut self, task: usize, depends_on: usize) {
        if let Some(t) = self.tasks.get_mut(&task) {
            t.deps.push(depends_on);
        }
    }

    fn execute(&self) -> bool {
        let mut indeg: HashMap<usize, usize> = HashMap::new();
        let mut adj: HashMap<usize, Vec<usize>> = HashMap::new();
        for (&id, task) in &self.tasks {
            indeg.entry(id).or_insert(0);
            for &d in &task.deps {
                *indeg.entry(id).or_insert(0) += 1;
                adj.entry(d).or_default().push(id);
            }
        }

        let mut ready: VecDeque<usize> = indeg
            .iter()
            .filter_map(|(&id, &deg)| if deg == 0 { Some(id) } else { None })
            .collect();
        let mut processed = 0usize;

        while !ready.is_empty() {
            let mut batch = Vec::new();
            for _ in 0..ready.len() {
                if let Some(id) = ready.pop_front() {
                    batch.push(id);
                }
            }
            for id in &batch {
                if let Some(task) = self.tasks.get(id) {
                    unsafe { (task.func)(task.data) };
                }
            }
            processed += batch.len();

            let mut next = Vec::new();
            for (&from, list) in &adj {
                if indeg.get(&from) == Some(&0) {
                    for &to in list {
                        if let Some(entry) = indeg.get_mut(&to) {
                            if *entry > 0 {
                                *entry -= 1;
                                if *entry == 0 {
                                    next.push(to);
                                }
                            }
                        }
                    }
                }
            }
            ready.extend(next);
        }

        processed == self.tasks.len()
    }
}

#[no_mangle]
pub extern "C" fn dag_new() -> *mut DagScheduler {
    Box::into_raw(Box::new(DagScheduler::new()))
}

#[no_mangle]
pub extern "C" fn dag_free(ptr: *mut DagScheduler) {
    if !ptr.is_null() {
        unsafe { Box::from_raw(ptr); }
    }
}

#[no_mangle]
pub extern "C" fn dag_add_task(ptr: *mut DagScheduler, id: usize, func: TaskFn, data: *mut c_void) {
    if let Some(s) = unsafe { ptr.as_mut() } {
        s.add_task(id, func, data);
    }
}

#[no_mangle]
pub extern "C" fn dag_add_dependency(ptr: *mut DagScheduler, task: usize, depends_on: usize) {
    if let Some(s) = unsafe { ptr.as_mut() } {
        s.add_dep(task, depends_on);
    }
}

#[no_mangle]
pub extern "C" fn dag_execute(ptr: *mut DagScheduler) -> bool {
    match unsafe { ptr.as_ref() } {
        Some(s) => s.execute(),
        None => false,
    }
}
