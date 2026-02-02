#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "image.h"
#include "filters.h"
#include "mkl_ops.h"
#include "io.h"

// Fonction pour mesurer le temps d'exécution en millisecondes
double get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
}

// Structure pour stocker les résultats de benchmark
typedef struct {
    const char *method_name;
    double time_ms;
    ImageFloat *result;
} BenchResult;

void print_banner(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║    DÉBRUITAGE D'IMAGES AVEC INTEL MKL                         ║\n");
    printf("║    École Nationale Supérieure de Génie Mathématique           ║\n");
    printf("║    UNSTIM - Abomey                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void print_usage(const char *prog_name) {
    printf("Usage: %s [options]\n\n", prog_name);
    printf("Options:\n");
    printf("  -i <file>      Image d'entrée (PNG/JPG)\n");
    printf("  -o <prefix>    Préfixe pour les fichiers de sortie (défaut: output)\n");
    printf("  -k <size>      Taille du noyau gaussien (défaut: 7)\n");
    printf("  -s <sigma>     Sigma du filtre gaussien (défaut: 2.0)\n");
    printf("  -n <sigma>     Sigma du bruit à ajouter (défaut: 20.0)\n");
    printf("  -t <threads>   Nombre de threads MKL (défaut: auto)\n");
    printf("  -m <method>    Méthode: spatial|spatial_blas|separable|fft|all (défaut: all)\n");
    printf("  --test         Utiliser une image de test synthétique\n");
    printf("  -h             Afficher cette aide\n");
    printf("\n");
}

int main(int argc, char *argv[]) {
    // Paramètres par défaut
    const char *input_file = NULL;
    char output_prefix[256] = "data/output";
    int kernel_size = 7;
    float sigma = 2.0f;
    float noise_sigma = 20.0f;
    int num_threads = 0;  // Auto
    const char *method = "all";
    int use_test_image = 0;
    
    // Parsing des arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            input_file = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            snprintf(output_prefix, sizeof(output_prefix), "data/%s", argv[++i]);
        } else if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            kernel_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            sigma = atof(argv[++i]);
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            noise_sigma = atof(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            num_threads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            method = argv[++i];
        } else if (strcmp(argv[i], "--test") == 0) {
            use_test_image = 1;
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }
    
    print_banner();
    
    // Initialiser MKL
    mkl_init(num_threads);
    mkl_print_info();
    
    // Charger ou créer l'image
    ImageFloat *original = NULL;
    if (use_test_image || input_file == NULL) {
        printf("Création d'une image de test synthétique (512x512)...\n");
        original = create_test_image(512, 512);
    } else {
        printf("Chargement de l'image: %s\n", input_file);
        original = load_image(input_file);
    }
    
    if (!original) {
        fprintf(stderr, "Erreur: impossible de charger/créer l'image\n");
        return 1;
    }
    
    printf("Dimensions: %dx%d, %d canaux\n", original->width, original->height, original->channels);
    
    // Ajouter du bruit gaussien
    printf("\nAjout de bruit gaussien (sigma=%.1f)...\n", noise_sigma);
    ImageFloat *noisy = clone_image(original);
    add_gaussian_noise(noisy, noise_sigma);
    normalize_image(noisy);
    
    // Sauvegarder l'image bruitée
    char filename[256];
    snprintf(filename, sizeof(filename), "%s_noisy.png", output_prefix);
    save_image(filename, noisy);
    
    // Créer les noyaux
    printf("\nCréation du noyau gaussien (taille=%d, sigma=%.2f)...\n", kernel_size, sigma);
    Kernel *kernel_2d = create_gaussian_kernel(kernel_size, sigma);
    float *kernel_1d = create_gaussian_kernel_1d(kernel_size, sigma);
    
    if (!kernel_2d || !kernel_1d) {
        fprintf(stderr, "Erreur: impossible de créer les noyaux\n");
        free_image_float(original);
        free_image_float(noisy);
        return 1;
    }
    
    // Tableau pour stocker les résultats
    BenchResult results[4];
    int num_results = 0;
    
    printf("\n=== DÉBRUITAGE EN COURS ===\n\n");
    
    // Méthode 1: Convolution Spatiale
    if (strcmp(method, "all") == 0 || strcmp(method, "spatial") == 0) {
        printf("Méthode 1: Convolution Spatiale Directe...\n");
        double t0 = get_time_ms();
        ImageFloat *result = convolve_spatial(noisy, kernel_2d);
        double t1 = get_time_ms();
        
        if (result) {
            normalize_image(result);
            snprintf(filename, sizeof(filename), "%s_spatial.png", output_prefix);
            save_image(filename, result);
            
            results[num_results].method_name = "Spatial (naïve)";
            results[num_results].time_ms = t1 - t0;
            results[num_results].result = result;
            num_results++;
            
            printf("  → Temps: %.2f ms\n\n", t1 - t0);
        }
    }
    
    // Méthode 1bis: Convolution Spatiale BLAS
    if (strcmp(method, "all") == 0 || strcmp(method, "spatial_blas") == 0) {
        printf("Méthode 1bis: Convolution Spatiale avec BLAS...\n");
        double t0 = get_time_ms();
        ImageFloat *result = convolve_spatial_blas(noisy, kernel_2d);
        double t1 = get_time_ms();
        
        if (result) {
            normalize_image(result);
            snprintf(filename, sizeof(filename), "%s_spatial_blas.png", output_prefix);
            save_image(filename, result);
            
            results[num_results].method_name = "Spatial (BLAS)";
            results[num_results].time_ms = t1 - t0;
            results[num_results].result = result;
            num_results++;
            
            printf("  → Temps: %.2f ms\n\n", t1 - t0);
        }
    }
    
    // Méthode 2: Convolution Séparable
    if (strcmp(method, "all") == 0 || strcmp(method, "separable") == 0) {
        printf("Méthode 2: Convolution Séparable...\n");
        double t0 = get_time_ms();
        ImageFloat *result = convolve_separable(noisy, kernel_1d, kernel_size);
        double t1 = get_time_ms();
        
        if (result) {
            normalize_image(result);
            snprintf(filename, sizeof(filename), "%s_separable.png", output_prefix);
            save_image(filename, result);
            
            results[num_results].method_name = "Séparable";
            results[num_results].time_ms = t1 - t0;
            results[num_results].result = result;
            num_results++;
            
            printf("  → Temps: %.2f ms\n\n", t1 - t0);
        }
    }
    
    // Méthode 3: Convolution FFT
    if (strcmp(method, "all") == 0 || strcmp(method, "fft") == 0) {
        printf("Méthode 3: Convolution par FFT...\n");
        double t0 = get_time_ms();
        ImageFloat *result = convolve_fft(noisy, kernel_2d);
        double t1 = get_time_ms();
        
        if (result) {
            normalize_image(result);
            snprintf(filename, sizeof(filename), "%s_fft.png", output_prefix);
            save_image(filename, result);
            
            results[num_results].method_name = "FFT";
            results[num_results].time_ms = t1 - t0;
            results[num_results].result = result;
            num_results++;
            
            printf("  → Temps: %.2f ms\n\n", t1 - t0);
        }
    }
    
    // Afficher le tableau comparatif
    if (num_results > 1) {
        printf("\n=== COMPARAISON DES PERFORMANCES ===\n\n");
        printf("╔═══════════════════════════╦═════════════╦═════════════════╗\n");
        printf("║ Méthode                   ║ Temps (ms)  ║ Accélération    ║\n");
        printf("╠═══════════════════════════╬═════════════╬═════════════════╣\n");
        
        double baseline = results[0].time_ms;
        for (int i = 0; i < num_results; i++) {
            double speedup = baseline / results[i].time_ms;
            printf("║ %-25s ║ %10.2f  ║ %10.2fx      ║\n", 
                   results[i].method_name, results[i].time_ms, speedup);
        }
        printf("╚═══════════════════════════╩═════════════╩═════════════════╝\n\n");
    }
    
    // Nettoyage
    for (int i = 0; i < num_results; i++) {
        free_image_float(results[i].result);
    }
    free_image_float(original);
    free_image_float(noisy);
    free_kernel(kernel_2d);
    mkl_free(kernel_1d);
    
    printf("Traitement terminé avec succès!\n\n");
    
    return 0;
}
