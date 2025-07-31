use libc::c_int;

#[inline]
fn blend_pixel(ptr: *mut u8, offset: usize, r: u8, g: u8, b: u8, a: u8) {
    unsafe {
        let dst = std::slice::from_raw_parts_mut(ptr.add(offset), 4);
        if a == 255 {
            dst[0] = b;
            dst[1] = g;
            dst[2] = r;
            dst[3] = 255;
        } else if a == 0 {
            return;
        } else {
            let db = dst[0] as u32;
            let dg = dst[1] as u32;
            let dr = dst[2] as u32;
            let da = dst[3] as u32;
            let inv = 255 - a as u32;
            dst[0] = (b as u32 + (db * inv / 255)) as u8;
            dst[1] = (g as u32 + (dg * inv / 255)) as u8;
            dst[2] = (r as u32 + (dr * inv / 255)) as u8;
            dst[3] = (a as u32 + (da * inv / 255)) as u8;
        }
    }
}

#[no_mangle]
pub extern "C" fn sage_clear(buf: *mut u8, width: c_int, height: c_int, color: u32) {
    if buf.is_null() { return; }
    let size = (width as usize) * (height as usize) * 4;
    let (b, g, r, a) = ((color & 0xFF) as u8,
                        ((color >> 8) & 0xFF) as u8,
                        ((color >> 16) & 0xFF) as u8,
                        ((color >> 24) & 0xFF) as u8);
    unsafe {
        for i in 0..size/4 {
            let off = i*4;
            blend_pixel(buf, off, r, g, b, a);
        }
    }
}

#[no_mangle]
pub extern "C" fn sage_draw_rect(buf: *mut u8, width: c_int, height: c_int, pitch: c_int,
                                  x: c_int, y: c_int, w: c_int, h: c_int, color: u32) {
    if buf.is_null() { return; }
    let (b, g, r, a) = ((color & 0xFF) as u8,
                        ((color >> 8) & 0xFF) as u8,
                        ((color >> 16) & 0xFF) as u8,
                        ((color >> 24) & 0xFF) as u8);
    let width = width as usize;
    let height = height as usize;
    let pitch = pitch as usize;
    for yy in y.max(0) as usize .. ((y + h).min(height as i32) as usize) {
        for xx in x.max(0) as usize .. ((x + w).min(width as i32) as usize) {
            let off = yy * pitch + xx * 4;
            blend_pixel(buf, off, r, g, b, a);
        }
    }
}
