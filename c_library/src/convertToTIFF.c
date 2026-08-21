/*
convertToTIFF.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <MagickWand/MagickWand.h>

int convertToTIFF(const char *input, const char *output, int quality) {
    
    // Validate arguments before allocating anything
    if (input == NULL || output == NULL) {
        fprintf(stderr, "Error: Input and output paths cannot be NULL\n");
        return 1;
    }

    if (quality < 1 || quality > 100) {
        fprintf(stderr, "Error: Quality must be between 1 and 100\n");
        return 1;
    }
    
    // Allocate resources
    int result = 1;
    MagickWand *wand = NewMagickWand();

    // Read input image
    if (MagickReadImage(wand, input) == MagickFalse) {
        fprintf(stderr, "Error: Failed to read image '%s'\n", input);
        goto cleanup;
    }

    // Set format to TIFF
    if (MagickSetImageFormat(wand, "TIFF") == MagickFalse) {
        fprintf(stderr, "Error: Failed to set output format to TIFF\n");
        goto cleanup;
    }

    // Set quality
    if (MagickSetImageCompressionQuality(wand, quality) == MagickFalse) {
        fprintf(stderr, "Error: Failed to set image quality\n");
        goto cleanup;
    }

    // Write output TIFF
    if (MagickWriteImage(wand, output) == MagickFalse) {
        fprintf(stderr, "Error: Failed to write image '%s'\n", output);
        goto cleanup;
    }

    printf("Successfully converted '%s' → '%s' with quality %d\n", input, output, quality);

    // Mark conversion successuful
    result = 0;

    // Release resources - destroy MW object and return result as 1.
    cleanup:
        wand = DestroyMagickWand(wand);
        return result;
}