use pyo3::prelude::*;
use pyo3::types::{PyByteArray, PyBytes};

#[pyfunction]
fn merge_chunk_delta(py: Python<'_>, a: &[u8], b: &[u8]) -> Py<PyBytes> {
    let data = py.allow_threads(|| {
        let mut out = Vec::with_capacity(a.len() + b.len());
        out.extend_from_slice(a);
        out.extend_from_slice(b);
        out
    });
    PyBytes::new(py, &data).into()
}

#[pyfunction]
fn alloc_smart_slice(py: Python<'_>, count: usize) -> Py<PyByteArray> {
    let size = count * 1024;
    let data = py.allow_threads(|| vec![0u8; size]);
    PyByteArray::new(py, &data).into()
}

#[pymodule]
fn nano_core(_py: Python, m: &PyModule) -> PyResult<()> {
    pyo3::prepare_freethreaded_python();
    m.add_function(wrap_pyfunction!(merge_chunk_delta, m)?)?;
    m.add_function(wrap_pyfunction!(alloc_smart_slice, m)?)?;
    Ok(())
}
