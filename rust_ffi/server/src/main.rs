// This file initializes and launches rocket server
use rocket::{
    data::ToByteUnit,
    http::{ContentType, Status},
    Data, Request, Response,
    response::Responder,
};

use std::{fs, io::Cursor, path::PathBuf};
use tempfile::NamedTempFile;

// Safe wrapper import of Rust functions
// (previous version called unsafe C functions)
use image_conversion_wrapper::{ convert_to_jpg, convert_to_png, 
    convert_to_tiff, convert_to_webp, 
    make_gif };

// responder for raw bytes
struct Binary(Vec<u8>, &'static str);
impl<'r> Responder<'r, 'static> for Binary {
    fn respond_to(self, _req: &'r Request<'_>) -> rocket::response::Result<'static> {
        Response::build()
            .raw_header("Content-Type", self.1)
            .sized_body(self.0.len(), Cursor::new(self.0))
            .ok()
    }
}

#[rocket::get("/health")]
fn health() -> &'static str { "ok" }

// Save request body to a temp file (read into bytes, then write).
async fn save_upload_to_tmp(data: Data<'_>) -> Result<PathBuf, Status> {
    let bytes = data.open(64.mebibytes())
        .into_bytes().await
        .map_err(|_| Status::BadRequest)?
        .into_inner();

    let tmp = NamedTempFile::new().map_err(|_| Status::InternalServerError)?;
    let path = tmp.into_temp_path();
    fs::write(&path, &bytes).map_err(|_| Status::InternalServerError)?;
    // keep() yields a stable PathBuf
    path.keep().map_err(|_| Status::InternalServerError)
}

// Helper for functions
fn call_single_in_out<F>(
    input_path: &PathBuf,
    out_ext: &str,
    quality: i32,
    fun: F,
    mime: &'static str,
) -> Result<Binary, Status>
where
    F: Fn(&str, i32) -> i32,
{
    let mut out_path = input_path.clone();
    out_path.set_extension(out_ext);

    let in_str = input_path.to_str().ok_or(Status::InternalServerError)?;
    let rc = fun(in_str, quality);
    if rc != 0 {
        return Err(Status::InternalServerError);
    }

    // Try to read Rocket’s expected temp output first
    let mut read_from = out_path.clone();
    if !read_from.exists() {
        read_from = std::path::PathBuf::from(format!("output.{}", out_ext));
    }

    // If nothing exists at either path, return 500
    if !read_from.exists() {
        eprintln!("Conversion failed: expected file {:?} not found", read_from);
        return Err(Status::InternalServerError);
    }

    // Read bytes and serve to browser
    let bytes = fs::read(&read_from).map_err(|_| Status::InternalServerError)?;
    Ok(Binary(bytes, mime))
}

// binary upload roots
#[rocket::post("/convert/jpg?<quality>", data = "<data>")]

async fn convert_jpg(quality: Option<u8>, data: Data<'_>) -> Result<Binary, Status> {
    let q = quality.unwrap_or(90) as i32;
    let in_path = save_upload_to_tmp(data).await?;
    call_single_in_out(&in_path, "jpg", q, convert_to_jpg, "image/jpeg")
}

#[rocket::post("/convert/png?<quality>", data = "<data>")]
async fn convert_png(quality: Option<u8>, data: Data<'_>) -> Result<Binary, Status> {
    let q = quality.unwrap_or(6) as i32;
    let in_path = save_upload_to_tmp(data).await?;
    call_single_in_out(&in_path, "png", q, convert_to_png, "image/png")
}

#[rocket::post("/convert/tiff?<quality>", data = "<data>")]
async fn convert_tiff(quality: Option<u8>, data: Data<'_>) -> Result<Binary, Status> {
    let q = quality.unwrap_or(90) as i32;
    let in_path = save_upload_to_tmp(data).await?;
    call_single_in_out(&in_path, "tiff", q, convert_to_tiff, "image/tiff")
}

