#ifndef IO_H
#define IO_H

#include "image.h"

/**
 * Charge une image depuis un fichier (PNG, JPG, etc.)
 * Utilise stb_image pour la lecture
 * 
 * @param filename: chemin du fichier image
 * @return: image au format planaire, ou NULL en cas d'erreur
 */
ImageFloat *load_image(const char *filename);

/**
 * Sauvegarde une image dans un fichier PNG
 * Utilise stb_image_write pour l'écriture
 * 
 * @param filename: chemin du fichier de sortie
 * @param img: image à sauvegarder
 * @return: 1 si succès, 0 sinon
 */
int save_image(const char *filename, const ImageFloat *img);

/**
 * Crée une image de test synthétique (dégradé + motifs)
 * Utile pour les tests sans avoir besoin d'images externes
 * 
 * @param width: largeur de l'image
 * @param height: hauteur de l'image
 * @return: image RGB synthétique
 */
ImageFloat *create_test_image(int width, int height);

#endif // IO_H
