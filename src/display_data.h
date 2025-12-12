// ========================================
// Fichier: display_data.h
// Version 1.03 - Écran principal avec 7 cadres ombrés
// Module réutilisable
// ========================================
#ifndef DISPLAY_DATA_H
#define DISPLAY_DATA_H

#include <lvgl.h>

// ========================================
// FONCTIONS PUBLIQUES
// ========================================

// Crée l'écran principal d'affichage des données
// boat_name: nom du bateau pour le titre
// version: version affichée en bas
void createDataScreen(const char* boat_name, const char* version);

// Mise à jour des données affichées (pour plus tard)
// void updateDataScreen(...);

#endif // DISPLAY_DATA_H
