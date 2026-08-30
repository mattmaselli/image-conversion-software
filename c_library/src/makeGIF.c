/* 
makeGIF.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <MagickWand/MagickWand.h>
#include "ourconversionlib.h"

// Method for diagnosing errors with MagickWand specifically
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
    
// Calc fill size for each gif
// Each frame may have differing dimension sizes so
// this functions ensures standardized fill size  
static int calculate_fill_size(
    // size_t is an unsigned integer type used for sizes and counts
    size_t input_width,
    size_t input_height,
    size_t target_width, 
    size_t target_height,
    // Pointers -> point to addresses where result should be stored
    size_t *output_width,
    size_t *output_height
)
{   // ensure dimensions are positive and output pointers are valid
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

    // scale to FILL inside target while preserving aspect
    double width_scale = (double) target_width / (double) input_width;
    double height_scale = (double) target_height / (double) input_height;

    // condition ? value_if_true : value_if_false
    // Ensures that function selects larger scale so the resized image fits entirely 
    // inside target dimensions - this way, no empty space will be utilized.
    // All frames of the GIF will fit in the dimensions, 
    // even if some have to be heavily cropped and centered.
    // Ex: if image = 800 x 400 and target = 400 x 400:
    // width_scale = 400 / 800 = .5
    // height_scale = 400 / 400 = 1.0
    // Then fill scale = 1.0 
    // and resized image = 800 x 400
    // Then the image covers the 400 x 400 canvas and the extra 400 pixels are 
    // center-cropped
    double scale = width_scale > height_scale ? width_scale : height_scale;

    // round to positive decimal to nearest whole number
    // before converting it to size_t.
    *output_width = (size_t) ((double) input_width * scale + 0.5);
    *output_height = (size_t)((double) input_height * scale + 0.5);

    return 0;
}

int makeGIF (GIFInput input)
{
    int result = 1;
    // animation will hold completed sequence
    MagickWand *animation = NULL;
    // background supplies transparent pixels when needed
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
    // and find smallest combo of dimensions
    // Useful default for when user does not care to specify
    // If specified, disregarded
    // FOR LOOP START
    if (target_width == 0 || target_height == 0) 
    { 
        size_t min_width = 0;
        size_t min_height = 0;
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

            // intialize with first frame dimensions
            if (i ==0) 
            {
                min_width = width;
                min_height = height;
            }
            else 
            {
                // Compare each later frame with the current minimums.
                if (width < min_width) 
                {
                    min_width = width;
                }
                
                if (height < min_height)
                {
                    min_height = height;
                }
            }
            ClearMagickWand(probe);
        }

        probe = DestroyMagickWand(probe);

        if (target_width == 0) {
            target_width = min_width;
        }

        if (target_height == 0) {
            target_height = min_height;
        }

        if (target_width == 0 || target_height == 0) {
            fprintf(stderr, "Error: Could not determine GIF dimensions.\n");
            goto cleanup;
        }
    } // FOR LOOP END

    // Read, resize, and append each frame at a time.
    for (size_t i = 0; i < input.count; i++) 
    {
        // Create frame wand
        MagickWand *frame = NewMagickWand();

        if (frame == NULL) 
        {
            fprintf(stderr, 
                "Error: Failed to create wand for GIF frame %zu.\n", 
                i);
            goto cleanup;
        }

        // Read image
        if (MagickReadImage(frame, input.frames[i]) == MagickFalse) {
            print_wand_error("Reading GIF frame", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }

        size_t input_width = MagickGetImageWidth(frame);
        size_t input_height = MagickGetImageHeight(frame);
        size_t resized_width;
        size_t resized_height;

        // Pass each frame through calc_fill_size
        // the input dimensions change with each frame, but target 
        // dimensions stay the same
        // EX: 
        // Frame 1: c_f_s(800, 600, 500, 400, ...)
        // Frame 2: c_f_s(500, 700, 500, 400, ...)
        // If it returns 1, then invalid input(s) exist(s)
        if (calculate_fill_size(
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
    
        // Resize image 
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
        // Set background if ImageMagick needs to add canvas pixels
        // With fill-and-crop, padding normally shouldn't be necessary
        if (MagickSetImageBackgroundColor(frame, background) == 
            MagickFalse)
        {
            print_wand_error("setting GIF background", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }
        // This tells ImageMagick to position image relative to its 
        // center
        if (MagickSetImageGravity(frame, CenterGravity) == 
            MagickFalse) 
        {
            print_wand_error("setting GIF frame gravity", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }
        // Make every frame the exact target width and target height,
        // center cropping the excess pixels
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
        // Sets how long the frame appears for. Measured in centiseconds,
        // each frame will have the same length.
        if (MagickSetImageDelay(
            frame,
            (size_t) input.delay_cs) == MagickFalse)
        {
            print_wand_error("Setting GIF frame delay", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }
        // How frame is to be disposed of afterbeing displayed.
        // NoneDispose means no special clearing operation, 
        // which is fine because this produces full-canvas frames of 
        // identical dimensions.
        if (MagickSetImageDispose(
            frame,
            NoneDispose) == MagickFalse)
        {
            print_wand_error("Setting GIF disposal method", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }
        // Set frame output format.
        if (MagickSetImageFormat(
            frame, 
            "GIF") == MagickFalse)
        {
            print_wand_error("Setting GIF frame format", frame);
            frame = DestroyMagickWand(frame);
            goto cleanup;
        }
        // Adds the finished frame to animation sequence.
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
        // Now that frame is added to the GIF, we are done with it.
        frame = DestroyMagickWand(frame);
    // FOR LOOP END
    }

    // Verify that at least one frame was added
    if (MagickGetNumberImages(animation) == 0 )
    {
        fprintf(stderr, "Error: No frames were added to GIF\n");
        goto cleanup;
    }
    
    // Move wand iterator to first frame.
    MagickSetFirstIterator(animation);
    char iterations[32];

    // snprintf - used to format and store a string into a memory buffer
    // format: snprintf(char *buffer, size_t n, const char *format, ...);
    // format - the string containing text and format specifiers followed by args
    // memory buffer - reserved block of temp storage space in memory
    // used to hold data while it moves from one place to another
    snprintf(iterations, sizeof(iterations), "%d", input.loop);

    // Set animation loop count.
    // 0 = loop forever
    // pos. value = # of iterations
    if (MagickSetOption(
        animation,
        "gif:iterations",
        iterations) == MagickFalse)
    {
        print_wand_error("Setting GIF loop count", animation);
        goto cleanup;
    }

    // Leaving optimization out until basic GIF path is fully tested.
    // MagickOptimizeImageLayers returns a new wand rather than 
    // modifying animation in place.

    // Write complete animation
    if (MagickWriteImages(
        animation,
        input.out_gif,
        MagickTrue) == MagickFalse)
    { 
        print_wand_error("Writing GIF output", animation);
        goto cleanup;
    }
    
    // Success, go to cleanup
    result = 0;

// Clean up and destroy wands!
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