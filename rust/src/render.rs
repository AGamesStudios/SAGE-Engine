use libc::{c_uint};

#[no_mangle]
pub extern "C" fn sage_render_init(_width: c_uint, _height: c_uint) -> i32 {
    0
}

#[no_mangle]
pub extern "C" fn sage_render_frame() {}

#[no_mangle]
pub extern "C" fn sage_render_shutdown() {}
