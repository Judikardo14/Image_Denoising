# INDEX DU PROJET
## Débruitage d'Images avec Intel MKL

Tous les fichiers pour votre projet sont prêts ! Voici le guide complet.

---

## 📦 CONTENU DU PACKAGE

### 🔵 Fichiers Source C (.c)
1. **main.c** (9.7 KB)
   - Programme principal avec benchmarks
   - Gestion des arguments en ligne de commande
   - Interface utilisateur

2. **image.c** (3.9 KB)
   - Manipulation d'images (création, clonage, conversion)
   - Format planaire ↔ entrelacé
   - Ajout de bruit gaussien

3. **filters.c** (2.8 KB)
   - Génération de noyaux gaussiens 2D et 1D
   - Normalisation avec MKL BLAS

4. **mkl_ops.c** (14 KB)
   - ⭐ Cœur du projet
   - 3 méthodes de convolution optimisées MKL
   - FFT avec DFTI

5. **io.c** (2.9 KB)
   - Chargement/sauvegarde d'images
   - Interface avec stb_image

### 🔷 Fichiers Header C (.h)
6. **image.h** (2.2 KB)
   - Structure ImageFloat
   - Prototypes des fonctions d'image

7. **filters.h** (1.4 KB)
   - Structure Kernel
   - Prototypes des filtres

8. **mkl_ops.h** (3.1 KB)
   - Prototypes des opérations MKL
   - Interfaces des 3 méthodes de convolution

9. **io.h** (895 B)
   - Prototypes I/O

### 🔧 Compilation
10. **Makefile** (3.3 KB)
    - Compilation automatisée
    - Téléchargement des headers stb_image
    - Cibles: all, test, clean, etc.

### 📚 Documentation
11. **README.md** (7.6 KB)
    - Guide utilisateur principal
    - Instructions d'utilisation
    - Exemples de commandes

12. **INSTALLATION.md** (8.3 KB)
    - Guide d'installation pas à pas
    - Installation de MKL
    - Dépannage

13. **TECHNIQUE.md** (11 KB)
    - Documentation technique complète
    - Fondements mathématiques
    - Détails d'implémentation MKL

14. **PRESENTATION.md** (9.8 KB)
    - Vue d'ensemble du projet
    - Description des modules
    - Architecture du code

15. **STB_IMAGE_NOTE.md** (5.6 KB)
    - Guide sur les bibliothèques stb_image
    - Téléchargement et utilisation

### 🚀 Scripts
16. **demo.sh** (6.2 KB, exécutable)
    - Démonstration automatisée
    - Tests avec différents paramètres
    - Génération de résultats

17. **run_tests.sh** (11 KB, exécutable)
    - Suite de tests unitaires
    - 12+ tests automatisés
    - Validation et benchmarks

18. **download_stb.sh** (274 B, exécutable)
    - Utilitaire de téléchargement des headers stb

---

## 🚀 DÉMARRAGE RAPIDE

### Étape 1: Installation de MKL

```bash
# Ubuntu/Debian
sudo apt install intel-mkl

# Ou télécharger depuis:
# https://www.intel.com/content/www/us/en/developer/tools/oneapi/onemkl.html

# Configurer l'environnement
export MKLROOT=/opt/intel/oneapi/mkl/latest
export LD_LIBRARY_PATH=$MKLROOT/lib/intel64:$LD_LIBRARY_PATH
```

### Étape 2: Compilation

```bash
# Naviguer dans le dossier du projet
cd /chemin/vers/image_denoise_mkl

# Compiler (télécharge automatiquement stb_image)
make

# Vérifier
./image_denoise -h
```

### Étape 3: Test Rapide

```bash
# Test avec image synthétique
./image_denoise --test

# Ou lancer la démo complète
./demo.sh
```

---

## 📖 ORDRE DE LECTURE RECOMMANDÉ

Pour comprendre le projet:

1. **README.md** - Vue d'ensemble et utilisation
2. **PRESENTATION.md** - Architecture et modules
3. **TECHNIQUE.md** - Détails mathématiques et algorithmiques
4. **Code source** - Dans cet ordre:
   - image.h / image.c
   - filters.h / filters.c
   - mkl_ops.h / mkl_ops.c
   - io.h / io.c
   - main.c

Pour installer:

1. **INSTALLATION.md** - Guide complet étape par étape
2. **STB_IMAGE_NOTE.md** - Si problème avec stb_image

---

## 🎯 UTILISATION COURANTE

### Débruiter une Image

```bash
# Débruitage léger
./image_denoise -i photo.jpg -k 5 -s 1.5 -n 15.0

# Débruitage moyen
./image_denoise -i photo.jpg -k 7 -s 2.0 -n 20.0

# Débruitage fort
./image_denoise -i photo.jpg -k 11 -s 3.5 -n 30.0
```

