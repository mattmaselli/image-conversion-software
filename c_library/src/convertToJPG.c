#include <stdio.h>
#include <stdlib.h>
#include <MagickWand/MagickWand.h>

int convertToJPG(const char *input, int quality) {
  
    const char *output = "output.jpg";  // Output filename

    MagickWandGenesis();
    MagickWand *wand = NewMagickWand();

    // Read input image
    if (MagickReadImage(wand, input) == MagickFalse) {
        fprintf(stderr, "Error: Failed to read image '%s'\n", input);
        wand = DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    // Remove alpha channel if present (JPEG does not support transparency)
    if (MagickGetImageAlphaChannel(wand) == MagickTrue) {
        MagickSetImageAlphaChannel(wand, RemoveAlphaChannel);
    }

    // Set format to JPEG
    if (MagickSetImageFormat(wand, "JPEG") == MagickFalse) {
        fprintf(stderr, "Error: Failed to set output format to JPEG\n");
        wand = DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    // Set JPEG quality
    MagickSetImageCompressionQuality(wand, quality);

    // Write output JPEG
    if (MagickWriteImage(wand, output) == MagickFalse) {
        fprintf(stderr, "Error: Failed to write image '%s'\n", output);
        wand = DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    printf("Successfully converted '%s' → '%s' with quality %d\n", input, output, quality);

    wand = DestroyMagickWand(wand);
    MagickWandTerminus();

    return 0;
}
