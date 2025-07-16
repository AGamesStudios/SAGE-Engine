# ProfileFrame

The ProfileFrame collects simple timing information for core engine operations.
When the `profiling` feature is enabled in the Feather Core crate, each call to
MicroPython, ChronoPatchTree or the DAG scheduler prints the time spent in
milliseconds. This helps track down slow tasks during development.

Enable profiling by building the Rust library with:

```bash
cargo build --release --features profiling
```

Profiling output appears on stdout and can be disabled by omitting the feature.
