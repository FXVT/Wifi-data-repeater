// ========================================
// Fichier: display_values.h
// Version 1.05 - Ajout COG avec bateau tournant
// 
// Gere l'affichage dynamique des donnees NMEA
// sur l'ecran cree par display_data.cpp
// ========================================
#ifndef DISPLAY_VALUES_H
#define DISPLAY_VALUES_H

#include <lvgl.h>
#include "nmea_data.h"

// ========================================
// FONCTIONS PUBLIQUES
// ========================================

// Cree tous les labels pour l'affichage des donnees
// A appeler apres createDataScreen()
void createDataLabels();

// Met a jour les valeurs affichees
// data: pointeur vers les donnees NMEA decodees
void updateDataValues(const NmeaData* data);

#endif // DISPLAY_VALUES_H
