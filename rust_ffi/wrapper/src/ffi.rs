use std::os::raw::{c_char, c_int};

#[repr(C)]
pub struct GIFInput {

    pub frames: *const *const c_char,
    pub count: usize,       
    pub out_gif: *const c_char,
    pub delay_cs: c_int,
    pub loop_count: c_int,
    pub target_w: usize,    
    pub target_h: usize,

}

unsafe extern "C" {

    pub unsafe fn convertToJPG(input: *const c_char, quality: c_int) -> c_int;
    pub unsafe fn convertToPNG(input: *const c_char, quality: c_int) -> c_int;
    pub unsafe fn convertToTIFF(input: *const c_char, quality: c_int) -> c_int;
    pub unsafe fn convertToWEBP(input: *const c_char, quality: c_int) -> c_int;

    pub unsafe fn makeGIFInput(
        frames: *const *const c_char,
        count: usize,          
        out_gif: *const c_char,
        delay_cs: c_int,
        loop_count: c_int,
        target_w: usize,       
        target_h: usize,       
    ) -> GIFInput;

    pub unsafe fn makeGIF(input: GIFInput) -> c_int;
    
}
