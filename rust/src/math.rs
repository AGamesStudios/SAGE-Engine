use libc::c_int;

#[no_mangle]
pub extern "C" fn sage_q8_mul(a: c_int, b: c_int) -> c_int {
    ((a as i32 * b as i32) >> 8) as c_int
}

#[no_mangle]
pub extern "C" fn sage_q8_lerp(a: c_int, b: c_int, t: c_int) -> c_int {
    (((a as i32 * (255 - t as i32)) + (b as i32 * t as i32)) >> 8) as c_int
}

#[no_mangle]
pub extern "C" fn sage_blend_rgba_pm(dst: u32, src: u32) -> u32 {
    let sa = ((src >> 24) & 0xFF) as u32;
    let inv = 255 - sa;
    let r = (src & 0xFF) + (((dst & 0xFF) * inv) >> 8);
    let g = ((src >> 8) & 0xFF) + ((((dst >> 8) & 0xFF) * inv) >> 8);
    let b = ((src >> 16) & 0xFF) + ((((dst >> 16) & 0xFF) * inv) >> 8);
    let a = sa + (((dst >> 24) & 0xFF) * inv >> 8);
    (a << 24) | (b << 16) | (g << 8) | r
}
