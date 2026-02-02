#include "mkl_ops.h"
#include <mkl/mkl.h>
#include <mkl/mkl_dfti.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Initialisation MKL
// ============================================================================

void mkl_init(int num_threads) {
    if (num_threads > 0) {
        mkl_set_num_threads(num_threads);
    }
    printf("MKL initialisé avec %d threads\n", mkl_get_max_threads());
}

void mkl_print_info(void) {
    printf("=== Configuration MKL ===\n");
    printf("  Threads max      : %d\n", mkl_get_max_threads());
    printf("  Dynamic threads  : %d\n", mkl_get_dynamic());
    printf("\n");
}

// ============================================================================
// MÉTHODE 1: Convolution Spatiale Directe
// ============================================================================

ImageFloat *convolve_spatial(const ImageFloat *img, const Kernel *kernel) {
    ImageFloat *output = create_image_float(img->width, img->height, img->channels);
    if (!output) return NULL;
    
    int half_size = kernel->size / 2;
    size_t pixels_per_channel = (size_t)img->width * img->height;
    
    // Pour chaque canal (R, G, B)
    for (int c = 0; c < img->channels; c++) {
        const float *src = img->data + c * pixels_per_channel;
        float *dst = output->data + c * pixels_per_channel;
        
        // Pour chaque pixel
        for (int y = 0; y < img->height; y++) {
            for (int x = 0; x < img->width; x++) {
                float sum = 0.0f;
                
                // Appliquer le noyau
                for (int ky = 0; ky < kernel->size; ky++) {
                    for (int kx = 0; kx < kernel->size; kx++) {
                        // Gestion des bords (clamp)
                        int img_y = clamp(y + ky - half_size, 0, img->height - 1);
                        int img_x = clamp(x + kx - half_size, 0, img->width - 1);
                        
                        float pixel_val = src[img_y * img->width + img_x];
                        float kernel_val = kernel->weights[ky * kernel->size + kx];
                        sum += pixel_val * kernel_val;
                    }
                }
                
                dst[y * img->width + x] = sum;
            }
        }
    }
    
    return output;
}

// ============================================================================
// MÉTHODE 1bis: Convolution Spatiale avec BLAS
// ============================================================================

// Fonction auxiliaire pour extraire un patch d'image
static void extract_patch(const float *img, int w, int h, int x, int y, 
                          int patch_size, float *patch) {
    int half = patch_size / 2;
    
    for (int py = 0; py < patch_size; py++) {
        for (int px = 0; px < patch_size; px++) {
            int img_y = clamp(y + py - half, 0, h - 1);
            int img_x = clamp(x + px - half, 0, w - 1);
            patch[py * patch_size + px] = img[img_y * w + img_x];
        }
    }
}

// Calcul de convolution pour un pixel avec BLAS
static float convolve_pixel_blas(const float *patch, const float *kernel, int size) {
    // Produit scalaire optimisé par MKL
    // cblas_sdot calcule: sum(patch[i] * kernel[i])
    return cblas_sdot(size * size, patch, 1, kernel, 1);
}

ImageFloat *convolve_spatial_blas(const ImageFloat *img, const Kernel *kernel) {
    ImageFloat *output = create_image_float(img->width, img->height, img->channels);
    if (!output) return NULL;
    
    size_t pixels_per_channel = (size_t)img->width * img->height;
    int patch_size_sq = kernel->size * kernel->size;
    
    // Buffer temporaire pour le patch (réutilisé pour chaque pixel)
    float *patch = (float *)mkl_malloc(patch_size_sq * sizeof(float), 64);
    if (!patch) {
        free_image_float(output);
        return NULL;
    }
    
    // Pour chaque canal
    for (int c = 0; c < img->channels; c++) {
        const float *src = img->data + c * pixels_per_channel;
        float *dst = output->data + c * pixels_per_channel;
        
        // Pour chaque pixel
        for (int y = 0; y < img->height; y++) {
            for (int x = 0; x < img->width; x++) {
                // Extraire le patch autour du pixel
                extract_patch(src, img->width, img->height, x, y, kernel->size, patch);
                
                // Calculer la convolution avec BLAS
                dst[y * img->width + x] = convolve_pixel_blas(patch, kernel->weights, kernel->size);
            }
        }
    }
    
    mkl_free(patch);
    return output;
}
// Ce fichier contient la partie 2 de mkl_ops.c
// MÉTHODE 2: Convolution Séparable

// ============================================================================
// MÉTHODE 2: Convolution Séparable (continuation de mkl_ops.c)
// ============================================================================

