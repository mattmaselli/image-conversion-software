pub mod ffi;
use ffi::*;
use std::ffi::CString;
use std::os::raw::{c_char, c_int};

pub fn convert_to_jpg(input: &str, quality: i32) -> i32 {
    let c_input = CString::new(input).unwrap();
    unsafe { convertToJPG(c_input.as_ptr(), quality as c_int) }
}

pub fn convert_to_png(input: &str, quality: i32) -> i32 {
    let c_input = CString::new(input).unwrap();
    unsafe { convertToPNG(c_input.as_ptr(), quality as c_int) }
}

pub fn convert_to_tiff(input: &str, quality: i32) -> i32 {
    let c_input = CString::new(input).unwrap();
    unsafe { convertToTIFF(c_input.as_ptr(), quality as c_int) }
}

pub fn convert_to_webp(input: &str, quality: i32) -> i32 {
    let c_input = CString::new(input).unwrap();
    unsafe { convertToWEBP(c_input.as_ptr(), quality as c_int) }
}


/// Owned wrapper that keeps all C memory alive while in scope.
pub struct OwnedGIFInput {
    pub gif_input: GIFInput,
    // keep these fields so their memory isn't freed
    _c_frames: Vec<CString>,
    _frame_ptrs: Vec<*const c_char>,
    _c_out_gif: CString,
}

pub fn make_gif_input(
    frames: &[&str],
    out_gif: &str,
    delay_cs: i32,
    loop_count: i32,
    width: usize,
    height: usize,
) -> OwnedGIFInput {
    
    // convert strings to CStrings
    let c_frames: Vec<CString> = frames.iter().map(|s| CString::new(*s).unwrap()).collect();

    // create array of pointers referring into c_frames
    let frame_ptrs: Vec<*const c_char> = c_frames.iter().map(|f| f.as_ptr()).collect();

    let c_out_gif = CString::new(out_gif).unwrap();

    // call C function to make GIFInput
    let gif_input = unsafe {
        makeGIFInput(
            frame_ptrs.as_ptr(),
            frame_ptrs.len(),
            c_out_gif.as_ptr(),
            delay_cs as c_int,
            loop_count as c_int,
            width,
            height,
        )
    };

    //return OwnedGIFInput (owned rust struct)
    OwnedGIFInput {
        gif_input,
        _c_frames: c_frames,
        _frame_ptrs: frame_ptrs,
        _c_out_gif: c_out_gif,
    }
}


pub fn make_gif(input: OwnedGIFInput) -> i32 {
    unsafe { makeGIF(input.gif_input) }
}

