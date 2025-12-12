// ========================================
// Fichier: display_init.cpp
// Version 1.02 - Initialisation écran et LVGL
// Module réutilisable pour Waveshare ESP32-S3 Touch LCD 5B
// ========================================
#include <Arduino.h>
#include "display_init.h"
#include "config.h"

using namespace esp_panel::drivers;

// ========================================
// VARIABLES STATIQUES
// ========================================
static LCD *g_lcd = nullptr;
static Board *g_board = nullptr;

// ========================================
// INITIALISATION DU BOARD
// ========================================
Board* initBoard()
{
    Serial.println("[Display] Initialisation du board...");
    
    g_board = new Board();
    
    if (!g_board->begin()) {
        Serial.println("[Display] ERREUR: Impossible d'initialiser le board!");
        delete g_board;
        g_board = nullptr;
        return nullptr;
    }
    
    Serial.println("[Display] ✓ Board OK");
    return g_board;
}

// ========================================
// INITIALISATION LVGL
// ========================================
bool initLVGL(Board* board)
{
    if (board == nullptr) {
        Serial.println("[Display] ERREUR: Board null!");
        return false;
    }
    
    // Récupérer le LCD
    g_lcd = board->getLCD();
    if (g_lcd == nullptr) {
        Serial.println("[Display] ERREUR: LCD non disponible!");
        return false;
    }
    
    // Vérifier si LVGL déjà initialisé
    if (lv_is_initialized()) {
        Serial.println("[Display] LVGL déjà initialisé");
        return true;
    }
    
    Serial.println("[Display] Initialisation de LVGL...");
    lv_init();
    
    // ========================================
    // ALLOCATION DES BUFFERS EN PSRAM
    // Double buffering pour fluidité maximale
    // ========================================
    static lv_disp_draw_buf_t disp_buf;
    
    const size_t buf_size = SCREEN_WIDTH * SCREEN_HEIGHT;
    static lv_color_t *buf1 = (lv_color_t*)heap_caps_malloc(
        buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    static lv_color_t *buf2 = (lv_color_t*)heap_caps_malloc(
        buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    
    if (buf1 == nullptr || buf2 == nullptr) {
        Serial.println("[Display] ERREUR: Impossible d'allouer les buffers en PSRAM!");
        Serial.printf("[Display] PSRAM disponible: %d bytes\n", 
                      heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        return false;
    }
    
    Serial.printf("[Display] Buffer 1 @ %p (%d KB)\n", buf1, 
                  (buf_size * sizeof(lv_color_t)) / 1024);
    Serial.printf("[Display] Buffer 2 @ %p (%d KB)\n", buf2, 
                  (buf_size * sizeof(lv_color_t)) / 1024);
    Serial.printf("[Display] PSRAM restante: %d KB\n", 
                  heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024);
    
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, buf_size);
    
    // ========================================
    // CONFIGURATION DU DRIVER D'AFFICHAGE
    // ========================================
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    
    // Callback de flush vers le LCD
    disp_drv.flush_cb = [](lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
        if (g_lcd != nullptr) {
            uint32_t w = area->x2 - area->x1 + 1;
            uint32_t h = area->y2 - area->y1 + 1;
            g_lcd->drawBitmap(area->x1, area->y1, w, h, (uint8_t*)color_p);
        }
        lv_disp_flush_ready(drv);
    };
    
    lv_disp_drv_register(&disp_drv);
    
    // ========================================
    // ACTIVATION DU BACKLIGHT
    // ========================================
    auto backlight = board->getBacklight();
    if (backlight != nullptr) {
        backlight->on();
        Serial.println("[Display] Backlight activé");
    }
    
    Serial.println("[Display] ✓ LVGL initialisé");
    return true;
}

// ========================================
// ACCESSEUR LCD
// ========================================
LCD* getLCD()
{
    return g_lcd;
}
