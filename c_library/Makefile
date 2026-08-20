CC := gcc
AR := ar

CPPFLAGS := \
	-Iinclude \
	-Iinclude/ImageMagick-7 \
	-DMAGICKCORE_QUANTUM_DEPTH=16 \
	-DMAGICKCORE_HDRI_ENABLE=1
CFLAGS := -Wall -Wextra -fPIC

SRC := \
	src/convertToJPG.c \
	src/convertToPNG.c \
	src/convertToTIFF.c \
	src/convertToWEBP.c \
	src/makeGIF.c

OBJ := $(SRC:src/%.c=build/%.o)

CONVERSION_LIB := lib/libourconversionlib.a

LDLIBS := \
	-Llib \
	-lourconversionlib \
	-lMagickWand-7.Q16HDRI \
	-lMagickCore-7.Q16HDRI \
	-lpng16 \
	-ltiff \
	-ljpeg \
	-lwebp \
	-lwebpdemux \
	-lwebpmux \
	-lsharpyuv \
	-lz \
	-lm \
	-lpthread \
	-ldl

.PHONY: all run clean rebuild

all: main

main: main.c $(CONVERSION_LIB) include/ourconversionlib.h
	$(CC) $(CPPFLAGS) $(CFLAGS) main.c $(LDLIBS) -o $@

$(CONVERSION_LIB): $(OBJ)
	$(AR) rcs $@ $^

build/%.o: src/%.c include/ourconversionlib.h
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: main
	./main

clean:
	rm -rf build main

rebuild: clean all