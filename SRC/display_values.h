// ========================================
// Fichier: display_values.h
// Version 1.11 - Centralisation affichage + offset
// 
// CHANGEMENTS v1.11:
// - updateDataValues() prend decalage_Horaire en paramètre
// - Affichage heure + offset centralisé dans updateDataValues()
// - Suppression updateClockWithOffset() (devenue inutile)
// 
// CHANGEMENTS v1.08:
// - Ajout fonction updateWifiStatus() pour MAJ statut WiFi
// 
// CHANGEMENTS v1.05:
// - Ajout COG avec bateau tournant
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

// Met a jour les valeurs affichees (v1.11: avec offset UTC)
// data: pointeur vers les donnees NMEA decodees
// decalage_Horaire: offset UTC en heures (-12 à +14)
void updateDataValues(const NmeaData* data, int decalage_Horaire);

// Met a jour le statut WiFi (v1.08)
// message: message a afficher
// isError: true = rouge, false = blanc
void updateWifiStatus(const char* message, bool isError);

#endif // DISPLAY_VALUES_H
