#ifndef IMAGE_H
#define IMAGE_H

#include <stddef.h>

/**
 * Structure pour représenter une image en virgule flottante
 * Format planaire : tous les pixels R, puis tous les G, puis tous les B
 * Layout mémoire : [R0 R1 R2 ... Rn G0 G1 G2 ... Gn B0 B1 B2 ... Bn]
 */
typedef struct {
    float *data;      // Données en format planaire (alignées pour MKL)
    int width;        // Largeur de l'image
    int height;       // Hauteur de l'image
    int channels;     // Nombre de canaux (1=grayscale, 3=RGB)
} ImageFloat;

/**
 * Crée une nouvelle image flottante
 * Utilise mkl_malloc pour garantir l'alignement mémoire optimal
 */
ImageFloat *create_image_float(int width, int height, int channels);

/**
 * Libère la mémoire d'une image flottante
 */
void free_image_float(ImageFloat *img);

/**
 * Convertit une image entrelacée (RGBRGBRGB...) en format planaire (RRR...GGG...BBB...)
 * @param data: données sources au format entrelacé (unsigned char)
 * @param w: largeur
 * @param h: hauteur
 * @param c: nombre de canaux
 * @return: nouvelle image au format planaire
 */
ImageFloat *interleaved_to_planar(unsigned char *data, int w, int h, int c);

/**
 * Convertit une image planaire (RRR...GGG...BBB...) en format entrelacé (RGBRGBRGB...)
 * @param img: image source au format planaire
 * @return: données au format entrelacé (unsigned char), à libérer avec free()
 */
unsigned char *planar_to_interleaved(const ImageFloat *img);

/**
 * Clone une image (copie profonde)
 */
ImageFloat *clone_image(const ImageFloat *img);

/**
 * Normalise les valeurs de l'image dans la plage [0, 255]
 */
void normalize_image(ImageFloat *img);

/**
 * Ajoute du bruit gaussien à une image
 * @param img: image à bruiter
 * @param sigma: écart-type du bruit gaussien
 */
void add_gaussian_noise(ImageFloat *img, float sigma);

/**
 * Fonction utilitaire pour clamper une valeur
 */
static inline int clamp(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

/**
 * Fonction utilitaire pour clamper un float
 */
static inline float clampf(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

#endif // IMAGE_H
