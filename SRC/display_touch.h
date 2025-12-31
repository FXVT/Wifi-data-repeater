// ========================================
// Fichier: display_touch.h
// Version 1.10 - Module gestion tactile
// 
// Fonctionnalités:
// - Mode veille (touch cadre COG)
// - Reset vent max (touch cadre WIND)
// - Boutons +/- décalage horaire (cadre CLOCK)
// ========================================
#ifndef DISPLAY_TOUCH_H
#define DISPLAY_TOUCH_H

#include <lvgl.h>
#include <esp_display_panel.hpp>
#include "nmea_data.h"

using namespace esp_panel::board;
using namespace esp_panel::drivers;

// ========================================
// VARIABLES GLOBALES
// ========================================
extern bool sleep_mode;

// ========================================
// FONCTIONS PUBLIQUES
// ========================================

// Créer les boutons +/- décalage horaire (v1.10e)
void createClockButtons();

// Créer les boutons +/- et initialiser le système tactile
void createTouchHandler(Board *board);

// Mettre à jour la gestion tactile (à appeler dans loop)
void updateTouchInput(Board *board, NmeaData *data, int *decalage_Horaire);

// Accesseurs pour tests
bool isTouchInHdgFrame(int x, int y);
bool isTouchInWindFrame(int x, int y);
bool isTouchInClockPlusButton(int x, int y);
bool isTouchInClockMinusButton(int x, int y);

#endif // DISPLAY_TOUCH_H
