use libc::c_int;

#[no_mangle]
pub extern "C" fn sage_is_visible(x0: c_int, y0: c_int, x1: c_int, y1: c_int,
                                    vx0: c_int, vy0: c_int, vx1: c_int, vy1: c_int) -> bool {
    !(x1 < vx0 || x0 > vx1 || y1 < vy0 || y0 > vy1)
}

#[no_mangle]
pub extern "C" fn sage_cull(boxes: *const c_int, count: usize,
                             viewport: *const c_int, out: *mut usize) -> usize {
    if boxes.is_null() || viewport.is_null() || out.is_null() { return 0; }
    let vp = unsafe { std::slice::from_raw_parts(viewport, 4) };
    let bx = unsafe { std::slice::from_raw_parts(boxes, count * 4) };
    let mut n = 0usize;
    for i in 0..count {
        let idx = i*4;
        let x0 = bx[idx];
        let y0 = bx[idx+1];
        let x1 = bx[idx+2];
        let y1 = bx[idx+3];
        if !(x1 < vp[0] || x0 > vp[2] || y1 < vp[1] || y0 > vp[3]) {
            unsafe { *out.add(n) = i; }
            n += 1;
        }
    }
    n
}
