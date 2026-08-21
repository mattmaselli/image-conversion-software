/* 
convertToWEBP.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <MagickWand/MagickWand.h>

// (Note: * denotes ptr variable)
// (Note: %s in print statement denotes string, %s denotes decimal)
int convertToWEBP(const char *input, const char *output, int quality) {
    
    // Validate args
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
    // Create wand object for MagickWand use
    MagickWand *wand = NewMagickWand();

    // Read input image
    if (MagickReadImage(wand, input) == MagickFalse) {
        fprintf(stderr, "Error: Failed to read image '%s'\n", input);
        goto cleanup;
    }

    // Set output format to WEBP
    if (MagickSetImageFormat(wand, "WEBP") == MagickFalse) {
        fprintf(stderr, "Error: Failed to set output format to WEBP\n");
        goto cleanup;
    }

    // Set quality
    if (MagickSetImageCompressionQuality(wand, quality) == MagickFalse) {
        fprintf(stderr, "Error: Failed to set image quality\n");
        goto cleanup;
    }

    // Write output image
    if (MagickWriteImage(wand, output) == MagickFalse) {
        fprintf(stderr, "Error: Failed to write image '%s'\n", output);
        goto cleanup;
    }

    printf("Successfully converted '%s' → '%s' with quality %d\n", input, output, quality);

    // Mark conversion successful
    result = 0;

    // Release resources - destroy MW object and return result as 1.
    cleanup: 
        wand = DestroyMagickWand(wand);
        return result;
}
