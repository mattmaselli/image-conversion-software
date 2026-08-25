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
    const char **frames;
    size_t count;
    const char *out_gif;
    int delay_cs;
    int loop;
    size_t target_w;
    size_t target_h;
} GIFInput;
                           
int makeGIF(const GIFInput *input);

#ifdef __cplusplus
}
#endif

#endif 