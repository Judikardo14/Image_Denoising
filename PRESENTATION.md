# Projet: Débruitage d'Images avec Intel MKL
## Présentation du Code Source

---

## 📌 Vue d'Ensemble

Ce projet implémente **trois méthodes de débruitage d'images** optimisées avec Intel MKL:

1. **Convolution Spatiale** - Approche directe avec optimisation BLAS
2. **Convolution Séparable** - Décomposition en deux passes 1D
3. **Convolution par FFT** - Transformée de Fourier rapide

**Contexte académique:**
- École: ENSGMM (UNSTIM - Abomey)
- Cours: Outils de Calcul Scientifique
- Date: Janvier 2026

---

## 📂 Structure du Projet

```
image_denoise_mkl/
│
├── 📄 Fichiers Source (.c)
│   ├── main.c              # Programme principal avec benchmarks
│   ├── image.c             # Manipulation d'images
│   ├── filters.c           # Génération de noyaux gaussiens
│   ├── mkl_ops.c           # Opérations MKL (convolutions)
│   └── io.c                # Entrées/sorties d'images
│
├── 📄 Fichiers Header (.h)
│   ├── image.h             # Structures et prototypes images
│   ├── filters.h           # Prototypes filtres
│   ├── mkl_ops.h           # Prototypes opérations MKL
│   └── io.h                # Prototypes I/O
│
├── 📄 Compilation
│   └── Makefile            # Fichier de compilation
│
├── 📄 Documentation
│   ├── README.md           # Guide utilisateur
│   ├── INSTALLATION.md     # Guide d'installation
│   ├── TECHNIQUE.md        # Documentation technique
│   └── PRESENTATION.md     # Ce fichier
│
└── 📄 Scripts
    ├── demo.sh             # Démonstration automatisée
    └── run_tests.sh        # Tests unitaires
```

---

## 🔧 Description des Modules

### 1. main.c (Programme Principal)

**Rôle:** Point d'entrée du programme

**Fonctionnalités:**
- Parsing des arguments de ligne de commande
- Chargement/création d'images
- Ajout de bruit gaussien contrôlé
- Exécution des différentes méthodes de débruitage
- Benchmarking et affichage des résultats
- Sauvegarde des images résultantes

**Fonctions principales:**
```c
int main(int argc, char *argv[])
double get_time_ms(void)
void print_banner(void)
void print_usage(const char *prog_name)
```

**Utilisation:**
```bash
./image_denoise [options]
  -i <file>      # Image d'entrée
  -k <size>      # Taille du noyau
  -s <sigma>     # Sigma du filtre
  -n <sigma>     # Sigma du bruit
  -m <method>    # Méthode à utiliser
```

---

### 2. image.c/h (Gestion des Images)

**Rôle:** Structures de données et opérations de base sur les images

**Structure principale:**
```c
typedef struct {
    float *data;      // Format planaire
    int width;
    int height;
    int channels;     // 1=grayscale, 3=RGB
} ImageFloat;
```

**Fonctions clés:**
```c
// Création/destruction
ImageFloat *create_image_float(int w, int h, int c);
void free_image_float(ImageFloat *img);

// Conversions
ImageFloat *interleaved_to_planar(unsigned char *data, ...);
unsigned char *planar_to_interleaved(const ImageFloat *img);

// Opérations
ImageFloat *clone_image(const ImageFloat *img);
void normalize_image(ImageFloat *img);
void add_gaussian_noise(ImageFloat *img, float sigma);
```

**Format Planaire:**
```
Entrelacé:  [R0 G0 B0 R1 G1 B1 R2 G2 B2 ...]
Planaire:   [R0 R1 R2 ... Rn G0 G1 G2 ... Gn B0 B1 B2 ... Bn]
```

**Avantages:**
- Meilleure localité mémoire
- Vectorisation SIMD plus efficace
- Compatible avec MKL

---

### 3. filters.c/h (Filtres Gaussiens)

