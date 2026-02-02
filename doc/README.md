# Débruitage d'Images avec Intel MKL

Projet d'Outils de Calcul Scientifique  
École Nationale Supérieure de Génie Mathématique et Modélisation  
UNSTIM - Abomey  
Janvier 2026

**Présenté par:**
- AFFOUKOU Prosper
- BOTCHI Parfait
- DOBOEVI Judicaël Karol

**Sous la supervision de:** Dr. AGOSSOU Carlos

---

## 📋 Description

Ce projet implémente trois méthodes de débruitage d'images par convolution gaussienne, optimisées avec la bibliothèque Intel Math Kernel Library (MKL):

1. **Convolution Spatiale**: Application directe de la formule mathématique
2. **Convolution Séparable**: Décomposition en deux passes 1D (horizontal + vertical)
3. **Convolution par FFT**: Utilisation du théorème de convolution dans le domaine fréquentiel

## 🎯 Objectifs Pédagogiques

- Comprendre les fondements mathématiques du débruitage d'images
- Maîtriser l'architecture d'un système de traitement d'images en C
- Utiliser concrètement les fonctions MKL (BLAS, DFTI)
- Analyser et comparer les performances de différentes méthodes

## 🚀 Fonctionnalités

- ✅ Chargement/sauvegarde d'images PNG/JPG
- ✅ Ajout de bruit gaussien contrôlé
- ✅ Trois méthodes de convolution optimisées
- ✅ Format planaire pour optimisation mémoire
- ✅ Benchmarking automatique
- ✅ Parallélisme multi-cœurs transparent
- ✅ Instructions SIMD (AVX-512)

## 📦 Prérequis

### Système d'exploitation
- Linux (Ubuntu 20.04+, Fedora, etc.)
- macOS (avec Homebrew)
- Windows (WSL2 recommandé)

### Dépendances

1. **Intel MKL** (Math Kernel Library)
   ```bash
   # Ubuntu/Debian
   sudo apt-get install intel-mkl
   
   # Ou télécharger depuis:
   # https://www.intel.com/content/www/us/en/developer/tools/oneapi/onemkl.html
   ```

2. **Compilateur GCC**
   ```bash
   sudo apt-get install build-essential
   ```

3. **Bibliothèques stb_image** (téléchargement automatique)
   - stb_image.h
   - stb_image_write.h

## 🛠️ Installation

### 1. Cloner/Télécharger le projet

```bash
cd ~
mkdir image_denoise_mkl
cd image_denoise_mkl
# Copier tous les fichiers .c, .h et le Makefile ici
```

### 2. Configuration de MKL

Si MKL est installé dans un emplacement non standard:

```bash
export MKLROOT=/chemin/vers/intel/mkl
```

### 3. Compilation

```bash
make clean
make
```

Le Makefile téléchargera automatiquement les headers stb_image si nécessaire.

### 4. Vérification

```bash
./image_denoise -h
```

## 💻 Utilisation

### Test Rapide avec Image Synthétique

```bash
./image_denoise --test
```

Génère automatiquement:
- `output_noisy.png` - Image bruitée
- `output_spatial.png` - Résultat méthode spatiale
- `output_spatial_blas.png` - Résultat spatial avec BLAS
- `output_separable.png` - Résultat séparable
- `output_fft.png` - Résultat FFT

### Utilisation avec une Image Personnalisée

```bash
./image_denoise -i mon_image.jpg -k 11 -s 3.0 -n 25.0 -o resultat
```

**Options:**
- `-i <file>` : Image d'entrée (PNG/JPG)
- `-o <prefix>` : Préfixe des fichiers de sortie (défaut: output)
- `-k <size>` : Taille du noyau gaussien (défaut: 7)
- `-s <sigma>` : Sigma du filtre (défaut: 2.0)
- `-n <sigma>` : Sigma du bruit à ajouter (défaut: 20.0)
- `-t <threads>` : Nombre de threads (défaut: auto)
- `-m <method>` : Méthode spécifique (spatial|spatial_blas|separable|fft|all)

### Exemples

**Débruitage léger:**
```bash
./image_denoise -i photo.jpg -k 5 -s 1.5 -n 15.0
```

**Débruitage fort:**
```bash
./image_denoise -i photo.jpg -k 15 -s 4.0 -n 30.0
```

**Comparer seulement FFT vs Séparable:**
```bash
./image_denoise -i photo.jpg -k 11 -m fft
./image_denoise -i photo.jpg -k 11 -m separable
```

## 📊 Résultats de Performance

Benchmarks typiques (Intel Core i7-12700K, 8 cœurs, image 1920×1080):

