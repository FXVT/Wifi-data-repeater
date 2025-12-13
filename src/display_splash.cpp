// ========================================
// Fichier: display_splash.cpp
// Version 1.04 - Ecran splash au demarrage
// Module reutilisable
// 
// CHANGEMENTS v1.04:
// - Logo Victron mis en commentaire (a remplacer)
// 
// Affiche:
// - Image de fond (341x200 zoomee)
// - "Repeteur WiFi" (titre principal)
// - Nom du bateau (parametre)
// - Logo Victron EN COMMENTAIRE (a remplacer plus tard)
// - Version (en bas a gauche)
// ========================================
#include <Arduino.h>
#include "display_splash.h"
#include "config.h"

// ========================================
// DECLARATION DES IMAGES
// Ces fichiers .c doivent etre presents dans le projet
// ========================================
LV_IMG_DECLARE(Splash_screen_vierge341x200TC);  // Image de fond (341x200 pixels)
// LV_IMG_DECLARE(Logo_Victron120x120TC);          // Logo Victron (120x120 pixels) - COMMENTE v1.04

// ========================================
// AFFICHAGE SPLASH SCREEN
// ========================================
void displaySplash(const char* boat_name, const char* version)
{
    Serial.println("[Splash] Affichage Ã©cran splash...");
    
    // RÃ©cupÃ©ration de l'Ã©cran actif LVGL
    lv_obj_t *screen = lv_scr_act();
    
    // ========================================
    // FOND NOIR
    // ========================================
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    
    // ========================================
    // IMAGE DE FOND (341x200 pixels agrandie Ã  300%)
    // Zoom 768/256 = 3x
    // ========================================
    lv_obj_t *splash_img = lv_img_create(screen);
    lv_img_set_src(splash_img, &Splash_screen_vierge341x200TC);
    lv_img_set_zoom(splash_img, 768);  // Zoom Ã  768/256 = 3x (agrandissement Ã  300%)
    lv_obj_center(splash_img);         // CentrÃ© sur l'Ã©cran
    
    // ========================================
    // LOGO VICTRON (120x120 pixels) - COMMENTE v1.04
    // Position: bas droite avec marge de 30px
    // A REMPLACER PAR UN AUTRE LOGO PLUS TARD
    // ========================================
    /*
    lv_obj_t *logo = lv_img_create(screen);
    lv_img_set_src(logo, &Logo_Victron120x120TC);
    lv_obj_align(logo, LV_ALIGN_BOTTOM_RIGHT, -30, -30);
    */
    
    // ========================================
    // TITRE: "RÃ©pÃ©teur WiFi"
    // Police Montserrat 48px, blanc, centrÃ©, dÃ©calÃ© vers le haut
    // ========================================
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Repeteur WiFi");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -80);
    
    // ========================================
    // NOM DU BATEAU
    // Police Montserrat 48px, blanc, centrÃ©
    // ========================================
    lv_obj_t *boat = lv_label_create(screen);
    lv_label_set_text(boat, boat_name);
    lv_obj_set_style_text_font(boat, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(boat, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(boat, LV_ALIGN_CENTER, 0, 0);
    
    // ========================================
    // VERSION
    // Police Montserrat 20px, blanc, bas gauche
    // ========================================
    lv_obj_t *ver = lv_label_create(screen);
    lv_label_set_text(ver, version);
    lv_obj_set_style_text_font(ver, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(ver, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(ver, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    
    // ========================================
    // RAFRAÃŽCHISSEMENT IMMÃ‰DIAT
    // ========================================
    lv_refr_now(NULL);
    
    // ========================================
    // BOUCLE D'ATTENTE (SPLASH_DURATION_MS)
    // 60 itÃ©rations Ã— 50ms = 3000ms par dÃ©faut
    // lv_timer_handler() maintient LVGL actif
    // ========================================
    int iterations = SPLASH_DURATION_MS / 50;
    for (int i = 0; i < iterations; i++) {
        lv_timer_handler();
        delay(50);
    }
    
    // ========================================
    // NETTOYAGE: Suppression des objets splash
    // Libere la memoire avant l'ecran principal
    // ========================================
    lv_obj_del(splash_img);
    // lv_obj_del(logo);  // COMMENTE v1.04 - logo non cree
    lv_obj_del(title);
    lv_obj_del(boat);
    lv_obj_del(ver);
    
    Serial.println("[Splash] âœ“ Splash terminÃ©");
}
