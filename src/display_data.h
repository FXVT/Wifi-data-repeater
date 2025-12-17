// ========================================
// Fichier: display_data.h
// Version 1.07 - Ajout pictogramme GWD
// Module réutilisable
// 
// CHANGEMENTS v1.07:
// - Ajout pictogramme GWD dans cadre GWD
// 
// CHANGEMENTS v1.06:
// - Ajout constantes de positionnement pour display_values
// - Ajout accesseur pour triangle girouette (AWA)
// ========================================
#ifndef DISPLAY_DATA_H
#define DISPLAY_DATA_H

#include <lvgl.h>

// ========================================
// CONSTANTES DE POSITIONNEMENT
// Utilisées par display_values.cpp
// ========================================

// Dimensions des cadres
#define FRAME_WIDTH           318
#define FRAME_HEIGHT_TOP      390
#define FRAME_HEIGHT_CLOCK    183
#define FRAME_HEIGHT_DEPTH    182
#define FRAME_HEIGHT_BOTTOM   100

// Espacements et marges
#define FRAME_GAP             25
#define FRAME_MARGIN_X        10
#define BANNER_HEIGHT         34

// Position Y de départ des cadres (après bandeau)
#define FRAMES_TOP_Y          (BANNER_HEIGHT + FRAME_GAP)  // 59

// Positions X des colonnes de cadres
#define FRAME_COL1_X          FRAME_MARGIN_X                                    // 10 (gauche)
#define FRAME_COL2_X          (FRAME_MARGIN_X + FRAME_WIDTH + FRAME_GAP)        // 353 (centre)
#define FRAME_COL3_X          (FRAME_MARGIN_X + (FRAME_WIDTH + FRAME_GAP) * 2)  // 696 (droite)

// Positions Y des lignes de cadres
#define FRAME_ROW1_Y          FRAMES_TOP_Y                                      // 59 (haut)
#define FRAME_ROW2_Y          (FRAMES_TOP_Y + FRAME_HEIGHT_CLOCK + FRAME_GAP)   // 267 (milieu droite)
#define FRAME_ROW3_Y          (FRAMES_TOP_Y + FRAME_HEIGHT_TOP + FRAME_GAP)     // 474 (bas)

// Centres des compas
#define COMPASS_WIND_CENTER_X (FRAME_MARGIN_X + FRAME_WIDTH / 2)                          // 169
#define COMPASS_WIND_CENTER_Y (FRAMES_TOP_Y + 155)                                        // 214
#define COMPASS_COG_CENTER_X  (FRAME_MARGIN_X + FRAME_WIDTH + FRAME_GAP + FRAME_WIDTH / 2) // 512
#define COMPASS_COG_CENTER_Y  (FRAMES_TOP_Y + 155)                                        // 214

// Dimensions des compas
#define COMPASS_RADIUS        128
#define COMPASS_TEXT_RADIUS   143

// Zone d'affichage sous les compas
// Entre le bas du "180" et le bord inférieur du cadre
#define COMPASS_BOTTOM_ZONE_Y (COMPASS_WIND_CENTER_Y + COMPASS_RADIUS + 20)  // Début zone texte sous compas

// ========================================
// FONCTIONS PUBLIQUES
// ========================================

// Crée l'écran principal d'affichage des données
// boat_name: nom du bateau pour le titre
// version: version affichée en bas
void createDataScreen(const char* boat_name, const char* version);

// Accesseurs pour les frames (pour display_values.cpp)
lv_obj_t* getWindFrame();
lv_obj_t* getCogFrame();
lv_obj_t* getClockFrame();
lv_obj_t* getDepthFrame();
lv_obj_t* getGwdFrame();
lv_obj_t* getSocFrame();
lv_obj_t* getAmpFrame();

// Accesseur pour le triangle girouette (AWA)
lv_obj_t* getWindVaneTriangle();

#endif // DISPLAY_DATA_H
