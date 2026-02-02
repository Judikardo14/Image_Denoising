#include "io.h"
#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Utilisation de stb_image pour charger/sauvegarder les images
// Ces bibliothèques header-only sont très pratiques
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

ImageFloat *load_image(const char *filename) {
    int width, height, channels;
    
    // Charger l'image avec stb_image
    // Force à charger en RGB (3 canaux) ou grayscale (1 canal)
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 0);
    
    if (!data) {
        fprintf(stderr, "Erreur: impossible de charger l'image '%s'\n", filename);
        fprintf(stderr, "Raison: %s\n", stbi_failure_reason());
        return NULL;
    }
    
    printf("Image chargée: %s (%dx%d, %d canaux)\n", filename, width, height, channels);
    
    // Convertir en format planaire
    ImageFloat *img = interleaved_to_planar(data, width, height, channels);
    
    // Libérer les données stb_image
    stbi_image_free(data);
    
    return img;
}

int save_image(const char *filename, const ImageFloat *img) {
    if (!img || !img->data) {
        fprintf(stderr, "Erreur: image invalide\n");
        return 0;
    }
    
    // Convertir en format entrelacé
    unsigned char *data = planar_to_interleaved(img);
    if (!data) {
        fprintf(stderr, "Erreur: échec de la conversion planaire->entrelacé\n");
        return 0;
    }
    
    // Sauvegarder en PNG avec stb_image_write
    int result = stbi_write_png(filename, img->width, img->height, 
                                 img->channels, data, img->width * img->channels);
    
    free(data);
    
    if (result) {
        printf("Image sauvegardée: %s\n", filename);
    } else {
        fprintf(stderr, "Erreur: impossible de sauvegarder '%s'\n", filename);
    }
    
    return result;
}

ImageFloat *create_test_image(int width, int height) {
    ImageFloat *img = create_image_float(width, height, 3);
    if (!img) return NULL;
    
    size_t pixels = (size_t)width * height;
    float *r = img->data;
    float *g = img->data + pixels;
    float *b = img->data + 2 * pixels;
    
    // Créer un motif de test intéressant
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            size_t idx = y * width + x;
            
            // Dégradé horizontal pour R
            r[idx] = 255.0f * (float)x / (float)width;
            
            // Dégradé vertical pour G
            g[idx] = 255.0f * (float)y / (float)height;
            
            // Motif sinusoïdal pour B
            float freq = 10.0f;
            float wave = sinf(freq * 2.0f * M_PI * (float)x / (float)width) * 
                        cosf(freq * 2.0f * M_PI * (float)y / (float)height);
            b[idx] = 127.5f + 127.5f * wave;
        }
    }
    
    return img;
}
