build-rust:
        cargo build --manifest-path=rust/Cargo.toml --release
        mkdir -p sage_engine/native
        if [ -f rust/target/release/libsagegfx.dll ]; then cp rust/target/release/libsagegfx.dll sage_engine/native/; fi
        if [ -f rust/target/release/libsagegfx.so ];  then cp rust/target/release/libsagegfx.so  sage_engine/native/;  fi
        if [ -f rust/target/release/libsagegfx.dylib ]; then cp rust/target/release/libsagegfx.dylib sage_engine/native/; fi
