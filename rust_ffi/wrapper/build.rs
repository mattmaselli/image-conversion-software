fn main() {
    // Tell Cargo where to find our static libraries
    println!("cargo:rustc-link-search=native=./lib");

    // Link your own library (ourconversionlib)
    println!("cargo:rustc-link-lib=static=ourconversionlib");

    // Link ImageMagick static libraries
    println!("cargo:rustc-link-lib=static=MagickWand-7.Q16HDRI");
    println!("cargo:rustc-link-lib=static=MagickCore-7.Q16HDRI");

    // Link image format static libraries
    println!("cargo:rustc-link-lib=static=png16");
    println!("cargo:rustc-link-lib=static=tiff");
    println!("cargo:rustc-link-lib=static=jpeg");
    println!("cargo:rustc-link-lib=static=webp");
    println!("cargo:rustc-link-lib=static=webpdemux");
    println!("cargo:rustc-link-lib=static=webpmux");
    println!("cargo:rustc-link-lib=static=sharpyuv");

    // Link system libraries
    println!("cargo:rustc-link-lib=static=z");

    // Optional: rerun build.rs if lib directory changes
    println!("cargo:rerun-if-changed=lib");
}
