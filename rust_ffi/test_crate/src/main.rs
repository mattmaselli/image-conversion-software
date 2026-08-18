use image_conversion_wrapper::*;

fn main() {

    convert_to_jpg("input.png", 100);
    convert_to_png("input.png", 100);
    convert_to_tiff("input.png", 100);
    convert_to_webp("input.png", 1);

    
    //let gif_input = make_gif_input(&["input.png"], "output.gif", 100, -1, 500, 500);
    // Inputs: frames, output, delay time, loop, res height, res width
    let gif_input = make_gif_input(&["frame1.jpg", "frame2.jpg","frame3.jpg"], "output.gif", 100, -1, 500, 500);
    
    make_gif(gif_input);

}