**Rôle:** Génération des noyaux de convolution

**Structure:**
```c
typedef struct {
    float *weights;   // Poids du noyau
    int size;         // Taille (size × size)
    float sigma;      // Paramètre gaussien
} Kernel;
```

**Fonctions:**
```c
// Noyau 2D (pour convolution spatiale et FFT)
Kernel *create_gaussian_kernel(int size, float sigma);

// Noyau 1D (pour convolution séparable)
float *create_gaussian_kernel_1d(int size, float sigma);
```

**Formules:**
- **Gaussienne 2D:** G(x,y,σ) = (1/(2πσ²)) × exp(-(x²+y²)/(2σ²))
- **Gaussienne 1D:** G(x,σ) = (1/√(2πσ²)) × exp(-x²/(2σ²))

**Normalisation:** Utilise `cblas_sscal` de MKL pour garantir Σ K = 1

---

### 4. mkl_ops.c/h (Opérations MKL)

**Rôle:** Cœur algorithmique avec optimisations MKL

#### A. Convolution Spatiale

**Version naïve:**
```c
ImageFloat *convolve_spatial(const ImageFloat *img, const Kernel *kernel);
```
- Implémentation directe de la formule mathématique
- Complexité: O(N × K²)

**Version optimisée BLAS:**
```c
ImageFloat *convolve_spatial_blas(const ImageFloat *img, const Kernel *kernel);
```
- Utilise `cblas_sdot` pour les produits scalaires
- Gain: 2-3× par vectorisation SIMD

#### B. Convolution Séparable

```c
ImageFloat *convolve_separable(const ImageFloat *img, 
                                const float *kernel_1d, 
                                int kernel_size);
```
- Décompose en deux passes 1D (horizontal + vertical)
- Complexité: O(2NK) au lieu de O(NK²)
- Optimal pour noyaux moyens (5×5 à 11×11)

#### C. Convolution FFT

```c
ImageFloat *convolve_fft(const ImageFloat *img, const Kernel *kernel);
```
- Utilise DFTI (Discrete Fourier Transform Interface)
- Théorème: I ∗ K ⟺ FFT(I) × FFT(K)
- Complexité: O(N log N)
- Optimal pour grands noyaux (≥ 11×11)

**Fonctions auxiliaires FFT:**
```c
void *fft_2d_forward(const float *img, int width, int height);
float *fft_2d_backward(void *fft_data, int width, int height);
void fft_multiply(void *fft1, const void *fft2, int w, int h);
```

---

### 5. io.c/h (Entrées/Sorties)

**Rôle:** Chargement et sauvegarde d'images

**Dépendance:** stb_image.h et stb_image_write.h (headers only)

**Fonctions:**
```c
// Chargement PNG/JPG
ImageFloat *load_image(const char *filename);

// Sauvegarde PNG
int save_image(const char *filename, const ImageFloat *img);

// Image synthétique pour tests
ImageFloat *create_test_image(int width, int height);
```

**Formats supportés:**
- Lecture: PNG, JPG, BMP, TGA, GIF
- Écriture: PNG (via stb_image_write)

---

## ⚙️ Configuration MKL

### Variables d'Environnement

```bash
# Chemin racine MKL
export MKLROOT=/opt/intel/oneapi/mkl/latest

# Bibliothèques dans le PATH
export LD_LIBRARY_PATH=$MKLROOT/lib/intel64:$LD_LIBRARY_PATH

# Nombre de threads
export MKL_NUM_THREADS=8

# Mode verbeux (debug)
export MKL_VERBOSE=1
```

### Flags de Compilation

```makefile
# Includes
MKL_INCLUDE = -I$(MKLROOT)/include

# Bibliothèques
MKL_LIBS = -L$(MKLROOT)/lib/intel64 \
           -lmkl_intel_lp64 \      # Interface LP64
           -lmkl_gnu_thread \      # Threading GNU
           -lmkl_core \            # Core MKL
           -lgomp -lpthread -lm -ldl
```

