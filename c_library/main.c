#include <MagickWand/MagickWand.h>
#include "ourconversionlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

 // readInt helper function for processing ints and discarding chars
int readInt(int* value)
{
    char line[128], *end;

    while  (fgets(line, sizeof line, stdin))
    {
        // Reject and discard lines too long for buffer
        if (!strchr(line, '\n') && !feof(stdin)) 
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
        }
        else 
        {
            errno = 0;
            long n = strtol(line, &end, 10);

            if (end != line && errno != ERANGE && 
            n >= INT_MIN && n <= INT_MAX &&
            end[strspn(end, " \t\r\n\v\f")] == '\0')  
            {
                *value = (int)n;
                return 1;
            }
        }
        printf("Invalid integer. Try again: ");
        fflush(stdout);
    }
    return 0;
}

int main() 
{

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
    int c;

    // Priming read
    printf("GIF CREATION\nType 1 to continue, 2 to exit: ");
    scanf("%d", &user_exit);

    while (user_exit !=1 && user_exit !=2) 
    {
        int c;
        while ((c=getchar()) != '\n' && c != EOF);
        printf("Invalid input, try again. ");
        scanf("%d", &user_exit);
    }
    while (user_exit != 2)
    {

    int c;
    while ((c=getchar()) != '\n' && c != EOF);
    printf("How many frames do you want to use? (Limit 10) Enter a count (int): ");
    scanf("%d", &user_count);

    while (user_count <= 0 || user_count > 10) 
    {
        int c;
        while ((c=getchar()) != '\n' && c != EOF);
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
