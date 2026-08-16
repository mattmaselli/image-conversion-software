#include <stdio.h>
#include <stdlib.h>
#include <MagickWand/MagickWand.h>

int convertToTIFF(const char *input, int quality) {
    
    const char *output = "output.tiff"; // change to your output filename

    MagickWandGenesis();
    MagickWand *wand = NewMagickWand();

    // Read input image
    if (MagickReadImage(wand, input) == MagickFalse) {
        fprintf(stderr, "Error: Failed to read image '%s'\n", input);
        wand = DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    // If the image has alpha, keep it; if not, remove it
    MagickBooleanType hasAlpha = MagickGetImageAlphaChannel(wand);
    if (hasAlpha == MagickTrue) {
        MagickSetImageAlphaChannel(wand, ActivateAlphaChannel);
        MagickSetOption(wand, "tiff:alpha", "unassociated");
    } else {
        MagickSetImageAlphaChannel(wand, RemoveAlphaChannel);
    }

    // Set format to TIFF~
    if (MagickSetImageFormat(wand, "TIFF") == MagickFalse) {~
        fprintf(stderr, "Error: Failed to set output format to TIFF\n");
        wand = DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    // Set TIFF quality
    MagickSetImageCompressionQuality(wand, quality);

    // Write output TIFF
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