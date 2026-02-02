# Makefile pour le projet de débruitage d'images avec Intel MKL
# ENSGMM-1 - Outils de Calcul Scientifique

# Compilateur
CC = gcc

# Nom de l'exécutable
TARGET = image_denoise

# Fichiers sources
SRCS = src/main.c src/image.c src/filters.c src/mkl_ops.c src/io.c
OBJDIR = obj
OBJS = $(SRCS:src/%.c=$(OBJDIR)/%.o)

# Headers
HEADERS = src/image.h src/filters.h src/mkl_ops.h src/io.h

# Options de compilation
CFLAGS = -O3 -Wall -Wextra -std=c11 -I. -Isrc
CFLAGS += -fopenmp  # Support OpenMP pour parallélisme

# Chemins Intel MKL
# Ajuster selon votre installation MKL
MKLROOT ?= /opt/intel/oneapi/mkl/latest
MKL_INCLUDE = -I$(MKLROOT)/include
MKL_LIBS = -L$(MKLROOT)/lib/intel64 -lmkl_intel_lp64 -lmkl_gnu_thread -lmkl_core -lgomp -lpthread -lm -ldl

# Options complètes
CFLAGS += $(MKL_INCLUDE)
LDFLAGS = $(MKL_LIBS)

# Déclaration des cibles fantômes (phony targets)
.PHONY: all test test_image clean distclean mkl_info help stb_headers

# Règle par défaut
all: stb_headers $(TARGET)

# Télécharger les headers stb_image si nécessaire
stb_headers:
	@if [ ! -f stb_image.h ]; then \
		echo "Téléchargement de stb_image.h..."; \
		wget -q https://raw.githubusercontent.com/nothings/stb/master/stb_image.h || \
		curl -sL https://raw.githubusercontent.com/nothings/stb/master/stb_image.h -o stb_image.h; \
	fi
	@if [ ! -f stb_image_write.h ]; then \
		echo "Téléchargement de stb_image_write.h..."; \
		wget -q https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h || \
		curl -sL https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h -o stb_image_write.h; \
	fi

# Compilation de l'exécutable
$(TARGET): $(OBJS)
	@echo "Édition des liens avec Intel MKL..."
	$(CC) $(OBJS) -o bin/$(TARGET) $(LDFLAGS)
	@echo "Compilation terminée: bin/$(TARGET)"

# Règles de compilation des fichiers objets

$(OBJDIR)/%.o: src/%.c $(HEADERS)
	@mkdir -p $(OBJDIR)
	@echo "Compilation de $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Tests rapides
test: $(TARGET)
	@echo "Test avec image synthétique..."
	./$(TARGET) --test -k 7 -s 2.0 -n 15.0 -m all

# Test avec une image spécifique (à adapter)
test_image: $(TARGET)
	@echo "Test avec une image externe..."
	./$(TARGET) -i test.jpg -k 7 -s 2.0 -n 20.0 -m all

# Nettoyage
clean:
	rm -f $(OBJDIR)/*.o bin/$(TARGET)
	rm -f data/output_*.png

# Nettoyage complet (inclut les headers stb)
distclean: clean
	rm -f stb_image.h stb_image_write.h

# Informations sur l'environnement MKL
mkl_info:
	@echo "Configuration Intel MKL:"
	@echo "  MKLROOT   = $(MKLROOT)"
	@echo "  Includes  = $(MKL_INCLUDE)"
	@echo "  Libraries = $(MKL_LIBS)"

# Aide
help:
	@echo "Makefile pour le débruitage d'images avec Intel MKL"
	@echo ""
	@echo "Cibles disponibles:"
	@echo "  all         - Compile le programme (défaut)"
	@echo "  test        - Exécute un test avec image synthétique"
	@echo "  test_image  - Exécute un test avec une image externe"
	@echo "  clean       - Supprime les fichiers objets et l'exécutable"
	@echo "  distclean   - Nettoyage complet (inclut les headers stb)"
	@echo "  mkl_info    - Affiche la configuration MKL"
	@echo "  help        - Affiche cette aide"
	@echo ""
	@echo "Variables d'environnement:"
	@echo "  MKLROOT     - Chemin vers Intel MKL (défaut: /opt/intel/oneapi/mkl/latest)"
	@echo ""
	@echo "Exemples d'utilisation:"
	@echo "  make                    # Compile le programme"
	@echo "  make test               # Test rapide"
	@echo "  ./image_denoise --test  # Test interactif"
	@echo "  ./image_denoise -i mon_image.jpg -k 11 -s 3.0"

.PHONY: all test test_image clean distclean mkl_info help stb_headers
