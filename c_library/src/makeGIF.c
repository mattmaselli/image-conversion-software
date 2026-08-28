/* 
makeGIF.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <MagickWand/MagickWand.h>
#include "ourconversionlib.h"

// Met
static void print_wand_error(const char *operation, MagickWand *wand) {

    ExceptionType severity;
    char *desc = MagickGetException(wand, &severity);

    fprintf(
        stderr,
        "ImageMagick error during %s: %s\n",
        operation,
        desc != NULL ? desc: "unknown error"
    );

    if (desc != NULL) {
        MagickRelinquishMemory(desc);
    }
}
    
static int calculate_fit_size(
    // size_t = unsigned integer able to hold largest possible size
    size_t input_width,
    size_t input_height,
    size_t target_width, 
    size_t target_height,
    // Pointers -> point to addresses where result should be stored
    size_t *output_width,
    size_t *output_height
)
{   // ensure valid args
    if (
        input_width == 0 || 
        input_height == 0 ||
        target_width == 0 || 
        target_height == 0 || 
        output_width == NULL || 
        output_height == NULL 
    ) {
        return 1;
    }

    // scale to FIT inside target while preserving aspect
    double width_scale = (double) target_width / (double) input_width;

    double height_scale = (double) target_height / (double) input_width;

    // condition ? value_if_true : value_if_false
    // Ensures that function selects smaller scale so the resized image fits entirely 
    // inside target dimensions without stretching or cropping.
    // Ex: if width_scale = 0.5 and height_scale = 0.75
    // Then scale should = 0.5 to guarantee both dimensions fit
    double scale = width_scale < height_scale ? width_scale : height_scale;

    // round to positive decimal to nearest whole number
    // before converting it to size_t.
    *output_width = (size_t) ((double) input_width * scale + 0.5);

    *output_height = (size_t)((double) input_height * scale + 0.5);

    // Prevent small images from being round down to nothing
    if (*output_width == 0) {
        *output_width = 1;
    }

    if (*output_height == 0) {
        *output_height = 1;
    }
    return 0;
}

int makeGIF (GIFInput input)
{
    int result = 1;
    MagickWand *animation = NULL;
    PixelWand *background = NULL; 
    size_t target_width = input.target_w;
    size_t target_height = input.target_h;

    // Validate args before allocating resources
    if (input.frames == NULL) 
    {
        fprintf(stderr, "Error: GIF frame array cannot be NULL.\n");
        return 1;
    }

    if (input.count == 0) 
    {
        fprintf(stderr, "Error: GIF must contain at least one frame.\n");
        return 1;
    }

    if (input.out_gif == NULL) 
    {
        fprintf(stderr, "Error: GIF output path cannot be NULL.\n");
        return 1;
    }

    if (input.delay_cs < 0) 
    {
        fprintf(stderr, "Error: GIF frame delay cannot be negative.\n");
        return 1;
    }

    if (input.loop < 0) 
    {
        fprintf(stderr, "Error: GIF loop count cannot be negative\n");
        return 1;
    }

    for (size_t i = 0; i < input.count; i++) 
    {
        if (input.frames[i] == NULL) {
            fprintf(stderr, "Error: GIF frame %zu is NULL.\n", i);
            return 1;
        }
    } 

    // Allocate resources used by animation
    animation = NewMagickWand();

    if (animation == NULL) 
    {
        fprintf(stderr, "Error: Failed to create animation wand\n");
        goto cleanup;
    }

    background = NewPixelWand();

    if (background == NULL) 
    {
        fprintf(stderr, "Error: Failed to create background PixelWand\n");
        goto cleanup;
    }

    if (PixelSetColor(background, "transparent") == MagickFalse) 
    {
        fprintf(stderr, "Error: Failed to set transparent background\n");
        goto cleanup;
    }

    // If either target dimension was omitted, inspect all frames 
    // and find largest missing dimension
    // (useful default for when user does not care to specify)
    // FOR LOOP START
    if (target_width == 0 || target_height == 0) 
    { 
        size_t max_width = 0;
        size_t max_height = 0;

        MagickWand *probe = NewMagickWand();

        if (probe == NULL) 
        { 
            fprintf(stderr, "Error: Failed to create probe wand\n");
            goto cleanup;
        }

        for (size_t i = 0; i< input.count; i++) 
        {
            if (MagickReadImage(probe, input.frames[i]) == MagickFalse)
            {
                print_wand_error("Reading GIF frame dimensions", probe);
                probe = DestroyMagickWand(probe);
                goto cleanup;
            }

            size_t width = MagickGetImageWidth(probe);
            size_t height = MagickGetImageHeight(probe);

            if (width == 0 || height == 0) 
            {
                fprintf(stderr, "Error: GIF frame %zu has invalid dimensions.\n", i);
                
                probe = DestroyMagickWand(probe);
                goto cleanup;
            }

            if (width > max_width) 
            {
                max_width = width;
            }

            if (height > max_height) 
            {
                max_height = height;
            }
            ClearMagickWand(probe);
        }

        probe = DestroyMagickWand(probe);

        if (target_width == 0) {
            target_width = max_width;
        }

        if (target_height == 0) {
            target_height = max_height;
        }

        if (target_width == 0 || target_height == 0) {
            fprintf(stderr, "Error: Could not determine GIF dimensions.\n");
            goto cleanup;
        }
    } // FOR LOOP END

    // Read, resize, and append each frame.
    for (size_t i = 0; i < input.count; i++) {
        MagickWand *frame = NewMagickWand();

        if (frame == NULL) 
        {
            fprintf(stderr, 
                "Error: Failed to create wand for GIF frame %zu.\n", 
                i);
            goto cleanup;
        }

        if (MagickReadImage(frame, input.frames[i]) == MagickFalse) {
            print_wand_error("Reading GIF frame", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }

        size_t input_width = MagickGetImageWidth(frame);
        size_t input_height = MagickGetImageHeight(frame);
        size_t resized_width;
        size_t resized_height;

        // Check if each frame passes calc_fit_size test
        // If it returns 1, then invalid input(s) exist(s)
        if (calculate_fit_size(
            input_width,
            input_height,
            target_width,
            target_height,
            &resized_width,
            &resized_height
        ) != 0 ) {
            fprintf(stderr, "Error: Invalid dimensions for GIF frame %zu.\n", i);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }
    
        if (resized_width != input_width || resized_height != input_height)
        {
            if (MagickResizeImage(frame, 
                resized_width, 
                resized_height, 
                LanczosFilter) == MagickFalse
            )
                {
                    print_wand_error("resizing GIF frame", frame);
                    frame = DestroyMagickWand(frame);
                    goto cleanup;
                }
        }
        if (MagickSetImageBackgroundColor(frame, background) == 
            MagickFalse)
        {
            print_wand_error("setting GIF background", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }
        
        if (MagickSetImageGravity(frame, CenterGravity) == 
            MagickFalse) 
        {
            print_wand_error("setting GIF frame gravity", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }

        // Gravity handles centering, so use offsets of zero.
        if (MagickExtentImage(
            frame,
            target_width,
            target_height,
            0,
            0 ) == MagickFalse) 
        {
            print_wand_error("Extending GIF frame canvas", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }

        if (MagickSetImageDelay(
            frame,
            (size_t) input.delay_cs) == MagickFalse)
        {
            print_wand_error("Setting GIF frame delay", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }
        if (MagickSetImageDispose(
            frame,
            NoneDispose) == MagickFalse)
        {
            print_wand_error("Setting GIF disposal method", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }

        if (MagickSetImageFormat(
            frame, 
            "GIF") == MagickFalse)
        {
            print_wand_error("Setting GIF frame format", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }

        if (MagickAddImage
            (
            animation,
            frame
            ) == MagickFalse
        )
        {
            print_wand_error("Adding GIF frame", animation);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }
        frame = DestroyMagickWand(frame);
    }

    if (MagickGetNumberImages(animation) == 0 )
    {
        fprintf(stderr, "Error: No frames were added to GIF\n");
        goto cleanup;
    }

    MagickSetFirstIterator(animation);
    
    // 0 means loop forever.
    char iterations[32];

    // snprintf - used to format and store a string into a memory buffer
    // format: snprintf(char *buffer, size_t n, const char *format, ...);
    // format - the string containing text and format specifiers followed by args
    // memory buffer - reserved block of temp storage space in memory
    // used to hold data while it moves from one place to another
    snprintf(iterations, sizeof(iterations), "%d", input.loop);

    if (MagickSetOption(
        animation,
        "gif:iterations",
        iterations) == MagickFalse)
    {
        print_wand_error("Setting GIF loop count", animation);
        goto cleanup;
    }

    // Leave optimization out until basic GIF path is fully tested
    // MagickOptimizeImageLayers returns a new wand rather than 
    // modifying animation in place.

    if (MagickWriteImages(
        animation,
        input.out_gif,
        MagickTrue) == MagickFalse)
    { 
        print_wand_error("Writing GIF output", animation);
        goto cleanup;
    }
    
    // End
    result = 0;
    
cleanup: 
    if (background != NULL) 
    {
        background = DestroyPixelWand(background);
    }

    if (animation != NULL) 
    {
        animation = DestroyMagickWand(animation);
    }

    return result;
    
}
        

//         }

//     )

//     }

// static void fit_scale(size_t in_w, size_t in_h,
//                       size_t tgt_w, size_t tgt_h,
//                       size_t *out_w, size_t *out_h)
// {
//     // scale to FIT inside target while preserving aspect (letterbox if needed)
//     double sx = (double)tgt_w / (double)in_w;
//     double sy = (double)tgt_h / (double)in_h;
//     double s  = (sx < sy) ? sx : sy;
//     if (s <= 0.0) { s = 1.0; }
//     *out_w = (size_t)((double)in_w * s + 0.5);
//     *out_h = (size_t)((double)in_h * s + 0.5);
// }

// //GIF Input Struct
// typedef struct GIFInput {

//     char const **frames;
//     size_t count;
//     const char *out_gif;
//     int delay_cs;
//     int loop;
//     size_t target_w;
//     size_t target_h;

// } GIFInput;

// //Function to generate a GIFInput Struct
// GIFInput makeGIFInput(const char **frames, size_t count,
//                           const char *out_gif,
//                           int delay_cs, int loop,
//                           size_t target_w, size_t target_h) 
                          
// {

//     GIFInput input;

//     input.frames = frames;
//     input.count = count;
//     input.out_gif = out_gif;
//     input.delay_cs = delay_cs;
//     input.loop = loop;
//     input.target_w = target_w;
//     input.target_h = target_h;

//     return input;

// }

// int makeGIF(GIFInput input)

// {
//     if (!input.frames || input.count == 0 || !input.out_gif) {
//         fprintf(stderr, "[make_gif] invalid args\n");
//         return 1;
//     }

//     int rc = 1;
//     MagickBooleanType ok = MagickTrue;
//     MagickWand *anim = NewMagickWand();
//     PixelWand *bg = NewPixelWand();
//     PixelSetColor(bg, "transparent"); // change to "#000" or similar if desired

//     size_t TW = input.target_w, TH = input.target_h;
//     if (TW == 0 || TH == 0) {
//         size_t maxW = 0, maxH = 0;
//         MagickWand *probe = NewMagickWand();
//         for (size_t i = 0; i < input.count; ++i) {
//             if (!input.frames[i]) continue;
//             if (MagickReadImage(probe, input.frames[i]) != MagickTrue) {
//                 print_wand_error("probe read", probe);
//                 ok = MagickFalse;
//                 break;
//             }
//             size_t w = MagickGetImageWidth(probe);
//             size_t h = MagickGetImageHeight(probe);
//             if (w > maxW) maxW = w;
//             if (h > maxH) maxH = h;
//             ClearMagickWand(probe);
//         }
//         DestroyMagickWand(probe);
//         if (!ok) { goto done; }

//         TW = (TW ? TW : maxW);
//         TH = (TH ? TH : maxH);
//         if (TW == 0 || TH == 0) {
//             fprintf(stderr, "[make_gif] could not determine target size\n");
//             goto done;
//         }
//     }

//     // Build frames
//     for (size_t i = 0; i < input.count; ++i) {
//         const char *path = input.frames[i];
//         if (!path) continue;

//         MagickWand *w = NewMagickWand();
//         if (MagickReadImage(w, path) != MagickTrue) {
//             print_wand_error("read", w);
//             DestroyMagickWand(w);
//             ok = MagickFalse;
//             break;
//         }

//         // Compute fit size
//         size_t in_w = MagickGetImageWidth(w);
//         size_t in_h = MagickGetImageHeight(w);
//         size_t rw=0, rh=0;
//         fit_scale(in_w, in_h, TW, TH, &rw, &rh);

//         // Resize if needed (Lanczos = good default)
//         if ((rw != in_w || rh != in_h) &&
//             MagickResizeImage(w, rw, rh, LanczosFilter) != MagickTrue) {
//             print_wand_error("resize", w);
//             DestroyMagickWand(w);
//             ok = MagickFalse;
//             break;
//         }

//         // Prepare background + gravity for extent
//         if (MagickSetImageBackgroundColor(w, bg) != MagickTrue) {
//             print_wand_error("set bg", w);
//             DestroyMagickWand(w);
//             ok = MagickFalse;
//             break;
//         }
//         if (MagickSetImageGravity(w, CenterGravity) != MagickTrue) {
//             print_wand_error("set gravity", w);
//             DestroyMagickWand(w);
//             ok = MagickFalse;
//             break;
//         }

//         // Center
//         ssize_t off_x = (ssize_t)((TW - MagickGetImageWidth(w)) / 2);
//         ssize_t off_y = (ssize_t)((TH - MagickGetImageHeight(w)) / 2);
//         if (MagickExtentImage(w, TW, TH, off_x, off_y) != MagickTrue) {
//             print_wand_error("extent", w);
//             DestroyMagickWand(w);
//             ok = MagickFalse;
//             break;
//         }

//         // Reset page 
//         if (MagickSetImagePage(w, TW, TH, 0, 0) != MagickTrue) {
//             print_wand_error("set page", w);
//             DestroyMagickWand(w);
//             ok = MagickFalse;
//             break;
//         }

//         // Per frame delay
//         if (MagickSetImageDelay(w, (size_t)input.delay_cs) != MagickTrue) {
//             print_wand_error("set delay", w);
//             DestroyMagickWand(w);
//             ok = MagickFalse;
//             break;
//         }
//         if (MagickSetImageDispose(w, NoneDispose) != MagickTrue) {
//             // With full canvases, NoneDispose is simplest
//             print_wand_error("set dispose", w);
//             DestroyMagickWand(w);
//             ok = MagickFalse;
//             break;
//         }

//         // Ensure GIF frame format
//         if (MagickSetImageFormat(w, "GIF") != MagickTrue) {
//             print_wand_error("frame format", w);
//             DestroyMagickWand(w);
//             ok = MagickFalse;
//             break;
//         }

//         if (MagickAddImage(anim, w) != MagickTrue) {
//             print_wand_error("add frame", anim);
//             DestroyMagickWand(w);
//             ok = MagickFalse;
//             break;
//         }
//         DestroyMagickWand(w);
//     }

//     if (!ok) goto done;

//     if (MagickGetNumberImages(anim) == 0) {
//         fprintf(stderr, "[make_gif] no frames added\n");
//         goto done;
//     }

//     MagickSetFirstIterator(anim);

//     // Loop count (0 = infinite)
//     char iters[32];
//     snprintf(iters, sizeof iters, "%d", input.loop);
//     if (MagickSetOption(anim, "gif:iterations", iters) != MagickTrue) {
//         print_wand_error("gif:iterations", anim);
//         goto done;
//     }

    
//     (void)MagickOptimizeImageLayers(anim);       // basic optimization
//     (void)MagickOptimizeImageTransparency(anim); // for transparency

//     if (MagickWriteImages(anim, input.out_gif, MagickTrue) != MagickTrue) {
//         print_wand_error("write", anim);
//         goto done;
//     }

//     rc = 0;

// done:
//     if (bg) DestroyPixelWand(bg);
//     if (anim) DestroyMagickWand(anim);
//     return rc;
// }