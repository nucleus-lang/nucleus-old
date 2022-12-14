#![crate_type="staticlib"]

#[no_mangle]
pub extern "C" fn HelloFromRust() -> f64
{
	println!("Hello! I'm Rust!\n");
	return 0.0;
}