#include <stdio.h>
#include <stdlib.h>
#include <MagickWand/MagickWand.h>

static void print_wand_error(const char *where, MagickWand *w) {
    ExceptionType et;
    char *desc = MagickGetException(w, &et);
    if (desc) {
        fprintf(stderr, "[%s] Magick error (%d): %s\n", where, et, desc);
        MagickRelinquishMemory(desc);
    } else {
        fprintf(stderr, "[%s] (no exception text)\n", where);
    }
}

static void fit_scale(size_t in_w, size_t in_h,
                      size_t tgt_w, size_t tgt_h,
                      size_t *out_w, size_t *out_h)
{
    // scale to FIT inside target while preserving aspect (letterbox if needed)
    double sx = (double)tgt_w / (double)in_w;
    double sy = (double)tgt_h / (double)in_h;
    double s  = (sx < sy) ? sx : sy;
    if (s <= 0.0) { s = 1.0; }
    *out_w = (size_t)((double)in_w * s + 0.5);
    *out_h = (size_t)((double)in_h * s + 0.5);
}

//GIF Input Struct
typedef struct GIFInput {

    char const **frames;
    size_t count;
    const char *out_gif;
    int delay_cs;
    int loop;
    size_t target_w;
    size_t target_h;

} GIFInput;

//Function to generate a GIFInput Struct
GIFInput makeGIFInput(const char **frames, size_t count,
                          const char *out_gif,
                          int delay_cs, int loop,
                          size_t target_w, size_t target_h) 
                          
{

    GIFInput input;

    input.frames = frames;
    input.count = count;
    input.out_gif = out_gif;
    input.delay_cs = delay_cs;
    input.loop = loop;
    input.target_w = target_w;
    input.target_h = target_h;

    return input;

}

int makeGIF(GIFInput input)

{
    if (!input.frames || input.count == 0 || !input.out_gif) {
        fprintf(stderr, "[make_gif] invalid args\n");
        return 1;
    }

    int rc = 1;
    MagickBooleanType ok = MagickTrue;
    MagickWand *anim = NewMagickWand();
    PixelWand *bg = NewPixelWand();
    PixelSetColor(bg, "transparent"); // change to "#000" or similar if desired

    size_t TW = input.target_w, TH = input.target_h;
    if (TW == 0 || TH == 0) {
        size_t maxW = 0, maxH = 0;
        MagickWand *probe = NewMagickWand();
        for (size_t i = 0; i < input.count; ++i) {
            if (!input.frames[i]) continue;
            if (MagickReadImage(probe, input.frames[i]) != MagickTrue) {
                print_wand_error("probe read", probe);
                ok = MagickFalse;
                break;
            }
            size_t w = MagickGetImageWidth(probe);
            size_t h = MagickGetImageHeight(probe);
            if (w > maxW) maxW = w;
            if (h > maxH) maxH = h;
            ClearMagickWand(probe);
        }
        DestroyMagickWand(probe);
        if (!ok) { goto done; }

        TW = (TW ? TW : maxW);
        TH = (TH ? TH : maxH);
        if (TW == 0 || TH == 0) {
            fprintf(stderr, "[make_gif] could not determine target size\n");
            goto done;
        }
    }

    // Build frames
    for (size_t i = 0; i < input.count; ++i) {
        const char *path = input.frames[i];
        if (!path) continue;

        MagickWand *w = NewMagickWand();
        if (MagickReadImage(w, path) != MagickTrue) {
            print_wand_error("read", w);
            DestroyMagickWand(w);
            ok = MagickFalse;
            break;
        }

        // Compute fit size
        size_t in_w = MagickGetImageWidth(w);
        size_t in_h = MagickGetImageHeight(w);
        size_t rw=0, rh=0;
        fit_scale(in_w, in_h, TW, TH, &rw, &rh);

        // Resize if needed (Lanczos = good default)
        if ((rw != in_w || rh != in_h) &&
            MagickResizeImage(w, rw, rh, LanczosFilter) != MagickTrue) {
            print_wand_error("resize", w);
            DestroyMagickWand(w);
            ok = MagickFalse;
            break;
        }

        // Prepare background + gravity for extent
        if (MagickSetImageBackgroundColor(w, bg) != MagickTrue) {
            print_wand_error("set bg", w);
            DestroyMagickWand(w);
            ok = MagickFalse;
            break;
        }
        if (MagickSetImageGravity(w, CenterGravity) != MagickTrue) {
            print_wand_error("set gravity", w);
            DestroyMagickWand(w);
            ok = MagickFalse;
            break;
        }

        // Center
        ssize_t off_x = (ssize_t)((TW - MagickGetImageWidth(w)) / 2);
        ssize_t off_y = (ssize_t)((TH - MagickGetImageHeight(w)) / 2);
        if (MagickExtentImage(w, TW, TH, off_x, off_y) != MagickTrue) {
            print_wand_error("extent", w);
            DestroyMagickWand(w);
            ok = MagickFalse;
            break;
        }

        // Reset page 
        if (MagickSetImagePage(w, TW, TH, 0, 0) != MagickTrue) {
            print_wand_error("set page", w);
            DestroyMagickWand(w);
            ok = MagickFalse;
            break;
        }

        // Per frame delay
        if (MagickSetImageDelay(w, (size_t)input.delay_cs) != MagickTrue) {
            print_wand_error("set delay", w);
            DestroyMagickWand(w);
            ok = MagickFalse;
            break;
        }
        if (MagickSetImageDispose(w, NoneDispose) != MagickTrue) {
            // With full canvases, NoneDispose is simplest
            print_wand_error("set dispose", w);
            DestroyMagickWand(w);
            ok = MagickFalse;
            break;
        }

        // Ensure GIF frame format
        if (MagickSetImageFormat(w, "GIF") != MagickTrue) {
            print_wand_error("frame format", w);
            DestroyMagickWand(w);
            ok = MagickFalse;
            break;
        }

        if (MagickAddImage(anim, w) != MagickTrue) {
            print_wand_error("add frame", anim);
            DestroyMagickWand(w);
            ok = MagickFalse;
            break;
        }
        DestroyMagickWand(w);
    }

    if (!ok) goto done;

    if (MagickGetNumberImages(anim) == 0) {
        fprintf(stderr, "[make_gif] no frames added\n");
        goto done;
    }

    MagickSetFirstIterator(anim);

    // Loop count (0 = infinite)
    char iters[32];
    snprintf(iters, sizeof iters, "%d", input.loop);
    if (MagickSetOption(anim, "gif:iterations", iters) != MagickTrue) {
        print_wand_error("gif:iterations", anim);
        goto done;
    }

    
    (void)MagickOptimizeImageLayers(anim);       // basic optimization
    (void)MagickOptimizeImageTransparency(anim); // for transparency

    if (MagickWriteImages(anim, input.out_gif, MagickTrue) != MagickTrue) {
        print_wand_error("write", anim);
        goto done;
    }

    rc = 0;

done:
    if (bg) DestroyPixelWand(bg);
    if (anim) DestroyMagickWand(anim);
    return rc;
}