#[rocket::post("/convert/webp?<quality>", data = "<data>")]
async fn convert_webp(quality: Option<u8>, data: Data<'_>) -> Result<Binary, Status> {
    let q = quality.unwrap_or(80) as i32;
    let in_path = save_upload_to_tmp(data).await?;
    call_single_in_out(&in_path, "webp", q, convert_to_webp, "image/webp")
}

// GIF via rocket multipart function
use rocket_multipart_form_data::{
    MultipartFormData, MultipartFormDataOptions, MultipartFormDataField,
    Repetition, RawField, TextField,
};

#[rocket::post("/gif", data = "<data>")]
async fn make_gif_route(content_type: &ContentType, data: Data<'_>) -> Result<Binary, Status> {
    // Declare fields
    let mut opts = MultipartFormDataOptions::new();
    opts.allowed_fields.push(
        MultipartFormDataField::raw("frames")
            .repetition(Repetition::infinite())
            .size_limit(64 * 1024 * 1024)
    );
    for name in ["delay_cs", "loop_count", "target_w", "target_h"] {
        opts.allowed_fields.push(MultipartFormDataField::text(name));
    }

    // Parse
    let mut form = MultipartFormData::parse(content_type, data, opts)
        .await
        .map_err(|_| Status::BadRequest)?;

    // Collect frames to disk
    let mut frame_paths: Vec<PathBuf> = Vec::new();
    if let Some(files) = form.raw.remove("frames") {
        for (i, f) in files.into_iter().enumerate() {
            let RawField { raw, .. } = f;
            let tmp = NamedTempFile::new().map_err(|_| Status::InternalServerError)?;
            let path = tmp.into_temp_path();
            let final_path = path.with_extension(format!("frame{i}.bin"));
            fs::write(&final_path, &raw).map_err(|_| Status::InternalServerError)?;
            frame_paths.push(final_path);
        }
    }
    if frame_paths.is_empty() {
        return Err(Status::BadRequest);
    }

    // pull single text field
    fn take_text(mut v: Option<Vec<TextField>>, default: &str) -> String {
        v.as_mut()
            .and_then(|vv| vv.pop())
            .map(|t| t.text)
            .unwrap_or_else(|| default.to_string())
    }

    let delay_cs  = take_text(form.texts.remove("delay_cs"),  "5").parse::<i32>().unwrap_or(5);
    let loop_cnt  = take_text(form.texts.remove("loop_count"),"0").parse::<i32>().unwrap_or(0);
    let target_w  = take_text(form.texts.remove("target_w"),  "0").parse::<usize>().unwrap_or(0);
    let target_h  = take_text(form.texts.remove("target_h"),  "0").parse::<usize>().unwrap_or(0);

    // Output path for the GIF
    let out_tmp = NamedTempFile::new().map_err(|_| Status::InternalServerError)?;
    let mut out_path = out_tmp.path().to_path_buf();
    out_path.set_extension("gif");
    let out_path = out_tmp.into_temp_path().keep().map_err(|_| Status::InternalServerError)?;

    // Safe for for GIF
    // Convert PathBuf to &str for make_gif_input
    let frame_str_paths: Vec<&str> = frame_paths
        .iter()
        .map(|p| p.to_str().unwrap())
        .collect();
    let out_gif_str = out_path.to_str().unwrap();

    // Create OwnedGIFInput
    let owned_gif_input = image_conversion_wrapper::make_gif_input(&frame_str_paths, out_gif_str, delay_cs, loop_cnt, target_w, target_h);

    //make_gif function only accepts one struct par
    if make_gif(owned_gif_input) != 0 {
        return Err(Status::InternalServerError);
    }

    // Return the produced GIF
    let bytes = fs::read(&PathBuf::from(&out_path)).map_err(|_| Status::InternalServerError)?;
    Ok(Binary(bytes, "image/gif"))
}

// Launches server
#[rocket::launch]
fn rocket() -> _ {
    rocket::build()
        .mount("/", rocket::routes![
            health,
            convert_jpg,
            convert_png,
            convert_tiff,
            convert_webp,
            make_gif_route,
        ])
        .mount("/", rocket::fs::FileServer::from("static"))
}

