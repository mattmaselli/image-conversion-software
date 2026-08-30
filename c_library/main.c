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
    int user_exit = 0;

    // Priming read
    printf("GIF CREATION\nType 1 to continue, 2 to exit: ");
    scanf("%d", &user_exit);
    while (user_exit != 2)
    {


    if (user_exit != 1) 
    {
        printf("Try again: ");
        scanf("%d", &user_exit);
    }
    
    printf("How many frames do you want to use? (Limit 10) Enter a count (int): ");
    
    scanf("%d", &user_count);

    if (user_count <= 0 || user_count >= 10) 
    {
        printf("Invalid input, try again: ");
        scanf("%d", &user_count);
    }

    for (size_t i = 0; i < user_count; i++)
    {
        printf("Hello");
    }
    } 

    printf("Session terminated.\n");
    
     // Clean up and releases resources 
    MagickWandTerminus ();

    return gif_result;
}
