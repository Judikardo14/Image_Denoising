#ifndef FILTERS_H
#define FILTERS_H

/**
 * Structure pour représenter un noyau de convolution 2D
 */
typedef struct {
    float *weights;   // Poids du noyau (alignés pour MKL)
    int size;         // Taille du noyau (size x size)
    float sigma;      // Paramètre sigma (pour gaussien)
} Kernel;

/**
 * Crée un noyau de convolution vide
 */
Kernel *create_kernel(int size);

/**
 * Libère la mémoire d'un noyau
 */
void free_kernel(Kernel *kernel);

/**
 * Crée un noyau gaussien 2D
 * @param size: taille du noyau (doit être impair)
 * @param sigma: écart-type de la gaussienne
 * @return: noyau gaussien normalisé (somme = 1.0)
 * 
 * Formule: G(x,y,σ) = (1/(2πσ²)) * exp(-(x²+y²)/(2σ²))
 */
Kernel *create_gaussian_kernel(int size, float sigma);

/**
 * Crée un noyau gaussien 1D (pour convolution séparable)
 * @param size: taille du noyau (doit être impair)
 * @param sigma: écart-type de la gaussienne
 * @return: vecteur gaussien normalisé (somme = 1.0)
 * 
 * Formule: G(x,σ) = (1/√(2πσ²)) * exp(-x²/(2σ²))
 */
float *create_gaussian_kernel_1d(int size, float sigma);

/**
 * Affiche les valeurs d'un noyau (pour debug)
 */
void print_kernel(const Kernel *kernel);

/**
 * Affiche les valeurs d'un noyau 1D (pour debug)
 */
void print_kernel_1d(const float *kernel, int size);

#endif // FILTERS_H
