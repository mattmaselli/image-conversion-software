#ifndef OURCONVERSIONLIB_H
#define OURCONVERSIONLIB_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int convertToJPG(const char *input, int quality);
int convertToPNG(const char *input, int quality);
int convertToTIFF(const char *input, int quality);
int convertToWEBP(const char *input, int quality);

typedef struct {
    const char **frames;
    size_t count;
    const char *out_gif;
    int delay_cs;
    int loop;
    size_t target_w;
    size_t target_h;
} GIFInput;

GIFInput makeGIFInput(const char **frames, size_t count,
                          const char *out_gif,
                          int delay_cs, int loop,
                          size_t target_w, size_t target_h);
                           
int makeGIF(GIFInput input);

#ifdef __cplusplus
}
#endif

#endif 