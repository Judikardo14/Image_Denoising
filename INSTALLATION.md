# Guide d'Installation Détaillé
## Débruitage d'Images avec Intel MKL

Ce guide vous accompagne pas à pas dans l'installation et la configuration du projet.

---

## Table des Matières

1. [Installation d'Intel MKL](#1-installation-dintel-mkl)
2. [Installation des Outils de Développement](#2-installation-des-outils-de-développement)
3. [Téléchargement du Projet](#3-téléchargement-du-projet)
4. [Compilation](#4-compilation)
5. [Tests](#5-tests)
6. [Dépannage](#6-dépannage)

---

## 1. Installation d'Intel MKL

Intel MKL (Math Kernel Library) est disponible gratuitement via Intel oneAPI.

### Option A: Installation via APT (Ubuntu/Debian)

```bash
# Ajouter le dépôt Intel
wget -O- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB \
  | gpg --dearmor | sudo tee /usr/share/keyrings/oneapi-archive-keyring.gpg > /dev/null

echo "deb [signed-by=/usr/share/keyrings/oneapi-archive-keyring.gpg] \
  https://apt.repos.intel.com/oneapi all main" \
  | sudo tee /etc/apt/sources.list.d/oneAPI.list

# Installer MKL
sudo apt update
sudo apt install intel-oneapi-mkl intel-oneapi-mkl-devel

# Configurer l'environnement
echo "source /opt/intel/oneapi/setvars.sh" >> ~/.bashrc
source ~/.bashrc
```

### Option B: Installation via Script Intel (Toutes Plateformes)

```bash
# Télécharger l'installateur
cd ~/Downloads
wget https://registrationcenter-download.intel.com/akdlm/IRC_NAS/...[URL depuis intel.com]

# Installer
sudo bash l_BaseKit_*.sh

# Configurer
source /opt/intel/oneapi/setvars.sh
```

### Option C: Installation Manuelle (Plus Simple)

Si les options ci-dessus ne marchent pas:

```bash
# Ubuntu/Debian
sudo apt-get install libmkl-dev

# Fedora/RHEL
sudo dnf install mkl mkl-devel

# macOS (via Homebrew)
brew install intel-oneapi-mkl
```

### Vérification de l'Installation

```bash
# Vérifier que MKL est installé
ls /opt/intel/oneapi/mkl/latest/lib/intel64/

# Doit afficher: libmkl_core.so, libmkl_intel_lp64.so, etc.

# Vérifier les variables d'environnement
echo $MKLROOT
# Doit afficher: /opt/intel/oneapi/mkl/latest (ou similaire)
```

---

## 2. Installation des Outils de Développement

### Ubuntu/Debian

```bash
sudo apt update
sudo apt install build-essential gcc g++ make
sudo apt install wget curl  # Pour télécharger stb_image
```

### Fedora/RHEL

```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install gcc gcc-c++ make wget curl
```

### macOS

```bash
# Installer Xcode Command Line Tools
xcode-select --install

# Ou installer via Homebrew
brew install gcc make wget
```

---

## 3. Téléchargement du Projet

### Option A: Depuis une Archive

```bash
# Créer le dossier projet
mkdir -p ~/projets/image_denoise_mkl
cd ~/projets/image_denoise_mkl

# Copier tous les fichiers fournis:
# - main.c, image.c, image.h, filters.c, filters.h
# - mkl_ops.c, mkl_ops.h, io.c, io.h
# - Makefile, README.md, demo.sh
# - INSTALLATION.md (ce fichier)
```

### Option B: Depuis Git (si disponible)

```bash
git clone <URL_DU_REPO>
cd image_denoise_mkl
```

---

## 4. Compilation

### Étape 1: Configuration de l'Environnement

```bash
# Si MKL n'est pas dans le chemin par défaut
export MKLROOT=/opt/intel/oneapi/mkl/latest

# Ajouter les bibliothèques au chemin de chargement
export LD_LIBRARY_PATH=$MKLROOT/lib/intel64:$LD_LIBRARY_PATH
```

**Astuce**: Ajouter ces lignes à `~/.bashrc` pour les rendre permanentes.

### Étape 2: Téléchargement des Headers stb_image

Le Makefile tente de télécharger automatiquement les headers. Si cela échoue:

```bash
# Téléchargement manuel
wget https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
wget https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

# Ou avec curl
curl -O https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
curl -O https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
```

### Étape 3: Compilation

```bash
# Vérifier la configuration MKL
make mkl_info

# Nettoyer les anciens builds
make clean

# Compiler
make

# Si tout se passe bien, vous devriez voir:
# Compilation de main.c...
# Compilation de image.c...
# ...
# Édition des liens avec Intel MKL...
# Compilation terminée: image_denoise
```

### Erreurs Courantes lors de la Compilation

#### Erreur: "mkl.h: No such file or directory"

**Solution**:
```bash
# Vérifier où est installé MKL
find /opt/intel -name "mkl.h" 2>/dev/null

# Ajuster MKLROOT
export MKLROOT=/chemin/trouvé/mkl
make clean
make
```

#### Erreur: "cannot find -lmkl_intel_lp64"

**Solution**:
```bash
# Vérifier les bibliothèques
ls $MKLROOT/lib/intel64/libmkl_*.so

# Si absentes, réinstaller MKL
# Si présentes, ajuster LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$MKLROOT/lib/intel64:$LD_LIBRARY_PATH
```

#### Erreur: "stb_image.h: No such file or directory"

**Solution**:
```bash
# Télécharger manuellement (voir Étape 2 ci-dessus)
```

---

## 5. Tests

### Test 1: Afficher l'Aide

```bash
./image_denoise -h
```

**Résultat attendu**: Affichage du menu d'aide.

### Test 2: Test Rapide avec Image Synthétique

```bash
./image_denoise --test
```

**Résultat attendu**:
- Création de 5 images PNG dans le dossier courant
- Affichage du tableau comparatif des performances
- Temps d'exécution < 5 secondes (pour une image 512×512)

### Test 3: Démonstration Complète

```bash
./demo.sh
```

**Résultat attendu**:
- Création d'un dossier `demo_results_XXX`
- Exécution de 4 séries de tests
- Génération de ~20 images de résultats

### Test 4: Test avec une Image Personnelle

```bash
# Télécharger une image de test
wget https://picsum.photos/800/600 -O test_image.jpg

# Débruiter
./image_denoise -i test_image.jpg -k 7 -s 2.0 -n 25.0
```

---

## 6. Dépannage

### Problème: Programme très lent

**Causes possibles**:
1. MKL n'utilise qu'un seul thread
2. Optimisations de compilation désactivées

**Solutions**:
```bash
# Forcer le nombre de threads
./image_denoise --test -t 8

# Recompiler avec optimisations natives
make clean
CFLAGS="-O3 -march=native -fopenmp" make

# Vérifier les threads MKL
export MKL_NUM_THREADS=8
./image_denoise --test
```

### Problème: Erreur de segmentation

**Causes possibles**:
1. Problème d'alignement mémoire
2. Conflit de version MKL

**Solutions**:
```bash
# Vérifier la version de MKL
ls -l $MKLROOT/lib/intel64/

# Tester avec Valgrind
sudo apt install valgrind
valgrind --leak-check=full ./image_denoise --test

# Compiler en mode debug
make clean
CFLAGS="-g -O0" make
gdb ./image_denoise
```

### Problème: Images de sortie incorrectes

**Vérifications**:
```bash
# Vérifier les dimensions
file output_*.png

# Vérifier la luminosité (doit être entre 0-255)
# Utiliser un visualiseur d'images

# Tester avec différents paramètres
./image_denoise --test -k 5 -s 1.0 -n 10.0
```

---

## 7. Optimisations Avancées

### A. Optimisation des Performances

```bash
# Compiler avec le compilateur Intel (si disponible)
CC=icc make

# Optimisations agressives
CFLAGS="-O3 -xHost -ipo -march=native" make
```

### B. Profiling

```bash
# Installer perf
sudo apt install linux-tools-generic

# Profiler le programme
perf record -g ./image_denoise --test -m fft
perf report

# Ou utiliser gprof
gcc -pg -O2 *.c -o image_denoise $MKL_LIBS
./image_denoise --test
gprof ./image_denoise > profile.txt
```

### C. Ajustement MKL

```bash
# Variables d'environnement MKL utiles
export MKL_NUM_THREADS=8          # Forcer 8 threads
export MKL_DYNAMIC=FALSE          # Désactiver l'ajustement dynamique
export MKL_VERBOSE=1              # Mode verbeux pour debug
```

---

## 8. Ressources Supplémentaires

### Documentation Intel MKL

- **Guide de l'utilisateur**: https://www.intel.com/content/www/us/en/docs/onemkl/
- **Exemples**: https://github.com/oneapi-src/oneMKL
- **Forum**: https://community.intel.com/

### Documentation stb_image

- **GitHub**: https://github.com/nothings/stb
- **Documentation**: Commentaires dans les headers

### Traitement d'Images

- Gonzalez & Woods: "Digital Image Processing"
- OpenCV Documentation: https://docs.opencv.org/

---

## 9. Support

Pour toute question ou problème:

1. Vérifier ce guide d'installation
2. Consulter le README.md
3. Contacter le superviseur: Dr. AGOSSOU Carlos
4. Consulter les forums Intel MKL

---

**Bonne installation et bon débruitage d'images ! 🚀**

---

*Document créé pour le projet d'Outils de Calcul Scientifique*  
*ENSGMM - Janvier 2026*
