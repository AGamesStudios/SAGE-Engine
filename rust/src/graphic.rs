use libc::{c_int};

#[repr(C)]
pub struct SageColor { pub r: u8, pub g: u8, pub b: u8, pub a: u8 }

#[no_mangle]
pub extern "C" fn sage_draw_rect(_x: c_int, _y: c_int, _w: c_int, _h: c_int, _color: SageColor) {}
