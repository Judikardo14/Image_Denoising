# Note sur les Bibliothèques stb_image

## Vue d'Ensemble

Les bibliothèques **stb** sont des bibliothèques C header-only (un seul fichier .h) créées par Sean Barrett. Elles sont très populaires pour leur simplicité d'utilisation.

Pour ce projet, nous utilisons:
- **stb_image.h** - Pour charger des images (PNG, JPG, BMP, etc.)
- **stb_image_write.h** - Pour sauvegarder des images (PNG)

## Téléchargement

### Option 1: Automatique via Makefile

Le Makefile télécharge automatiquement ces fichiers:

```bash
make
```

### Option 2: Téléchargement Manuel

Si le Makefile échoue, téléchargez manuellement:

```bash
# Méthode 1: wget
wget https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
wget https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

# Méthode 2: curl
curl -O https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
curl -O https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

# Méthode 3: Navigateur web
# Visitez: https://github.com/nothings/stb
# Téléchargez: stb_image.h et stb_image_write.h
```

### Option 3: Copier Depuis le Code Source

Si vous avez accès au dépôt officiel:

```bash
git clone https://github.com/nothings/stb.git
cp stb/stb_image.h .
cp stb/stb_image_write.h .
rm -rf stb
```

## Vérification

Après téléchargement, vérifiez:

```bash
ls -lh stb_image*.h
```

Devrait afficher:
```
-rw-r--r-- 1 user group 737K stb_image.h
-rw-r--r-- 1 user group 169K stb_image_write.h
```

## Utilisation dans le Code

### Chargement d'Image (stb_image.h)

```c
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Charger une image
int width, height, channels;
unsigned char *data = stbi_load("image.jpg", &width, &height, &channels, 0);

if (data) {
    // Utiliser l'image...
    
    // Libérer
    stbi_image_free(data);
}
```

### Sauvegarde d'Image (stb_image_write.h)

```c
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Sauvegarder en PNG
unsigned char *image_data = ...; // Vos données RGB
int width = 800, height = 600, channels = 3;
stbi_write_png("output.png", width, height, channels, image_data, width * channels);
```

## Formats Supportés

### stb_image.h (Lecture)

- **PNG** (.png)
- **JPEG** (.jpg, .jpeg)
- **BMP** (.bmp)
- **TGA** (.tga)
- **GIF** (.gif)
- **PSD** (.psd) - Photoshop
- **HDR** (.hdr) - High Dynamic Range
- **PIC** (.pic) - Softimage PIC

### stb_image_write.h (Écriture)

- **PNG** (.png)
- **BMP** (.bmp)
- **TGA** (.tga)
- **JPEG** (.jpg) - Qualité ajustable

## Avantages de stb_image

1. **Header-only**: Un seul fichier à inclure
2. **Pas de dépendances**: Pas de libpng, libjpeg, etc.
3. **Simple**: API minimaliste et claire
4. **Portable**: Windows, Linux, macOS
5. **Licence**: Public domain / MIT

## Alternatives

Si vous ne pouvez pas utiliser stb_image:

### libpng + libjpeg

```bash
# Installation
sudo apt install libpng-dev libjpeg-dev

# Compilation
gcc ... -lpng -ljpeg
```

### OpenCV

```bash
# Installation
sudo apt install libopencv-dev

# Compilation
gcc ... -lopencv_core -lopencv_imgcodecs
```

### ImageMagick (MagickWand)

```bash
# Installation
sudo apt install libmagickwand-dev

# Compilation
gcc ... -lMagickWand-6.Q16 -lMagickCore-6.Q16
```

## Dépannage

### Erreur: "stb_image.h: No such file"

**Solution**: Téléchargez les fichiers (voir ci-dessus)

### Erreur: "multiple definition of stbi_load"

**Cause**: `STB_IMAGE_IMPLEMENTATION` défini dans plusieurs fichiers

**Solution**: Ne définir qu'une seule fois, dans io.c:

```c
// io.c
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// autres_fichiers.c
#include "stb_image.h"  // SANS #define
```

### Avertissement: "unused function"

**Solution**: Ignorer ou compiler avec `-Wno-unused-function`

### Erreur de mémoire lors du chargement

**Cause**: Image trop grande

**Solution**: Vérifier la taille avant chargement:

```c
int w, h, c;
if (!stbi_info("huge_image.png", &w, &h, &c)) {
    fprintf(stderr, "Cannot get image info\n");
} else {
    if (w * h > MAX_PIXELS) {
        fprintf(stderr, "Image too large: %dx%d\n", w, h);
    }
}
```

## Ressources

- **GitHub**: https://github.com/nothings/stb
- **Documentation**: Commentaires dans les headers
- **Exemples**: https://github.com/nothings/stb/tree/master/tests

## Notes Importantes

1. **Format mémoire**: stb_image retourne les données en format **entrelacé** (RGBRGBRGB...)
2. **Notre projet** utilise le format **planaire** (RRR...GGG...BBB...)
3. **Conversion nécessaire**: Via `interleaved_to_planar()` (dans image.c)

## Exemple Complet

```c
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
    // Charger
    int width, height, channels;
    unsigned char *img = stbi_load("input.jpg", &width, &height, &channels, 0);
    
    if (!img) {
        fprintf(stderr, "Error: %s\n", stbi_failure_reason());
        return 1;
    }
    
    printf("Image: %dx%d, %d channels\n", width, height, channels);
    
    // Traiter l'image...
    // (ici: juste copier)
    
    // Sauvegarder
    stbi_write_png("output.png", width, height, channels, img, width * channels);
    
    // Libérer
    stbi_image_free(img);
    
    printf("Done!\n");
    return 0;
}
```

Compilation:
```bash
gcc exemple.c -o exemple -lm
./exemple
```

---

**Note**: Les bibliothèques stb sont très stables et largement utilisées dans l'industrie (jeux vidéo, traitement d'images, etc.).

---

*Document créé pour le projet de débruitage d'images*  
*ENSGMM - Janvier 2026*
