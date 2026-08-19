#include "ourconversionlib.h"

int main() {

    // Initialize MagickWand C API environment
    // Setting up internal resources, thread safety, and data structures
    MagickWandGenesis(); 

    convertToJPG("input.png", "output.jpg", 100);
    convertToPNG("input.png", 100);
    convertToTIFF("input.png", 100);
    convertToWEBP("input.png", 100);

    GIFInput input = makeGIFInput ((const char*[]){"input.png"}, 1, "output.gif", 100, -1, 500, 500);

    makeGIF(input);

    // Clean up and releases resources 
    MagickWandTerminus ();

    return 0;
}
