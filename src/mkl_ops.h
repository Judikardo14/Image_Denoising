#ifndef MKL_OPS_H
#define MKL_OPS_H

#include "image.h"
#include "filters.h"
#include <mkl/mkl.h>

/**
 * Initialise MKL avec le nombre de threads spécifié
 * @param num_threads: nombre de threads (0 = automatique)
 */
void mkl_init(int num_threads);

/**
 * Affiche les informations de configuration MKL
 */
void mkl_print_info(void);

/**
 * MÉTHODE 1: Convolution spatiale directe (naïve)
 * Applique directement la formule de convolution
 * Complexité: O(N * K²) où N = nb pixels, K = taille noyau
 * 
 * @param img: image source
 * @param kernel: noyau de convolution
 * @return: image filtrée
 */
ImageFloat *convolve_spatial(const ImageFloat *img, const Kernel *kernel);

/**
 * MÉTHODE 1bis: Convolution spatiale optimisée avec BLAS
 * Utilise cblas_sdot pour calculer les produits scalaires
 * Gain typique: 2-3x par rapport à la version naïve
 * 
 * @param img: image source
 * @param kernel: noyau de convolution
 * @return: image filtrée
 */
ImageFloat *convolve_spatial_blas(const ImageFloat *img, const Kernel *kernel);

/**
 * MÉTHODE 2: Convolution séparable (pour noyaux gaussiens)
 * Décompose la convolution 2D en deux convolutions 1D
 * Complexité: O(2*N*K) au lieu de O(N*K²)
 * Gain typique: 5-10x pour noyaux moyens
 * 
 * @param img: image source
 * @param kernel_1d: noyau gaussien 1D
 * @param kernel_size: taille du noyau
 * @return: image filtrée
 */
ImageFloat *convolve_separable(const ImageFloat *img, const float *kernel_1d, int kernel_size);

/**
 * MÉTHODE 3: Convolution par FFT (transformée de Fourier)
 * Utilise le théorème: Convolution(I,K) = IFFT(FFT(I) * FFT(K))
 * Complexité: O(N log N)
 * Optimal pour grands noyaux (K > 9x9 typiquement)
 * 
 * @param img: image source
 * @param kernel: noyau de convolution
 * @return: image filtrée
 */
ImageFloat *convolve_fft(const ImageFloat *img, const Kernel *kernel);

// ============================================================================
// Fonctions auxiliaires pour la convolution séparable
// ============================================================================

/**
 * Convolution 1D (horizontale ou verticale)
 * @param horizontal: 1 pour horizontal, 0 pour vertical
 */
ImageFloat *convolve_separable_1d(const ImageFloat *img, const float *kernel_1d, 
                                   int kernel_size, int horizontal);

// ============================================================================
// Fonctions auxiliaires pour la convolution FFT
// ============================================================================

/**
 * FFT 2D forward (Réel -> Complexe)
 * Utilise MKL DFTI
 * @return: buffer complexe (à libérer avec mkl_free)
 */
void *fft_2d_forward(const float *img, int width, int height);

/**
 * FFT 2D backward (Complexe -> Réel)
 * Utilise MKL DFTI + normalisation
 * @return: buffer réel (à libérer avec mkl_free)
 */
float *fft_2d_backward(void *fft_data, int width, int height);

/**
 * Multiplication complexe point-à-point dans le domaine fréquentiel
 * (a+bi) * (c+di) = (ac-bd) + (ad+bc)i
 */
void fft_multiply(void *fft1, const void *fft2, int width, int height);

#endif // MKL_OPS_H
