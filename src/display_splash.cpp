// ========================================
// Fichier: display_splash.cpp
// Version 1.02 - Écran splash au démarrage
// Module réutilisable
// 
// Affiche:
// - Image de fond (341x200 zoomée)
// - "Répéteur WiFi" (titre principal)
// - Nom du bateau (paramètre)
// - Logo Victron (en bas à droite)
// - Version (en bas à gauche)
// ========================================
#include <Arduino.h>
#include "display_splash.h"
#include "config.h"

// ========================================
// DÉCLARATION DES IMAGES
// Ces fichiers .c doivent être présents dans le projet
// ========================================
LV_IMG_DECLARE(Splash_screen_vierge341x200TC);  // Image de fond (341x200 pixels)
LV_IMG_DECLARE(Logo_Victron120x120TC);          // Logo Victron (120x120 pixels)

// ========================================
// AFFICHAGE SPLASH SCREEN
// ========================================
void displaySplash(const char* boat_name, const char* version)
{
    Serial.println("[Splash] Affichage écran splash...");
    
    // Récupération de l'écran actif LVGL
    lv_obj_t *screen = lv_scr_act();
    
    // ========================================
    // FOND NOIR
    // ========================================
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    
    // ========================================
    // IMAGE DE FOND (341x200 pixels agrandie à 300%)
    // Zoom 768/256 = 3x
    // ========================================
    lv_obj_t *splash_img = lv_img_create(screen);
    lv_img_set_src(splash_img, &Splash_screen_vierge341x200TC);
    lv_img_set_zoom(splash_img, 768);  // Zoom à 768/256 = 3x (agrandissement à 300%)
    lv_obj_center(splash_img);         // Centré sur l'écran
    
    // ========================================
    // LOGO VICTRON (120x120 pixels)
    // Position: bas droite avec marge de 30px
    // ========================================
    lv_obj_t *logo = lv_img_create(screen);
    lv_img_set_src(logo, &Logo_Victron120x120TC);
    lv_obj_align(logo, LV_ALIGN_BOTTOM_RIGHT, -30, -30);
    
    // ========================================
    // TITRE: "Répéteur WiFi"
    // Police Montserrat 48px, blanc, centré, décalé vers le haut
    // ========================================
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Repeteur WiFi");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -80);
    
    // ========================================
    // NOM DU BATEAU
    // Police Montserrat 48px, blanc, centré
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
    // RAFRAÎCHISSEMENT IMMÉDIAT
    // ========================================
    lv_refr_now(NULL);
    
    // ========================================
    // BOUCLE D'ATTENTE (SPLASH_DURATION_MS)
    // 60 itérations × 50ms = 3000ms par défaut
    // lv_timer_handler() maintient LVGL actif
    // ========================================
    int iterations = SPLASH_DURATION_MS / 50;
    for (int i = 0; i < iterations; i++) {
        lv_timer_handler();
        delay(50);
    }
    
    // ========================================
    // NETTOYAGE: Suppression des objets splash
    // Libère la mémoire avant l'écran principal
    // ========================================
    lv_obj_del(splash_img);
    lv_obj_del(logo);
    lv_obj_del(title);
    lv_obj_del(boat);
    lv_obj_del(ver);
    
    Serial.println("[Splash] ✓ Splash terminé");
}
