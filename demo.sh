#!/bin/bash

# Script de démonstration pour le projet de débruitage d'images
# ENSGMM - Outils de Calcul Scientifique

set -e  # Arrêter en cas d'erreur

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║    DÉMONSTRATION: DÉBRUITAGE D'IMAGES AVEC INTEL MKL          ║"
echo "║    ENSGMM - Janvier 2026                                      ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Vérifier que le programme est compilé
if [ ! -f "./image_denoise" ]; then
    echo "⚠️  Programme non trouvé. Compilation..."
    make clean
    make
    echo ""
fi

# Créer un dossier pour les résultats
RESULTS_DIR="demo_results_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$RESULTS_DIR"
cd "$RESULTS_DIR"

echo "📁 Résultats dans: $RESULTS_DIR"
echo ""

# ============================================================================
# Test 1: Image synthétique avec différentes méthodes
# ============================================================================

echo "════════════════════════════════════════════════════════════════"
echo "Test 1: Comparaison des méthodes (image synthétique 512x512)"
echo "════════════════════════════════════════════════════════════════"
echo ""

../image_denoise --test -k 7 -s 2.0 -n 20.0 -o test1 -m all

echo ""
echo "✅ Test 1 terminé. Images générées:"
ls -lh test1_*.png
echo ""

# ============================================================================
# Test 2: Influence de la taille du noyau
# ============================================================================

echo "════════════════════════════════════════════════════════════════"
echo "Test 2: Influence de la taille du noyau (méthode séparable)"
echo "════════════════════════════════════════════════════════════════"
echo ""

for kernel_size in 3 5 7 9 11 15; do
    echo "  → Noyau ${kernel_size}x${kernel_size}..."
    ../image_denoise --test -k $kernel_size -s 2.0 -n 20.0 -o "test2_k${kernel_size}" -m separable -t 4
done

echo ""
echo "✅ Test 2 terminé. Comparaison des tailles de noyau:"
ls -lh test2_k*_separable.png
echo ""

# ============================================================================
# Test 3: Influence du sigma (intensité du lissage)
# ============================================================================

echo "════════════════════════════════════════════════════════════════"
echo "Test 3: Influence du sigma (intensité du lissage)"
echo "════════════════════════════════════════════════════════════════"
echo ""

for sigma in 0.5 1.0 2.0 3.0 4.0; do
    echo "  → Sigma = $sigma..."
    ../image_denoise --test -k 7 -s $sigma -n 20.0 -o "test3_s${sigma}" -m fft -t 4
done

echo ""
echo "✅ Test 3 terminé. Comparaison des sigmas:"
ls -lh test3_s*_fft.png
echo ""

# ============================================================================
# Test 4: Benchmark de performance (si assez rapide)
# ============================================================================

echo "════════════════════════════════════════════════════════════════"
echo "Test 4: Benchmark de performance"
echo "════════════════════════════════════════════════════════════════"
echo ""

echo "Méthode spatiale (noyau 5x5):"
time ../image_denoise --test -k 5 -s 1.5 -o bench_spatial -m spatial -t 4 2>&1 | grep "Temps:"

echo ""
echo "Méthode séparable (noyau 5x5):"
time ../image_denoise --test -k 5 -s 1.5 -o bench_sep -m separable -t 4 2>&1 | grep "Temps:"

echo ""
echo "Méthode FFT (noyau 5x5):"
time ../image_denoise --test -k 5 -s 1.5 -o bench_fft -m fft -t 4 2>&1 | grep "Temps:"

echo ""
echo "✅ Test 4 terminé."
echo ""

# ============================================================================
# Résumé
# ============================================================================

echo "════════════════════════════════════════════════════════════════"
echo "DÉMONSTRATION TERMINÉE"
echo "════════════════════════════════════════════════════════════════"
echo ""
echo "📊 Résumé:"
echo "  - Test 1: Comparaison des 4 méthodes"
echo "  - Test 2: Influence de la taille du noyau (6 tailles)"
echo "  - Test 3: Influence du sigma (5 valeurs)"
echo "  - Test 4: Benchmark de performance"
echo ""
echo "📁 Tous les résultats sont dans: $RESULTS_DIR"
echo "   Nombre total d'images: $(ls -1 *.png 2>/dev/null | wc -l)"
echo ""
echo "💡 Pour visualiser les résultats:"
echo "   cd $RESULTS_DIR"
echo "   # Utiliser un visualiseur d'images (eog, feh, etc.)"
echo ""
echo "🎓 Observations à faire:"
echo "  1. Comparer visuellement la qualité du débruitage"
echo "  2. Noter les différences de temps d'exécution"
echo "  3. Observer l'effet de la taille du noyau"
echo "  4. Analyser l'impact du paramètre sigma"
echo ""
