#ifndef STB_IMAGE_H
#define STB_IMAGE_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

#define STBI_rgb_alpha 4

#ifdef __cplusplus
extern "C" {
#endif

static unsigned char* stbi_load_from_memory(const unsigned char* data, int len, int* x, int* y, int* channels_in_file, int desired_channels);
static void stbi_image_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif

#ifdef STB_IMAGE_IMPLEMENTATION

static unsigned char* stbi_load_from_memory(const unsigned char* data, int len, int* x, int* y, int* channels_in_file, int desired_channels) {
    (void)desired_channels;
    if (data == NULL || len <= 0) {
        if (x) *x = 0;
        if (y) *y = 0;
        if (channels_in_file) *channels_in_file = 0;
        return NULL;
    }

    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_memory(&image, data, static_cast<size_t>(len))) {
        if (x) *x = 0;
        if (y) *y = 0;
        if (channels_in_file) *channels_in_file = 0;
        return NULL;
    }

    image.format = PNG_FORMAT_RGBA;
    if (x) *x = image.width;
    if (y) *y = image.height;
    if (channels_in_file) *channels_in_file = 4;

    unsigned char* buffer = static_cast<unsigned char*>(malloc(PNG_IMAGE_SIZE(image)));
    if (!buffer) {
        png_image_free(&image);
        return NULL;
    }

    if (!png_image_finish_read(&image, NULL, buffer, 0, NULL)) {
        free(buffer);
        png_image_free(&image);
        return NULL;
    }

    return buffer;
}

static void stbi_image_free(void* ptr) {
    free(ptr);
}

#endif