ImageFloat *convolve_separable_1d(const ImageFloat *img, const float *kernel_1d, 
                                   int kernel_size, int horizontal) {
    ImageFloat *output = create_image_float(img->width, img->height, img->channels);
    if (!output) return NULL;
    
    int half_size = kernel_size / 2;
    size_t pixels_per_channel = (size_t)img->width * img->height;
    
    // Pour chaque canal
    for (int c = 0; c < img->channels; c++) {
        const float *src = img->data + c * pixels_per_channel;
        float *dst = output->data + c * pixels_per_channel;
        
        if (horizontal) {
            // Convolution horizontale (sur chaque ligne)
            for (int y = 0; y < img->height; y++) {
                for (int x = 0; x < img->width; x++) {
                    float sum = 0.0f;
                    
                    for (int k = 0; k < kernel_size; k++) {
                        int src_x = clamp(x + k - half_size, 0, img->width - 1);
                        sum += src[y * img->width + src_x] * kernel_1d[k];
                    }
                    
                    dst[y * img->width + x] = sum;
                }
            }
        } else {
            // Convolution verticale (sur chaque colonne)
            for (int y = 0; y < img->height; y++) {
                for (int x = 0; x < img->width; x++) {
                    float sum = 0.0f;
                    
                    for (int k = 0; k < kernel_size; k++) {
                        int src_y = clamp(y + k - half_size, 0, img->height - 1);
                        sum += src[src_y * img->width + x] * kernel_1d[k];
                    }
                    
                    dst[y * img->width + x] = sum;
                }
            }
        }
    }
    
    return output;
}

ImageFloat *convolve_separable(const ImageFloat *img, const float *kernel_1d, 
                                int kernel_size) {
    // Première passe: convolution horizontale
    ImageFloat *temp = convolve_separable_1d(img, kernel_1d, kernel_size, 1);
    if (!temp) return NULL;
    
    // Deuxième passe: convolution verticale
    ImageFloat *result = convolve_separable_1d(temp, kernel_1d, kernel_size, 0);
    
    // Libérer l'image temporaire
    free_image_float(temp);
    
    return result;
}
// Ce fichier contient la partie 3 de mkl_ops.c
// MÉTHODE 3: Convolution par FFT

// ============================================================================
// MÉTHODE 3: Convolution par FFT (continuation de mkl_ops.c)
// ============================================================================