### Initialisation dans le Code

```c
// Définir le nombre de threads
mkl_set_num_threads(8);

// Allocation alignée (64 octets pour AVX-512)
float *data = (float *)mkl_malloc(size * sizeof(float), 64);

// Libération
mkl_free(data);
```

---

## 📊 Performances Typiques

**Configuration:** Intel Core i7-12700K, 8 cœurs, Image 1920×1080

| Méthode | Noyau 7×7 | Noyau 15×15 | Speedup |
|---------|-----------|-------------|---------|
| Spatiale (naïve) | 2850 ms | 12000 ms | 1.0× |
| Spatiale (BLAS) | 1200 ms | 5400 ms | 2.4× |
| **Séparable** | **85 ms** | 180 ms | **33.5×** |
| **FFT** | 65 ms | **70 ms** | **43.8×** |

**Observations:**
- Séparable est optimal pour noyaux 5×5 à 9×9
- FFT devient dominant à partir de 11×11
- BLAS apporte toujours un gain même sur la version naïve

---

## 🧪 Tests et Validation

### Tests Unitaires

```bash
./run_tests.sh
```

**Couverture:**
- 12+ tests fonctionnels
- Tests de cohérence mathématique
- Benchmarks de performance
- Tests de robustesse (différents paramètres)

### Démonstration

```bash
./demo.sh
```

**Génère:**
- Comparaison des 4 méthodes
- Tests avec différentes tailles de noyaux
- Tests avec différents sigmas
- Benchmarks automatiques

### Test Rapide

```bash
make test
# ou
./image_denoise --test
```

---

## 📚 Concepts Clés Implémentés

### 1. Format Planaire
- Optimisation de la localité mémoire
- Meilleure utilisation du cache CPU

### 2. Alignement Mémoire
- `mkl_malloc(..., 64)` pour AVX-512
- Accès mémoire optimisés

### 3. Vectorisation SIMD
- Instructions AVX-512 automatiques via MKL
- Traitement de 16 floats simultanés

### 4. Parallélisme Multi-Cœurs
- MKL parallélise automatiquement
- Scaling presque linéaire jusqu'à 8 cœurs

### 5. FFT Optimisée
- Plan FFT compilé avec `DftiCommitDescriptor`
- FFT réelle (exploitation de la symétrie hermitienne)

---

## 🎓 Valeur Pédagogique

Ce projet illustre:

**Mathématiques:**
- Convolution discrète
- Transformée de Fourier
- Séparabilité des filtres

**Algorithmique:**
- Analyse de complexité
- Trade-offs temps/mémoire
- Optimisations

**Programmation:**
- Langage C moderne
- Bibliothèques optimisées (MKL)
- Architecture modulaire

**Performance:**
- Profiling et benchmarking
- Parallélisme
- Instructions SIMD

---

## 📖 Pour Aller Plus Loin

### Extensions Possibles

1. **Autres Filtres:**
   - Filtre médian
   - Filtre bilatéral
   - Filtre de Wiener

2. **Algorithmes Avancés:**
   - NLM (Non-Local Means)
   - BM3D (Block-Matching 3D)
   - Réseaux de neurones (CNN)

3. **Optimisations:**
   - GPU avec CUDA/oneAPI
   - Traitement par tuiles
   - Précision mixte (FP16+FP32)

4. **Applications:**
   - Traitement vidéo temps réel
   - Batch processing
   - Interface graphique

---

## 👥 Équipe et Contacts

**Étudiants:**
- AFFOUKOU Prosper
- BOTCHI Parfait
- DOBOEVI Judicaël Karol

**Superviseur:**
- Dr. AGOSSOU Carlos

**Institution:**
- ENSGMM - UNSTIM, Abomey, Bénin

---

## 📄 Licence

Projet développé à des fins pédagogiques dans le cadre du cours d'Outils de Calcul Scientifique.

---

*Document de présentation - Janvier 2026*
