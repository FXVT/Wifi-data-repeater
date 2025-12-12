// ========================================
// Fichier: display_splash.h
// Version 1.02 - Écran splash au démarrage
// Module réutilisable
// ========================================
#ifndef DISPLAY_SPLASH_H
#define DISPLAY_SPLASH_H

#include <lvgl.h>

// ========================================
// FONCTIONS PUBLIQUES
// ========================================

// Affiche l'écran splash pendant SPLASH_DURATION_MS
// boat_name: nom du bateau à afficher
// version: numéro de version à afficher
// Bloquant pendant la durée du splash
void displaySplash(const char* boat_name, const char* version);

#endif // DISPLAY_SPLASH_H