void *fft_2d_forward(const float *img, int width, int height) {
    DFTI_DESCRIPTOR_HANDLE handle = NULL;
    MKL_LONG dims[2] = {(MKL_LONG)height, (MKL_LONG)width};
    
    // 1. Créer le descripteur FFT Réelle 2D
    DftiCreateDescriptor(&handle, DFTI_SINGLE, DFTI_REAL, 2, dims);
    
    // 2. Configuration Out-of-place (résultat dans un buffer séparé)
    DftiSetValue(handle, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
    
    // 3. Format de sortie: complexe complet
    DftiSetValue(handle, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX);
    
    // 4. Strides d'entrée (image réelle)
    MKL_LONG i_strides[3] = {0, (MKL_LONG)width, 1};
    DftiSetValue(handle, DFTI_INPUT_STRIDES, i_strides);
    
    // 5. Strides de sortie (spectre complexe)
    // Largeur spectrale: (width/2 + 1) pour FFT réelle
    MKL_LONG o_strides[3] = {0, (MKL_LONG)(width/2 + 1), 1};
    DftiSetValue(handle, DFTI_OUTPUT_STRIDES, o_strides);
    
    // 6. Commit (compilation du plan FFT pour optimisation)
    DftiCommitDescriptor(handle);
    
    // 7. Allocation du résultat complexe
    // Format: [real0, imag0, real1, imag1, ...]
    size_t complex_count = (size_t)height * (width/2 + 1);
    float *fft_result = (float *)mkl_malloc(complex_count * 2 * sizeof(float), 64);
    
    if (!fft_result) {
        DftiFreeDescriptor(&handle);
        return NULL;
    }
    
    // 8. Calcul de la FFT Forward
    DftiComputeForward(handle, (void *)img, fft_result);
    
    // 9. Libérer le descripteur
    DftiFreeDescriptor(&handle);
    
    return fft_result;
}

float *fft_2d_backward(void *fft_data, int width, int height) {
    DFTI_DESCRIPTOR_HANDLE handle = NULL;
    MKL_LONG dims[2] = {(MKL_LONG)height, (MKL_LONG)width};
    
    // Créer et configurer le descripteur
    DftiCreateDescriptor(&handle, DFTI_SINGLE, DFTI_REAL, 2, dims);
    DftiSetValue(handle, DFTI_PLACEMENT, DFTI_NOT_INPLACE);
    DftiSetValue(handle, DFTI_CONJUGATE_EVEN_STORAGE, DFTI_COMPLEX_COMPLEX);
    
    // Strides d'entrée (spectre complexe)
    MKL_LONG i_strides[3] = {0, (MKL_LONG)(width/2 + 1), 1};
    DftiSetValue(handle, DFTI_INPUT_STRIDES, i_strides);
    
    // Strides de sortie (image réelle)
    MKL_LONG o_strides[3] = {0, (MKL_LONG)width, 1};
    DftiSetValue(handle, DFTI_OUTPUT_STRIDES, o_strides);
    
    DftiCommitDescriptor(handle);
    
    // Allocation pour le résultat réel
    size_t real_size = (size_t)height * width;
    float *result = (float *)mkl_malloc(real_size * sizeof(float), 64);
    
    if (!result) {
        DftiFreeDescriptor(&handle);
        return NULL;
    }
    
    // Calcul de l'IFFT Backward
    DftiComputeBackward(handle, fft_data, result);
    
    DftiFreeDescriptor(&handle);
    
    // Normalisation (MKL ne normalise pas automatiquement)
    // Il faut diviser par (width * height)
    float scale = 1.0f / (float)(width * height);
    cblas_sscal((MKL_INT)real_size, scale, result, 1);
    
    return result;
}

void fft_multiply(void *fft1, const void *fft2, int width, int height) {
    float *f1 = (float *)fft1;
    const float *f2 = (const float *)fft2;
    size_t complex_count = (size_t)height * (width/2 + 1);
    
    // Multiplication complexe point-à-point
    // (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
    for (size_t i = 0; i < complex_count; i++) {
        float a = f1[2*i];       // Partie réelle de fft1
        float b = f1[2*i + 1];   // Partie imaginaire de fft1
        float c = f2[2*i];       // Partie réelle de fft2
        float d = f2[2*i + 1];   // Partie imaginaire de fft2
        
        f1[2*i]     = a * c - b * d;  // Nouvelle partie réelle
        f1[2*i + 1] = a * d + b * c;  // Nouvelle partie imaginaire
    }
}

ImageFloat *convolve_fft(const ImageFloat *img, const Kernel *kernel) {
    ImageFloat *output = create_image_float(img->width, img->height, img->channels);
    if (!output) return NULL;
    
    // Préparer le noyau zéro-paddé aux dimensions de l'image
    float *kernel_padded = (float *)mkl_calloc(img->width * img->height, sizeof(float), 64);
    if (!kernel_padded) {
        free_image_float(output);
        return NULL;
    }
    
    // Copier le noyau au centre (décalé pour éviter les artefacts circulaires)
    int k_half = kernel->size / 2;
    for (int y = 0; y < kernel->size; y++) {
        for (int x = 0; x < kernel->size; x++) {
            // Placer le centre du noyau à l'origine (coin supérieur gauche)
            int dst_y = (y - k_half + img->height) % img->height;
            int dst_x = (x - k_half + img->width) % img->width;
            kernel_padded[dst_y * img->width + dst_x] = 
                kernel->weights[y * kernel->size + x];
        }
    }
    
    // FFT du noyau (calculée une seule fois, réutilisée pour tous les canaux)
    void *kernel_fft = fft_2d_forward(kernel_padded, img->width, img->height);
    mkl_free(kernel_padded);
    
    if (!kernel_fft) {
        free_image_float(output);
        return NULL;
    }
    
    // Pour chaque canal RGB
    size_t pixels_per_channel = (size_t)img->width * img->height;
    for (int c = 0; c < img->channels; c++) {
        const float *src = img->data + c * pixels_per_channel;
        float *dst = output->data + c * pixels_per_channel;
        
        // 1. FFT de l'image (canal courant)
        void *img_fft = fft_2d_forward(src, img->width, img->height);
        if (!img_fft) continue;
        
        // 2. Multiplication dans le domaine fréquentiel
        fft_multiply(img_fft, kernel_fft, img->width, img->height);
        
        // 3. IFFT pour revenir au domaine spatial
        float *result = fft_2d_backward(img_fft, img->width, img->height);
        if (result) {
            memcpy(dst, result, pixels_per_channel * sizeof(float));
            mkl_free(result);
        }
        
        mkl_free(img_fft);
    }
    
    mkl_free(kernel_fft);
    return output;
}
