use std::ffi::c_void;
use std::ptr::null_mut;

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

#[repr(C)]
pub struct ChronoPatchTree {
    _private: u8,
}

impl ChronoPatchTree {
    pub fn new() -> Self { ChronoPatchTree { _private: 0 } }
    pub fn apply_patch(&mut self) {}
    pub fn revert_patch(&mut self) {}
}

#[no_mangle]
pub extern "C" fn cpt_new() -> *mut ChronoPatchTree {
    Box::into_raw(Box::new(ChronoPatchTree::new()))
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