### Comparer les Méthodes

```bash
# Toutes les méthodes
./image_denoise -i photo.jpg -m all

# Une méthode spécifique
./image_denoise -i photo.jpg -m fft
./image_denoise -i photo.jpg -m separable
./image_denoise -i photo.jpg -m spatial
```

### Tests Automatisés

```bash
# Démonstration complète
./demo.sh

# Tests unitaires
./run_tests.sh
```

---

## 🔍 STRUCTURE DES FICHIERS

```
image_denoise_mkl/
├── Source Code
│   ├── main.c, image.c, filters.c, mkl_ops.c, io.c
│   └── *.h (headers correspondants)
│
├── Build System
│   └── Makefile
│
├── Documentation
│   ├── README.md (à lire en premier ⭐)
│   ├── INSTALLATION.md
│   ├── TECHNIQUE.md
│   ├── PRESENTATION.md
│   └── STB_IMAGE_NOTE.md
│
├── Scripts
│   ├── demo.sh
│   ├── run_tests.sh
│   └── download_stb.sh
│
└── External (téléchargés automatiquement)
    ├── stb_image.h
    └── stb_image_write.h
```

---

## ⚠️ PRÉREQUIS

### Obligatoires
- ✅ GCC (ou compilateur C compatible)
- ✅ Intel MKL (Math Kernel Library)
- ✅ GNU Make

### Optionnels
- 📦 wget ou curl (pour télécharger stb_image)
- 📦 Images de test (.jpg ou .png)

---

## 💡 CONSEILS

### Premier Lancement

1. Commencez par lire **README.md**
2. Suivez **INSTALLATION.md** pour installer MKL
3. Compilez avec `make`
4. Testez avec `./image_denoise --test`

### Problèmes Courants

- **"mkl.h not found"** → Voir INSTALLATION.md, section "Dépannage"
- **"stb_image.h not found"** → Voir STB_IMAGE_NOTE.md
- **Programme lent** → Vérifier le nombre de threads avec `-t 8`

### Pour Approfondir

- **Comprendre le code**: Lire PRESENTATION.md
- **Mathématiques**: Consulter TECHNIQUE.md
- **Modifier le code**: Étudier les commentaires dans les .c/.h

---

## 📊 RÉSULTATS ATTENDUS

Après un test réussi, vous obtiendrez:

```
output_noisy.png           # Image bruitée
output_spatial.png         # Résultat convolution spatiale
output_spatial_blas.png    # Résultat spatial avec BLAS
output_separable.png       # Résultat convolution séparable
output_fft.png             # Résultat convolution FFT
```

Et un tableau comparatif des performances:

```
╔═══════════════════════════╦═════════════╦═════════════════╗
║ Méthode                   ║ Temps (ms)  ║ Accélération    ║
╠═══════════════════════════╬═════════════╬═════════════════╣
║ Spatial (naïve)           ║    2850.00  ║       1.00x     ║
║ Spatial (BLAS)            ║    1200.00  ║       2.38x     ║
║ Séparable                 ║      85.00  ║      33.53x     ║
║ FFT                       ║      65.00  ║      43.85x     ║
╚═══════════════════════════╩═════════════╩═════════════════╝
```

---

## 🎓 CONTEXTE ACADÉMIQUE

**Projet:** Débruitage d'Images avec Intel MKL  
**Cours:** Outils de Calcul Scientifique  
**Institution:** ENSGMM - UNSTIM, Abomey, Bénin  
**Date:** Janvier 2026

**Étudiants:**
- AFFOUKOU Prosper
- BOTCHI Parfait  
- DOBOEVI Judicaël Karol

**Superviseur:** Dr. AGOSSOU Carlos

---

## 📞 SUPPORT

En cas de problème:

1. Consultez **INSTALLATION.md** (section Dépannage)
2. Vérifiez les logs d'erreur
3. Testez avec `./image_denoise --test` pour isoler le problème
4. Contactez le superviseur si nécessaire

---

## ✅ CHECKLIST DE DÉMARRAGE

- [ ] Intel MKL installé
- [ ] Variable MKLROOT définie
- [ ] Tous les fichiers .c et .h présents
- [ ] Makefile présent
- [ ] `make` exécuté avec succès
- [ ] `./image_denoise -h` fonctionne
- [ ] `./image_denoise --test` génère des images

Si toutes les cases sont cochées, vous êtes prêt ! 🚀

---

## 🎉 BON TRAITEMENT D'IMAGES !

N'hésitez pas à explorer le code, modifier les paramètres, et expérimenter avec vos propres images.

**Rappel:** La documentation complète est dans README.md, INSTALLATION.md, TECHNIQUE.md et PRESENTATION.md.

---

*Index créé le 2 février 2026*  
*Projet: Débruitage d'Images avec Intel MKL - ENSGMM*
