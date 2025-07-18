fn main() {
    println!("cargo:rerun-if-env-changed=PYO3_PYTHON_VERSION");
    if let Ok(ver) = std::env::var("PYO3_PYTHON_VERSION") {
        println!("cargo:warning=Using Python interpreter: {}", ver);
    }
    println!("cargo:warning=PyO3 version: 0.22");
    if std::env::var("PYO3_USE_ABI3_FORWARD_COMPATIBILITY").is_ok() {
        println!("cargo:warning=ABI3 forward compatibility enabled");
    }
}
