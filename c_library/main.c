#include <MagickWand/MagickWand.h>
#include "ourconversionlib.h"
#include <stdio.h>

int main() {

    // Initialize MagickWand C API environment
    // Setting up internal resources, thread safety, and data structures
    MagickWandGenesis(); 

    convertToJPG("input.png", "output.jpg", 100);
    convertToPNG("input.png", "output.png", 100);
    convertToTIFF("input.png", "output.tiff", 100);
    convertToWEBP("input.png", "output.webp", 100);

    // Create GIF
    const char *frames[] =
    {
        "input.png",
        "frame1.jpg",
        "output.jpg"
    };

    GIFInput gif_input = {
        .frames = frames,
        .count = 3,
        .out_gif = "output.gif",
        .delay_cs = 500,
        .loop = 0,
        .target_w=0, 
        .target_h = 0
    };

    int gif_result = makeGIF(gif_input);

    if (gif_result != 0) 
    {
        fprintf(stderr, "GIF conversion failed.\n");
    }

    MagickWandTerminus ();

    return gif_result;
    //GIFInput input = makeGIFInput ((const char*[]){"input.png"}, 1, "output.gif", 100, -1, 500, 500);

    //makeGIF(input);

    // Clean up and releases resources 
    MagickWandTerminus ();

    return 0;
}
