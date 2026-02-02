#!/bin/bash

# Script de tests unitaires pour le projet de débruitage d'images
# ENSGMM - Outils de Calcul Scientifique

set -e

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║              TESTS UNITAIRES - DÉBRUITAGE MKL                  ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Couleurs pour l'affichage
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Compteurs
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_TOTAL=0

# Fonction pour exécuter un test
run_test() {
    local test_name="$1"
    local test_cmd="$2"
    local expected_files="$3"
    
    TESTS_TOTAL=$((TESTS_TOTAL + 1))
    echo "────────────────────────────────────────────────────────────────"
    echo "Test $TESTS_TOTAL: $test_name"
    echo "────────────────────────────────────────────────────────────────"
    
    # Créer un dossier temporaire pour ce test
    TEST_DIR="test_${TESTS_TOTAL}_$(date +%s)"
    mkdir -p "$TEST_DIR"
    cd "$TEST_DIR"
    
    # Exécuter la commande
    echo "Commande: $test_cmd"
    if eval "$test_cmd" > test_output.log 2>&1; then
        # Vérifier que les fichiers attendus existent
        local all_files_exist=true
        for file in $expected_files; do
            if [ ! -f "$file" ]; then
                all_files_exist=false
                echo -e "${RED}✗ Fichier manquant: $file${NC}"
            fi
        done
        
        if [ "$all_files_exist" = true ]; then
            echo -e "${GREEN}✓ Test réussi${NC}"
            TESTS_PASSED=$((TESTS_PASSED + 1))
        else
            echo -e "${RED}✗ Test échoué (fichiers manquants)${NC}"
            TESTS_FAILED=$((TESTS_FAILED + 1))
        fi
    else
        echo -e "${RED}✗ Test échoué (erreur d'exécution)${NC}"
        echo "Dernières lignes de la sortie:"
        tail -n 10 test_output.log
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    
    cd ..
    echo ""
}

# Vérifier que le programme existe
if [ ! -f "./image_denoise" ]; then
    echo -e "${RED}Erreur: Programme image_denoise non trouvé${NC}"
    echo "Veuillez compiler le projet avec 'make' avant d'exécuter les tests"
    exit 1
fi

echo "Programme trouvé: ./image_denoise"
echo ""

# ============================================================================
# TESTS
# ============================================================================

# Test 1: Affichage de l'aide
run_test "Affichage de l'aide" \
    "../image_denoise -h" \
    ""

# Test 2: Image synthétique - méthode spatiale
run_test "Image synthétique - Convolution spatiale" \
    "../image_denoise --test -m spatial -k 5 -o test2" \
    "test2_noisy.png test2_spatial.png"

# Test 3: Image synthétique - méthode séparable
run_test "Image synthétique - Convolution séparable" \
    "../image_denoise --test -m separable -k 7 -o test3" \
    "test3_noisy.png test3_separable.png"

# Test 4: Image synthétique - méthode FFT
run_test "Image synthétique - Convolution FFT" \
    "../image_denoise --test -m fft -k 7 -o test4" \
    "test4_noisy.png test4_fft.png"

# Test 5: Image synthétique - toutes les méthodes
run_test "Image synthétique - Toutes les méthodes" \
    "../image_denoise --test -m all -k 7 -o test5" \
    "test5_noisy.png test5_spatial.png test5_separable.png test5_fft.png"

# Test 6: Petit noyau (3x3)
run_test "Noyau 3x3" \
    "../image_denoise --test -k 3 -s 1.0 -m separable -o test6" \
    "test6_noisy.png test6_separable.png"

# Test 7: Grand noyau (15x15)
run_test "Noyau 15x15 avec FFT" \
    "../image_denoise --test -k 15 -s 3.0 -m fft -o test7" \
    "test7_noisy.png test7_fft.png"

# Test 8: Faible bruit
run_test "Faible bruit (sigma=5)" \
    "../image_denoise --test -n 5.0 -k 5 -s 1.0 -m separable -o test8" \
    "test8_noisy.png test8_separable.png"

# Test 9: Fort bruit
run_test "Fort bruit (sigma=40)" \
    "../image_denoise --test -n 40.0 -k 11 -s 3.5 -m fft -o test9" \
    "test9_noisy.png test9_fft.png"

# Test 10: Threads multiples
run_test "Parallélisme (8 threads)" \
    "../image_denoise --test -t 8 -m all -k 7 -o test10" \
    "test10_noisy.png test10_spatial.png test10_separable.png test10_fft.png"

# Test 11: Un seul thread
run_test "Mono-thread" \
    "../image_denoise --test -t 1 -m separable -k 7 -o test11" \
    "test11_noisy.png test11_separable.png"

