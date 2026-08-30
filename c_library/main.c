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
        "frame2.jpg",
        "frame3.jpg"
    };

    GIFInput gif_input = {
        .frames = frames,
        .count = 4,
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

    // MAKE OWN GIF, GET USER INPUT:

    const char *user_frames;
    int user_count;
    char user_out_gif[11];
    int user_delay;
    int user_loop;
    int user_target_w;
    int user_target_h;

    printf("Please enter a count: " );
    scanf("%d", &user_count);

    printf("You entered: %d\n", user_count);

     // Clean up and releases resources 
    MagickWandTerminus ();

    return gif_result;
}
