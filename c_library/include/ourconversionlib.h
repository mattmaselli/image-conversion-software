/*
ourconversionlib.h
Declares all functions and structs.
*/

#ifndef OURCONVERSIONLIB_H
#define OURCONVERSIONLIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int convertToJPG(const char *input, const char *output, int quality);
int convertToPNG(const char *input, const char *output, int quality);
int convertToTIFF(const char *input, const char *output, int quality);
int convertToWEBP(const char *input, const char *output, int quality);

typedef struct GIFInput {
    // frames hold multiple file paths so it has two pointer levels 
    // (ptr to an array of other ptrs, pointing to file paths)
    // const char * keywords are to protect external string data ("file paths") from modification
    const char **frames;
    // how many frames
    size_t count;
    // output GIF (ptr)
    const char *out_gif;
    // Time each frame of GIF lasts for
    int delay_cs;
    // How many times a GIF loops (default is infinity but 
    // can be changed by user input)
    int loop;
    // A value of 0 for w or h automatically uses the largest
    // corresponding dimension among all input frames.
    size_t target_w;
    size_t target_h;
} GIFInput;
                     
int makeGIF(GIFInput input);

#ifdef __cplusplus
}
#endif

#endif 