| Méthode | Noyau 7×7 | Noyau 15×15 | Accélération |
|---------|-----------|-------------|--------------|
| Spatial naïve | 2850 ms | 12000 ms | 1× |
| Spatial BLAS | 1200 ms | 5400 ms | 2.4× |
| Séparable | 85 ms | 180 ms | 33.5× |
| FFT | 65 ms | 70 ms | **43.8×** |

**Observations:**
- Petits noyaux (≤7×7): Séparable optimal
- Grands noyaux (≥11×11): FFT dominant
- BLAS apporte un gain 2-3× même sur la version naïve

## 🏗️ Architecture du Projet

```
.
├── main.c              # Programme principal
├── image.c/h           # Structures et manipulation d'images
├── filters.c/h         # Génération des noyaux gaussiens
├── mkl_ops.c/h         # Opérations MKL (convolutions)
├── io.c/h              # Lecture/écriture d'images
├── Makefile            # Compilation
├── README.md           # Cette documentation
├── stb_image.h         # Header stb (téléchargé automatiquement)
└── stb_image_write.h   # Header stb (téléchargé automatiquement)
```

### Modules Principaux

**image.c** - Gestion des images
- Structure `ImageFloat` en format planaire
- Conversions entrelacé ↔ planaire
- Ajout de bruit gaussien
- Normalisation

**filters.c** - Filtres
- Noyaux gaussiens 2D et 1D
- Normalisation avec `cblas_sscal`

**mkl_ops.c** - Cœur algorithmique
- Convolution spatiale (naïve + BLAS)
- Convolution séparable
- Convolution FFT (DFTI)

**io.c** - Entrées/Sorties
- Chargement PNG/JPG avec stb_image
- Sauvegarde PNG avec stb_image_write

## 🔬 Concepts Théoriques

### 1. Convolution 2D

```
(I * K)(x,y) = Σ Σ I(x+i, y+j) · K(i,j)
```

### 2. Filtre Gaussien

```
G(x,y,σ) = (1/(2πσ²)) · exp(-(x²+y²)/(2σ²))
```

### 3. Séparabilité

```
K(x,y) = kₓ(x) · kᵧ(y)
⟹ I * K = (I * kₓ) * kᵧ
```

Gain: O(N·K²) → O(2·N·K)

### 4. Théorème de Convolution

```
I * K ⟺ F(I) · F(K)
Convolution spatiale = Multiplication fréquentielle
```

## 🧪 Tests et Validation

### Test Unitaire Rapide
```bash
make test
```

### Tests Manuels
```bash
# Test avec différents noyaux
for k in 3 5 7 9 11 15; do
    ./image_denoise --test -k $k -s 2.0 -o test_k${k}
done

# Test avec différents sigmas
for s in 1.0 2.0 3.0 4.0; do
    ./image_denoise --test -k 7 -s $s -o test_s${s}
done
```

## 🐛 Dépannage

### Erreur: "mkl.h: No such file or directory"

MKL n'est pas trouvé. Solutions:
```bash
# Option 1: Définir MKLROOT
export MKLROOT=/opt/intel/oneapi/mkl/latest

# Option 2: Installer MKL
sudo apt-get install intel-mkl

# Option 3: Utiliser le compilateur Intel
source /opt/intel/oneapi/setvars.sh
```

### Erreur: "cannot find -lmkl_intel_lp64"

Chemin des bibliothèques MKL incorrect:
```bash
export LD_LIBRARY_PATH=$MKLROOT/lib/intel64:$LD_LIBRARY_PATH
```

### Performances Faibles

1. Vérifier le nombre de threads:
```bash
./image_denoise --test -t 8  # Forcer 8 threads
```

2. Vérifier la configuration MKL:
```bash
make mkl_info
```

3. Compiler avec optimisations:
```bash
CFLAGS="-O3 -march=native" make
```

## 📚 Références

1. Gonzalez, R. C., & Woods, R. E. (2018). *Digital Image Processing* (4th ed.). Pearson.
2. Cooley, J. W., & Tukey, J. W. (1965). *An algorithm for the machine calculation of complex Fourier series*. Mathematics of Computation.
3. Intel. (2023). *Intel® Math Kernel Library*. https://intel.com/mkl
4. Buades, A., Coll, B., & Morel, J. M. (2005). *A non-local algorithm for image denoising*. IEEE CVPR.

## 📜 Licence

Ce projet est développé à des fins pédagogiques dans le cadre du cours d'Outils de Calcul Scientifique à l'ENSGMM.

## 🤝 Contributions

Pour toute question ou suggestion:
- Email: judikardo@gmail.com
- Superviseur: Dr. AGOSSOU Carlos

---

**Note:** Ce projet utilise Intel MKL, qui nécessite une licence appropriée pour un usage commercial. Consultez la documentation Intel pour plus d'informations.
