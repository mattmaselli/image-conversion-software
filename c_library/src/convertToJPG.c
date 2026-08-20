/*
convertToJPG.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <MagickWand/MagickWand.h>

// (Note: * denotes ptr variable)
// (Note: %s in print statement denotes string, %s denotes decimal)
int convertToJPG(const char *input, const char *output, int quality) {
  
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
    // Create wand object for MagickWand use
    MagickWand *wand = NewMagickWand();

    // If wand fails to be created, print output and return 1
    if (wand == NULL) {
        fprintf(stderr, "Error: Could not create MagickWand\n");
        return 1;
    }

    // Read input image
    if (MagickReadImage(wand, input) == MagickFalse) {
        fprintf(stderr, "Error: Failed to read '%s'\n", input);
        goto cleanup;
    }

    // Set format 
    // Note: MagickWand oficially prefers JPEG as far as terminology goes(source: https://imagemagick.org/formats/#gsc.tab=0)
    // However, .jpg is still accepted. 
    if (MagickSetImageFormat(wand, "JPEG") == MagickFalse) {
        fprintf(stderr, "Error: Failed to set JPEG format\n");
        goto cleanup;
    }

    // Set quality
    if (MagickSetImageCompressionQuality(wand, quality) == MagickFalse) {
        fprintf(stderr, "Error: Failed to set image quality\n");
        goto cleanup;
    }

    // remove alpha channel to remove transparency from image 
    /*if (MagickGetImageAlphaChannel(wand) == MagickTrue) {
        if (MagickSetImageAlphaChannel(wand, RemoveAlphaChannel) == MagickFalse) {
        fprintf(stderr, "Error: Failed to remove alpha channel\n");
        goto cleanup;
        }   

    }*/

    // Write output image
    if (MagickWriteImage(wand, output) == MagickFalse) {
        fprintf(stderr, "Error: Failed to write image '%s'\n", output);
        goto cleanup;
    }

    printf("Successfully converted '%s' → '%s' with quality %d\n", input, output, quality);

    // Mark conversion successful
    result = 0;

    // Release resources
    cleanup:
        wand = DestroyMagickWand(wand);
        return result;
}