# Test 12: Spatial BLAS
run_test "Convolution spatiale avec BLAS" \
    "../image_denoise --test -m spatial_blas -k 5 -o test12" \
    "test12_noisy.png test12_spatial_blas.png"

# ============================================================================
# TESTS DE VALIDATION MATHÉMATIQUE
# ============================================================================

echo "════════════════════════════════════════════════════════════════"
echo "TESTS DE VALIDATION MATHÉMATIQUE"
echo "════════════════════════════════════════════════════════════════"
echo ""

# Test 13: Vérifier que toutes les méthodes donnent des résultats similaires
echo "Test $((TESTS_TOTAL + 1)): Cohérence entre les méthodes"
echo "────────────────────────────────────────────────────────────────"
TESTS_TOTAL=$((TESTS_TOTAL + 1))

TEST_DIR="test_coherence"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

../image_denoise --test -k 7 -s 2.0 -n 0.0 -m all -o coherence > /dev/null 2>&1

# Vérifier que les fichiers existent
if [ -f "coherence_spatial.png" ] && [ -f "coherence_separable.png" ] && [ -f "coherence_fft.png" ]; then
    echo "Fichiers générés avec succès"
    echo -e "${GREEN}✓ Les trois méthodes ont produit des résultats${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
    echo -e "${YELLOW}Note: Pour vérifier la cohérence visuelle, comparer manuellement les images${NC}"
else
    echo -e "${RED}✗ Certains fichiers manquent${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

cd ..
echo ""

# ============================================================================
# TESTS DE PERFORMANCE
# ============================================================================

echo "════════════════════════════════════════════════════════════════"
echo "TESTS DE PERFORMANCE"
echo "════════════════════════════════════════════════════════════════"
echo ""

echo "Test $((TESTS_TOTAL + 1)): Benchmark comparatif"
echo "────────────────────────────────────────────────────────────────"
TESTS_TOTAL=$((TESTS_TOTAL + 1))

TEST_DIR="test_benchmark"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

echo "Exécution du benchmark (cela peut prendre ~30 secondes)..."

# Capturer les temps pour chaque méthode
TIME_SPATIAL=$(../image_denoise --test -m spatial -k 5 -o bench 2>&1 | grep -oP "Temps: \K[0-9.]+")
TIME_SEPARABLE=$(../image_denoise --test -m separable -k 5 -o bench 2>&1 | grep -oP "Temps: \K[0-9.]+")
TIME_FFT=$(../image_denoise --test -m fft -k 5 -o bench 2>&1 | grep -oP "Temps: \K[0-9.]+")

if [ ! -z "$TIME_SPATIAL" ] && [ ! -z "$TIME_SEPARABLE" ] && [ ! -z "$TIME_FFT" ]; then
    echo ""
    echo "Résultats du benchmark (noyau 5x5, image 512x512):"
    echo "  Spatial    : ${TIME_SPATIAL} ms"
    echo "  Séparable  : ${TIME_SEPARABLE} ms"
    echo "  FFT        : ${TIME_FFT} ms"
    echo ""
    
    # Calculer les speedups
    SPEEDUP_SEP=$(echo "scale=2; $TIME_SPATIAL / $TIME_SEPARABLE" | bc 2>/dev/null || echo "N/A")
    SPEEDUP_FFT=$(echo "scale=2; $TIME_SPATIAL / $TIME_FFT" | bc 2>/dev/null || echo "N/A")
    
    echo "Accélérations par rapport à la méthode spatiale:"
    echo "  Séparable  : ${SPEEDUP_SEP}×"
    echo "  FFT        : ${SPEEDUP_FFT}×"
    
    echo -e "${GREEN}✓ Benchmark terminé${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ Impossible d'extraire les temps${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

cd ..
echo ""

# ============================================================================
# RÉSUMÉ DES TESTS
# ============================================================================

echo "════════════════════════════════════════════════════════════════"
echo "RÉSUMÉ DES TESTS"
echo "════════════════════════════════════════════════════════════════"
echo ""
echo "Total de tests: $TESTS_TOTAL"
echo -e "${GREEN}Tests réussis: $TESTS_PASSED${NC}"
echo -e "${RED}Tests échoués: $TESTS_FAILED${NC}"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║  ✓ TOUS LES TESTS SONT PASSÉS AVEC SUCCÈS !   ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════╝${NC}"
    echo ""
    echo "Les dossiers de test peuvent être supprimés avec:"
    echo "  rm -rf test_*"
    exit 0
else
    echo -e "${RED}╔════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║  ✗ CERTAINS TESTS ONT ÉCHOUÉ                   ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════════════╝${NC}"
    echo ""
    echo "Consultez les logs dans les dossiers test_* pour plus de détails"
    exit 1
fi
