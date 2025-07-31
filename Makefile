build-rust:
cargo build --manifest-path=rust/Cargo.toml --release
mkdir -p sage_engine/native
cp rust/target/release/libsagegfx.* sage_engine/native/ 2>/dev/null || true
