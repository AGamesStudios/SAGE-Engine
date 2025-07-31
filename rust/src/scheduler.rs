use std::os::raw::c_float;

pub struct Scheduler {
    history: Vec<f32>,
    max_len: usize,
}

#[no_mangle]
pub extern "C" fn sage_sched_new(history: usize) -> *mut Scheduler {
    Box::into_raw(Box::new(Scheduler { history: Vec::new(), max_len: history }))
}

#[no_mangle]
pub extern "C" fn sage_sched_drop(ptr: *mut Scheduler) {
    if !ptr.is_null() { unsafe { Box::from_raw(ptr); } }
}

#[no_mangle]
pub extern "C" fn sage_sched_record(ptr: *mut Scheduler, frame_time: c_float) {
    if ptr.is_null() { return; }
    let sch = unsafe { &mut *ptr };
    if sch.history.len() == sch.max_len { sch.history.remove(0); }
    sch.history.push(frame_time);
}

#[no_mangle]
pub extern "C" fn sage_sched_should_defer(ptr: *const Scheduler, budget_ms: c_float) -> bool {
    if ptr.is_null() { return false; }
    let sch = unsafe { &*ptr };
    if sch.history.is_empty() { return false; }
    let avg: f32 = sch.history.iter().sum::<f32>() / sch.history.len() as f32;
    avg > budget_ms / 1000.0
}
