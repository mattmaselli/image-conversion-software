#include <stdio.h>
#include <stdlib.h>
#include <MagickWand/MagickWand.h>

int convertToPNG(const char *input, int quality) {
    
    const char *output = "output.png";

    MagickWandGenesis();
    MagickWand *wand = NewMagickWand();

    // Read input image
    if (MagickReadImage(wand, input) == MagickFalse) {
        fprintf(stderr, "Error: Failed to read image '%s'\n", input);
        wand = DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    // PNG supports alpha, so keep it if present
    if (MagickGetImageAlphaChannel(wand) == MagickTrue) {
        MagickSetImageAlphaChannel(wand, ActivateAlphaChannel);
    }

    // Set output format to PNG
    if (MagickSetImageFormat(wand, "PNG") == MagickFalse) {
        fprintf(stderr, "Error: Failed to set output format to PNG\n");
        wand = DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    // Set compression quality (0–100)
    MagickSetImageCompressionQuality(wand, quality);

    // Write output image
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
