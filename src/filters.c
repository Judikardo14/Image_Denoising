#include "filters.h"
#if __has_include(<mkl.h>)
#  include <mkl.h>
#elif __has_include(<mkl/mkl.h>)
#  include <mkl/mkl.h>
#else
#  error "MKL header not found"
#endif
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Kernel *create_kernel(int size) {
    Kernel *kernel = (Kernel *)malloc(sizeof(Kernel));
    if (!kernel) return NULL;
    
    kernel->size = size;
    kernel->sigma = 0.0f;
    
    // Allocation alignée pour MKL
    kernel->weights = (float *)mkl_malloc(size * size * sizeof(float), 64);
    if (!kernel->weights) {
        free(kernel);
        return NULL;
    }
    
    return kernel;
}

void free_kernel(Kernel *kernel) {
    if (kernel) {
        if (kernel->weights) {
            mkl_free(kernel->weights);
        }
        free(kernel);
    }
}

Kernel *create_gaussian_kernel(int size, float sigma) {
    Kernel *kernel = create_kernel(size);
    if (!kernel) return NULL;
    
    kernel->sigma = sigma;
    int center = size / 2;
    float sum = 0.0f;
    float sigma_sq = sigma * sigma;
    float coeff = 1.0f / (2.0f * M_PI * sigma_sq);
    
    // Calcul des poids gaussiens 2D
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int dx = x - center;
            int dy = y - center;
            float dist_sq = (float)(dx * dx + dy * dy);
            float value = coeff * expf(-dist_sq / (2.0f * sigma_sq));
            kernel->weights[y * size + x] = value;
            sum += value;
        }
    }
    
    // Normalisation avec MKL BLAS : somme = 1.0
    // cblas_sscal multiplie le vecteur par 1/sum
    cblas_sscal(size * size, 1.0f / sum, kernel->weights, 1);
    
    return kernel;
}

float *create_gaussian_kernel_1d(int size, float sigma) {
    // Allocation alignée pour MKL
    float *kernel = (float *)mkl_malloc(size * sizeof(float), 64);
    if (!kernel) return NULL;
    
    int center = size / 2;
    float sum = 0.0f;
    float sigma_sq = sigma * sigma;
    
    // Calcul des poids gaussiens 1D
    // G(x,σ) = (1/√(2πσ²)) * exp(-x²/(2σ²))
    for (int i = 0; i < size; i++) {
        int x = i - center;
        float value = expf(-(float)(x * x) / (2.0f * sigma_sq));
        kernel[i] = value;
        sum += value;
    }
    
    // Normalisation avec MKL BLAS
    cblas_sscal(size, 1.0f / sum, kernel, 1);
    
    return kernel;
}

void print_kernel(const Kernel *kernel) {
    printf("Kernel %dx%d (sigma=%.2f):\n", kernel->size, kernel->size, kernel->sigma);
    for (int y = 0; y < kernel->size; y++) {
        for (int x = 0; x < kernel->size; x++) {
            printf("%.6f ", kernel->weights[y * kernel->size + x]);
        }
        printf("\n");
    }
    printf("\n");
}

void print_kernel_1d(const float *kernel, int size) {
    printf("Kernel 1D (size=%d): ", size);
    for (int i = 0; i < size; i++) {
        printf("%.6f ", kernel[i]);
    }
    printf("\n\n");
}
