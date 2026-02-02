#include "image.h"
#if __has_include(<mkl.h>)
#  include <mkl.h>
#elif __has_include(<mkl/mkl.h>)
#  include <mkl/mkl.h>
#else
#  error "MKL header not found"
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

ImageFloat *create_image_float(int width, int height, int channels) {
    ImageFloat *img = (ImageFloat *)malloc(sizeof(ImageFloat));
    if (!img) return NULL;
    
    img->width = width;
    img->height = height;
    img->channels = channels;
    
    // Allocation alignée sur 64 octets pour AVX-512
    size_t total_pixels = (size_t)width * height * channels;
    img->data = (float *)mkl_malloc(total_pixels * sizeof(float), 64);
    
    if (!img->data) {
        free(img);
        return NULL;
    }
    
    // Initialisation à zéro
    memset(img->data, 0, total_pixels * sizeof(float));
    
    return img;
}

void free_image_float(ImageFloat *img) {
    if (img) {
        if (img->data) {
            mkl_free(img->data);
        }
        free(img);
    }
}

ImageFloat *interleaved_to_planar(unsigned char *data, int w, int h, int c) {
    ImageFloat *img = create_image_float(w, h, c);
    if (!img) return NULL;
    
    size_t pixels = (size_t)w * h;
    
    // Conversion entrelacé -> planaire
    // Entrée: RGBRGBRGB... (data[i*c + ch])
    // Sortie: RRR...GGG...BBB... (img->data[ch*pixels + i])
    for (size_t i = 0; i < pixels; i++) {
        for (int ch = 0; ch < c; ch++) {
            img->data[ch * pixels + i] = (float)data[i * c + ch];
        }
    }
    
    return img;
}

unsigned char *planar_to_interleaved(const ImageFloat *img) {
    size_t pixels = (size_t)img->width * img->height;
    size_t total_bytes = pixels * img->channels;
    
    unsigned char *data = (unsigned char *)malloc(total_bytes);
    if (!data) return NULL;
    
    // Conversion planaire -> entrelacé avec clipping [0, 255]
    for (size_t i = 0; i < pixels; i++) {
        for (int ch = 0; ch < img->channels; ch++) {
            float val = img->data[ch * pixels + i];
            // Clamp et arrondi
            val = clampf(val, 0.0f, 255.0f);
            data[i * img->channels + ch] = (unsigned char)(val + 0.5f);
        }
    }
    
    return data;
}

ImageFloat *clone_image(const ImageFloat *img) {
    ImageFloat *clone = create_image_float(img->width, img->height, img->channels);
    if (!clone) return NULL;
    
    size_t total_pixels = (size_t)img->width * img->height * img->channels;
    memcpy(clone->data, img->data, total_pixels * sizeof(float));
    
    return clone;
}

void normalize_image(ImageFloat *img) {
    size_t total_pixels = (size_t)img->width * img->height * img->channels;
    
    // Trouver min et max
    float min_val = img->data[0];
    float max_val = img->data[0];
    
    for (size_t i = 1; i < total_pixels; i++) {
        if (img->data[i] < min_val) min_val = img->data[i];
        if (img->data[i] > max_val) max_val = img->data[i];
    }
    
    // Normaliser [min, max] -> [0, 255]
    float range = max_val - min_val;
    if (range > 1e-6f) {
        float scale = 255.0f / range;
        for (size_t i = 0; i < total_pixels; i++) {
            img->data[i] = (img->data[i] - min_val) * scale;
        }
    }
}

// Générateur de nombres aléatoires gaussiens (Box-Muller transform)
static float randn() {
    static int has_spare = 0;
    static float spare;
    
    if (has_spare) {
        has_spare = 0;
        return spare;
    }
    
    has_spare = 1;
    
    float u, v, s;
    do {
        u = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        v = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        s = u * u + v * v;
    } while (s >= 1.0f || s == 0.0f);
    
    s = sqrtf(-2.0f * logf(s) / s);
    spare = v * s;
    return u * s;
}

void add_gaussian_noise(ImageFloat *img, float sigma) {
    size_t total_pixels = (size_t)img->width * img->height * img->channels;
    
    // Initialiser le générateur aléatoire
    srand((unsigned int)time(NULL));
    
    for (size_t i = 0; i < total_pixels; i++) {
        float noise = randn() * sigma;
        img->data[i] += noise;
    }
}
