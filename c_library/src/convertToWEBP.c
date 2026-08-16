#include <stdio.h>
#include <stdlib.h>
#include <MagickWand/MagickWand.h>

int convertToWEBP(const char *input, int quality) {
    const char *output = "output.webp";

    MagickWandGenesis();
    MagickWand *wand = NewMagickWand();

    // Read input image
    if (MagickReadImage(wand, input) == MagickFalse) {
        fprintf(stderr, "Error: Failed to read image '%s'\n", input);
        wand = DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    // WEBP supports alpha — keep it if present
    if (MagickGetImageAlphaChannel(wand) == MagickTrue) {
        MagickSetImageAlphaChannel(wand, ActivateAlphaChannel);
    }

    // Set output format to WEBP
    if (MagickSetImageFormat(wand, "WEBP") == MagickFalse) {
        fprintf(stderr, "Error: Failed to set output format to WEBP\n");
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
