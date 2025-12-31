// ========================================
// Fichier: display_init.h
// Version 1.02 - Initialisation écran et LVGL
// Module réutilisable pour Waveshare ESP32-S3 Touch LCD 5B
// ========================================
#ifndef DISPLAY_INIT_H
#define DISPLAY_INIT_H

#include <lvgl.h>
#include <esp_display_panel.hpp>

using namespace esp_panel::board;
using namespace esp_panel::drivers;

// ========================================
// FONCTIONS PUBLIQUES
// ========================================

// Initialise le board (écran + tactile)
// Retourne le pointeur Board ou nullptr si échec
Board* initBoard();

// Initialise LVGL avec double buffering PSRAM
// board: pointeur vers le Board initialisé
// Retourne true si succès
bool initLVGL(Board* board);

// Accesseur pour le LCD (utilisé par d'autres modules si besoin)
LCD* getLCD();

#endif // DISPLAY_INIT_H
