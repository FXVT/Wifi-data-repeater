// ========================================
// Fichier: display_touch.cpp
// Version 1.10e - Module gestion tactile
// 
// CHANGEMENTS v1.10e:
// - MÃ©morisation offset UTC en NVS (timer 5s non-bloquant)
// - Zones tactiles Ã©largies +10px
// 
// CHANGEMENTS v1.10:
// - Gestion mode veille via setBrightness (Plan B: overlay en commentaire)
// - Reset vent max (touch cadre WIND)
// - Boutons +/- dÃ©calage horaire avec feedback visuel
// - Debouncing: sortie veille immÃ©diat, reste 300-500ms
// ========================================
#include <Arduino.h>
#include <Preferences.h>  // v1.10e: MÃ©morisation offset UTC
#include "display_touch.h"
#include "display_data.h"
#include "display_values.h"
#include "config.h"

// ========================================
// CONSTANTES ZONES TACTILES
// ========================================

// CADRE HDG (centre Ã©cran - mode veille)
#define HDG_TOUCH_X      FRAME_COL2_X          // 353
#define HDG_TOUCH_Y      FRAMES_TOP_Y          // 59
#define HDG_TOUCH_W      FRAME_WIDTH           // 318
#define HDG_TOUCH_H      FRAME_HEIGHT_TOP      // 390

// CADRE WIND (gauche - reset vent max)
#define WIND_TOUCH_X     FRAME_COL1_X          // 10
#define WIND_TOUCH_Y     FRAMES_TOP_Y          // 59
#define WIND_TOUCH_W     FRAME_WIDTH           // 318
#define WIND_TOUCH_H     FRAME_HEIGHT_TOP      // 390

// BOUTONS CLOCK +/- (dimensions)
#define CLOCK_BTN_X      5                     // Marge gauche dans cadre
#define CLOCK_BTN_WIDTH  70                    // MÃªme largeur que picto clock
#define CLOCK_BTN_HEIGHT 35                    // Hauteur boutons
#define CLOCK_BTN_GAP    5                     // Gap entre bouton et picto
#define CLOCK_BTN_TOUCH_MARGIN 10              // v1.10d: Marge de dÃ©tection autour des boutons (+10px toutes directions)

// Positions Y absolues des boutons (dans le cadre clock)
// v1.10b: CoordonnÃ©es ABSOLUES avec gap EXACT de 4px du picto
// Picto clock dans display_data.cpp: lv_obj_set_pos(clock_picto, 5, 42) 
// Donc picto Y=42, hauteur 70px â†’ fin Ã  Y=112
// 
// Bouton + : Y=38-35=3, hauteur 35 â†’ fin Ã  Y=38
// Gap avec picto : 42 - 38 = 4px âœ…
// 
// Bouton - : picto fin Ã  112, gap 4px â†’ dÃ©but Ã  Y=116
// Y=116, hauteur 35 â†’ fin Ã  Y=151
#define CLOCK_BTN_PLUS_Y  3                    // Gap 4px AVANT picto (38+4=42)
#define CLOCK_PICTO_Y     42                   // Position picto (ABSOLU dans display_data.cpp)
#define CLOCK_BTN_MINUS_Y 116                  // Gap 4px APRÃˆS picto (112+4=116)

// Debouncing
#define DEBOUNCE_SLEEP_ENTER  300              // 300ms pour entrer en veille
#define DEBOUNCE_SLEEP_EXIT   0                // ImmÃ©diat pour sortir
#define DEBOUNCE_WIND_RESET   500              // 500ms pour reset vent
#define DEBOUNCE_CLOCK_BTN    300              // 300ms pour boutons +/-

// Sauvegarde offset NVS (v1.10e)
#define NVS_SAVE_DELAY  5000                   // 5 secondes avant sauvegarde

// ========================================
// VARIABLES STATIQUES
// ========================================
bool sleep_mode = false;
static bool touch_detected = false;
static unsigned long last_touch_change = 0;

// Boutons LVGL
static lv_obj_t *btn_clock_plus = nullptr;
static lv_obj_t *btn_clock_minus = nullptr;
static lv_obj_t *label_plus = nullptr;
static lv_obj_t *label_minus = nullptr;

// Timer sauvegarde offset NVS (v1.10e)
static int offset_pending_value = 0;          // Valeur en attente de sauvegarde
static unsigned long offset_change_time = 0;  // Timestamp dernier changement
static bool offset_has_changed = false;       // Flag changement en attente

// Overlay mode veille (PLAN B - ACTIVÃ‰ v1.10b)
static lv_obj_t *sleep_overlay = nullptr;

void createSleepOverlay() {
    lv_obj_t *screen = lv_scr_act();
    
    sleep_overlay = lv_obj_create(screen);
    lv_obj_set_size(sleep_overlay, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_pos(sleep_overlay, 0, 0);
    lv_obj_set_style_bg_color(sleep_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(sleep_overlay, LV_OPA_90, 0);  // 90% opaque
    lv_obj_set_style_border_width(sleep_overlay, 0, 0);
    lv_obj_set_style_radius(sleep_overlay, 0, 0);
    
    lv_obj_t *sleep_label = lv_label_create(sleep_overlay);
    lv_label_set_text(sleep_label, "En veille\nToucher pour retablir");
    lv_obj_set_style_text_font(sleep_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(sleep_label, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_align(sleep_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(sleep_label);
    
    lv_obj_add_flag(sleep_overlay, LV_OBJ_FLAG_HIDDEN);
    
    Serial.println("[Touch] Overlay veille crÃ©Ã© (Plan B activÃ©)");
}

// ========================================
// CRÃ‰ATION BOUTONS +/- CLOCK
// ========================================
void createClockButtons() {
    lv_obj_t *clock_frame = getClockFrame();
    
    if (clock_frame == nullptr) {
        Serial.println("[Touch] ERREUR: clock_frame null");
        return;
    }
    
    // ========================================
    // BOUTON + (au-dessus du picto)
    // ========================================
    btn_clock_plus = lv_obj_create(clock_frame);
    lv_obj_set_size(btn_clock_plus, CLOCK_BTN_WIDTH, CLOCK_BTN_HEIGHT);
    lv_obj_set_pos(btn_clock_plus, CLOCK_BTN_X, CLOCK_BTN_PLUS_Y);
    
    // Style: DÃ©gradÃ© vertical (clair en haut, foncÃ© en bas)
    lv_obj_set_style_bg_color(btn_clock_plus, lv_color_hex(0x606060), 0);        // Haut: gris clair
    lv_obj_set_style_bg_grad_color(btn_clock_plus, lv_color_hex(0x404040), 0);   // Bas: gris foncÃ©
    lv_obj_set_style_bg_grad_dir(btn_clock_plus, LV_GRAD_DIR_VER, 0);
    
    lv_obj_set_style_border_width(btn_clock_plus, 1, 0);
    lv_obj_set_style_border_color(btn_clock_plus, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(btn_clock_plus, 8, 0);
    lv_obj_set_style_pad_all(btn_clock_plus, 0, 0);
    lv_obj_clear_flag(btn_clock_plus, LV_OBJ_FLAG_SCROLLABLE);
    
    // Label "+"
    label_plus = lv_label_create(btn_clock_plus);
    lv_label_set_text(label_plus, "+");
    lv_obj_set_style_text_font(label_plus, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(label_plus, lv_color_hex(0xCCCCCC), 0);
    lv_obj_center(label_plus);
    
    // ========================================
    // BOUTON - (sous le picto)
    // ========================================
    btn_clock_minus = lv_obj_create(clock_frame);
    lv_obj_set_size(btn_clock_minus, CLOCK_BTN_WIDTH, CLOCK_BTN_HEIGHT);
    lv_obj_set_pos(btn_clock_minus, CLOCK_BTN_X, CLOCK_BTN_MINUS_Y);
    
    // Style: DÃ©gradÃ© vertical (clair en haut, foncÃ© en bas)
    lv_obj_set_style_bg_color(btn_clock_minus, lv_color_hex(0x606060), 0);       // Haut: gris clair
    lv_obj_set_style_bg_grad_color(btn_clock_minus, lv_color_hex(0x404040), 0);  // Bas: gris foncÃ©
    lv_obj_set_style_bg_grad_dir(btn_clock_minus, LV_GRAD_DIR_VER, 0);
    
    lv_obj_set_style_border_width(btn_clock_minus, 1, 0);
    lv_obj_set_style_border_color(btn_clock_minus, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(btn_clock_minus, 8, 0);
    lv_obj_set_style_pad_all(btn_clock_minus, 0, 0);
    lv_obj_clear_flag(btn_clock_minus, LV_OBJ_FLAG_SCROLLABLE);
    
    // Label "-"
    label_minus = lv_label_create(btn_clock_minus);
    lv_label_set_text(label_minus, "-");
    lv_obj_set_style_text_font(label_minus, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(label_minus, lv_color_hex(0xCCCCCC), 0);
    lv_obj_center(label_minus);
    
    Serial.println("[Touch] Boutons +/- clock crÃ©Ã©s (70x35, dÃ©gradÃ© vertical)");
}

// ========================================
// DÃ‰TECTION ZONES TACTILES
// ========================================
bool isTouchInHdgFrame(int x, int y) {
    return (x >= HDG_TOUCH_X && x <= (HDG_TOUCH_X + HDG_TOUCH_W) &&
            y >= HDG_TOUCH_Y && y <= (HDG_TOUCH_Y + HDG_TOUCH_H));
}

bool isTouchInWindFrame(int x, int y) {
    return (x >= WIND_TOUCH_X && x <= (WIND_TOUCH_X + WIND_TOUCH_W) &&
            y >= WIND_TOUCH_Y && y <= (WIND_TOUCH_Y + WIND_TOUCH_H));
}

bool isTouchInClockPlusButton(int x, int y) {
    // CoordonnÃ©es absolues du bouton + dans le cadre clock
    // v1.10d: Zone Ã©largie de CLOCK_BTN_TOUCH_MARGIN (10px) dans toutes les directions
    int btn_abs_x = FRAME_COL3_X + CLOCK_BTN_X - CLOCK_BTN_TOUCH_MARGIN;
    int btn_abs_y = FRAMES_TOP_Y + CLOCK_BTN_PLUS_Y - CLOCK_BTN_TOUCH_MARGIN;
    
    return (x >= btn_abs_x && 
            x <= (btn_abs_x + CLOCK_BTN_WIDTH + 2 * CLOCK_BTN_TOUCH_MARGIN) &&
            y >= btn_abs_y && 
            y <= (btn_abs_y + CLOCK_BTN_HEIGHT + 2 * CLOCK_BTN_TOUCH_MARGIN));
}

bool isTouchInClockMinusButton(int x, int y) {
    // CoordonnÃ©es absolues du bouton - dans le cadre clock
    // v1.10d: Zone Ã©largie de CLOCK_BTN_TOUCH_MARGIN (10px) dans toutes les directions
    int btn_abs_x = FRAME_COL3_X + CLOCK_BTN_X - CLOCK_BTN_TOUCH_MARGIN;
    int btn_abs_y = FRAMES_TOP_Y + CLOCK_BTN_MINUS_Y - CLOCK_BTN_TOUCH_MARGIN;
    
    return (x >= btn_abs_x && 
            x <= (btn_abs_x + CLOCK_BTN_WIDTH + 2 * CLOCK_BTN_TOUCH_MARGIN) &&
            y >= btn_abs_y && 
            y <= (btn_abs_y + CLOCK_BTN_HEIGHT + 2 * CLOCK_BTN_TOUCH_MARGIN));
}

// ========================================
// TOGGLE MODE VEILLE (PLAN B - Overlay seul)
// v1.10b: setBrightness ne fonctionne pas, utilisation overlay uniquement
// ========================================
void toggleSleepMode(Board *board) {
    if (sleep_overlay == nullptr) {
        Serial.println("[Touch] ERREUR: Overlay null!");
        return;
    }
    
    if (sleep_mode) {
        // Mode veille â†’ Afficher overlay
        lv_obj_clear_flag(sleep_overlay, LV_OBJ_FLAG_HIDDEN);
        lv_obj_invalidate(sleep_overlay);
        Serial.println("[Touch] Mode VEILLE - Overlay affichÃ©");
    } else {
        // Mode normal â†’ Masquer overlay
        lv_obj_add_flag(sleep_overlay, LV_OBJ_FLAG_HIDDEN);
        lv_obj_invalidate(sleep_overlay);
        Serial.println("[Touch] Mode NORMAL - Overlay masquÃ©");
    }
    
    lv_refr_now(NULL);
}

// ========================================
// INITIALISATION SYSTÃˆME TACTILE
// ========================================
void createTouchHandler(Board *board) {
    Serial.println("[Touch] Initialisation systÃ¨me tactile...");
    
    // ========================================
    // VÃ‰RIFICATION CRITIQUE: Touch disponible ?
    // v1.10b: Debug pour identifier problÃ¨me "pas de trace touch"
    // ========================================
    auto touch = board->getTouch();
    if (touch == nullptr) {
        Serial.println("[Touch] â•”â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•—");
        Serial.println("[Touch] â•‘ ERREUR FATALE: Touch non disponible! â•‘");
        Serial.println("[Touch] â•‘ Le systÃ¨me tactile ne fonctionnera pasâ•‘");
        Serial.println("[Touch] â•šâ•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•");
        return;
    }
    
    Serial.println("[Touch] âœ“ Touch dÃ©tectÃ© et opÃ©rationnel");
    Serial.printf ("[Touch]   Pointeur touch: %p\n", touch);
    
    // CrÃ©er les boutons +/- clock
    createClockButtons();
    
    // PLAN B: CrÃ©er overlay veille (ACTIVÃ‰ v1.10b)
    createSleepOverlay();
    
    Serial.println("[Touch] âœ“ SystÃ¨me tactile initialisÃ© (Plan B overlay actif)");
    Serial.println("[Touch] âœ“ En attente de touches...");
}

// ========================================
// MISE Ã€ JOUR TACTILE (appelÃ© dans loop)
// ========================================
void updateTouchInput(Board *board, NmeaData *data, int *decalage_Horaire) {
    auto touch = board->getTouch();
    
    // ========================================
    // DEBUG v1.10b: VÃ©rifications critiques
    // ========================================
    if (touch == nullptr) {
        static bool error_shown = false;
        if (!error_shown) {
            Serial.println("[Touch] â•”â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•—");
            Serial.println("[Touch] â•‘ ERREUR: touch est NULL dans update!  â•‘");
            Serial.println("[Touch] â•‘ SystÃ¨me tactile non fonctionnel       â•‘");
            Serial.println("[Touch] â•šâ•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•");
            error_shown = true;
        }
        return;
    }
    
    if (data == nullptr || decalage_Horaire == nullptr) {
        static bool error_shown2 = false;
        if (!error_shown2) {
            Serial.println("[Touch] ERREUR: data ou decalage_Horaire NULL!");
            error_shown2 = true;
        }
        return;
    }
    
    // ========================================
    // DEBUG v1.10b: Message pÃ©riodique (toutes les 10s)
    // ========================================
    static unsigned long last_alive_msg = 0;
    if (millis() - last_alive_msg > 10000) {
        last_alive_msg = millis();
        Serial.println("[Touch] â™¥ SystÃ¨me tactile actif, en attente de touch...");
        Serial.printf("[Touch]   Sleep mode: %s\n", sleep_mode ? "VEILLE" : "NORMAL");
        Serial.printf("[Touch]   Offset UTC: %+d heures\n", *decalage_Horaire);
    }
    
    // Lire les points tactiles
    std::vector<TouchPoint> points;
    touch->readRawData(-1, -1, 10);
    touch->getPoints(points);
    
    // ========================================
    // DEBUG v1.10b: Afficher nombre de points dÃ©tectÃ©s
    // ========================================
    static bool last_touch_state = false;
    bool currently_touching = !points.empty();
    
    if (currently_touching && !last_touch_state) {
        Serial.printf("[Touch] âœ‹ %d point(s) tactile(s) dÃ©tectÃ©(s)\n", points.size());
        if (!points.empty()) {
            Serial.printf("[Touch]   CoordonnÃ©es brutes: x=%d, y=%d\n", 
                         points[0].x, points[0].y);
        }
    }
    last_touch_state = currently_touching;
    
    unsigned long now = millis();
    
    // ========================================
    // GESTION SORTIE DE VEILLE (IMMÃ‰DIAT)
    // ========================================
    if (currently_touching && sleep_mode && !touch_detected) {
        touch_detected = true;
        last_touch_change = now;
        sleep_mode = false;
        
        // Traces verbose
        int touch_x = points[0].x;
        int touch_y = points[0].y;
        Serial.printf("[Touch] SORTIE VEILLE dÃ©tectÃ©e: x=%d, y=%d\n", touch_x, touch_y);
        
        toggleSleepMode(board);
        return;  // Sortie immÃ©diate, pas d'autres actions
    }
    
    // ========================================
    // GESTION TOUCHES EN MODE NORMAL
    // ========================================
    if (currently_touching && !sleep_mode && !touch_detected) {
        // Debouncing: attendre DEBOUNCE_DELAY avant action
        if (now - last_touch_change < DEBOUNCE_SLEEP_ENTER) {
            // DEBUG v1.10b: Afficher debounce actif
            static unsigned long last_debounce_msg = 0;
            if (now - last_debounce_msg > 1000) {  // Max 1 msg/seconde
                last_debounce_msg = now;
                Serial.printf("[Touch] â³ Debounce actif: %lu ms restant\n", 
                             DEBOUNCE_SLEEP_ENTER - (now - last_touch_change));
            }
            return;  // Trop tÃ´t, ignorer
        }
        
        touch_detected = true;
        last_touch_change = now;
        
        // RÃ©cupÃ©rer coordonnÃ©es du premier point de touch
        int touch_x = points[0].x;
        int touch_y = points[0].y;
        
        Serial.printf("\n[Touch] =================================\n");
        Serial.printf("[Touch] Touch dÃ©tectÃ©: x=%d, y=%d\n", touch_x, touch_y);
        
        // ========================================
        // PRIORITÃ‰ 1: BOUTONS CLOCK +/- (zones petites)
        // v1.10c: Traces debug pour analyser sensibilitÃ© boutons
        // ========================================
        if (isTouchInClockPlusButton(touch_x, touch_y)) {
            Serial.println("[Touch] Zone CLOCK+ dÃ©tectÃ©e");
            
            // DEBUG v1.10c: Afficher zones de dÃ©tection
            int btn_abs_x = FRAME_COL3_X + CLOCK_BTN_X;
            int btn_abs_y = FRAMES_TOP_Y + CLOCK_BTN_PLUS_Y;
            Serial.printf("[Touch] Zone PLUS : X[%dâ†’%d], Y[%dâ†’%d]\n",
                         btn_abs_x, btn_abs_x + CLOCK_BTN_WIDTH,
                         btn_abs_y, btn_abs_y + CLOCK_BTN_HEIGHT);
            Serial.printf("[Touch] Touch (x=%d, y=%d) â†’ DANS ZONE +\n", touch_x, touch_y);
            
            (*decalage_Horaire)++;
            
            // Limiter Ã  UTC+14
            if (*decalage_Horaire > 14) *decalage_Horaire = 14;
            
            // v1.10e: DÃ©clencher timer sauvegarde NVS
            offset_pending_value = *decalage_Horaire;
            offset_change_time = millis();
            offset_has_changed = true;
            
            Serial.printf("[Touch] IncrÃ©ment offset â†’ dÃ©calage = %+d\n", *decalage_Horaire);
            Serial.printf("[Touch] Nouvel offset UTC: %+d heures\n", *decalage_Horaire);
            Serial.printf("[Touch] =================================\n\n");
            
            // Feedback visuel (flash bouton)
            if (btn_clock_plus != nullptr) {
                lv_obj_set_style_bg_color(btn_clock_plus, lv_color_hex(0x808080), 0);
                lv_refr_now(NULL);
                delay(100);
                lv_obj_set_style_bg_color(btn_clock_plus, lv_color_hex(0x606060), 0);
                lv_refr_now(NULL);
            }
            return;
        }
        
        if (isTouchInClockMinusButton(touch_x, touch_y)) {
            Serial.println("[Touch] Zone CLOCK- dÃ©tectÃ©e");
            
            // DEBUG v1.10c: Afficher zones de dÃ©tection
            int btn_abs_x = FRAME_COL3_X + CLOCK_BTN_X;
            int btn_abs_y = FRAMES_TOP_Y + CLOCK_BTN_MINUS_Y;
            Serial.printf("[Touch] Zone MINUS: X[%dâ†’%d], Y[%dâ†’%d]\n",
                         btn_abs_x, btn_abs_x + CLOCK_BTN_WIDTH,
                         btn_abs_y, btn_abs_y + CLOCK_BTN_HEIGHT);
            Serial.printf("[Touch] Touch (x=%d, y=%d) â†’ DANS ZONE -\n", touch_x, touch_y);
            
            (*decalage_Horaire)--;
            
            // Limiter Ã  UTC-12
            if (*decalage_Horaire < -12) *decalage_Horaire = -12;
            
            // v1.10e: DÃ©clencher timer sauvegarde NVS
            offset_pending_value = *decalage_Horaire;
            offset_change_time = millis();
            offset_has_changed = true;
            
            Serial.printf("[Touch] DÃ©crÃ©ment offset â†’ dÃ©calage = %+d\n", *decalage_Horaire);
            Serial.printf("[Touch] Nouvel offset UTC: %+d heures\n", *decalage_Horaire);
            Serial.printf("[Touch] =================================\n\n");
            
            // Feedback visuel (flash bouton)
            if (btn_clock_minus != nullptr) {
                lv_obj_set_style_bg_color(btn_clock_minus, lv_color_hex(0x808080), 0);
                lv_refr_now(NULL);
                delay(100);
                lv_obj_set_style_bg_color(btn_clock_minus, lv_color_hex(0x606060), 0);
                lv_refr_now(NULL);
            }
            return;
        }
        
        // ========================================
        // PRIORITÃ‰ 2: RESET VENT MAX (cadre WIND)
        // ========================================
        if (isTouchInWindFrame(touch_x, touch_y)) {
            Serial.println("[Touch] Zone WIND dÃ©tectÃ©e");
            
            // Debouncing spÃ©cifique pour reset vent (500ms)
            static unsigned long last_wind_reset = 0;
            if (now - last_wind_reset >= DEBOUNCE_WIND_RESET) {
                last_wind_reset = now;
                
                float old_max = data->windSpeedMaxApp;
                data->resetWindMaxApparent();
                
                Serial.printf("[Touch] Reset AWS Max effectuÃ©\n");
                Serial.printf("[Touch] Ancienne valeur: %.1f kts\n", old_max);
                Serial.printf("[Touch] Nouvelle valeur: %.1f kts\n", data->windSpeedMaxApp);
                Serial.printf("[Touch] =================================\n\n");
                
                // Forcer refresh immÃ©diat du label (v1.11: avec offset)
                updateDataValues(data, *decalage_Horaire);
            } else {
                Serial.printf("[Touch] Reset WIND ignorÃ© (debounce: %lu ms restant)\n", 
                            DEBOUNCE_WIND_RESET - (now - last_wind_reset));
                Serial.printf("[Touch] =================================\n\n");
            }
            return;
        }
        
        // ========================================
        // PRIORITÃ‰ 3: MODE VEILLE (cadre HDG)
        // ========================================
        if (isTouchInHdgFrame(touch_x, touch_y)) {
            Serial.println("[Touch] Zone HDG dÃ©tectÃ©e");
            Serial.println("[Touch] Passage en mode VEILLE");
            Serial.printf("[Touch] =================================\n\n");
            
            sleep_mode = true;
            toggleSleepMode(board);
            return;
        }
        
        // Aucune zone reconnue
        // DEBUG v1.10c: Afficher les zones testÃ©es pour comprendre pourquoi
        Serial.println("[Touch] âš ï¸  Aucune zone tactile dÃ©tectÃ©e");
        Serial.printf("[Touch] Touch (x=%d, y=%d) ne correspond Ã  aucune zone connue\n", touch_x, touch_y);
        Serial.println("[Touch] Zones testÃ©es:");
        
        // Afficher zones PLUS
        int plus_x = FRAME_COL3_X + CLOCK_BTN_X;
        int plus_y = FRAMES_TOP_Y + CLOCK_BTN_PLUS_Y;
        Serial.printf("[Touch]   PLUS : X[%dâ†’%d], Y[%dâ†’%d] â†’ %s\n",
                     plus_x, plus_x + CLOCK_BTN_WIDTH,
                     plus_y, plus_y + CLOCK_BTN_HEIGHT,
                     isTouchInClockPlusButton(touch_x, touch_y) ? "DEDANS" : "DEHORS");
        
        // Afficher zones MINUS
        int minus_x = FRAME_COL3_X + CLOCK_BTN_X;
        int minus_y = FRAMES_TOP_Y + CLOCK_BTN_MINUS_Y;
        Serial.printf("[Touch]   MINUS: X[%dâ†’%d], Y[%dâ†’%d] â†’ %s\n",
                     minus_x, minus_x + CLOCK_BTN_WIDTH,
                     minus_y, minus_y + CLOCK_BTN_HEIGHT,
                     isTouchInClockMinusButton(touch_x, touch_y) ? "DEDANS" : "DEHORS");
        
        // Afficher zones WIND et COG
        Serial.printf("[Touch]   WIND : X[%dâ†’%d], Y[%dâ†’%d] â†’ %s\n",
                     WIND_TOUCH_X, WIND_TOUCH_X + WIND_TOUCH_W,
                     WIND_TOUCH_Y, WIND_TOUCH_Y + WIND_TOUCH_H,
                     isTouchInWindFrame(touch_x, touch_y) ? "DEDANS" : "DEHORS");
        
        Serial.printf("[Touch]   COG  : X[%dâ†’%d], Y[%dâ†’%d] â†’ %s\n",
                     HDG_TOUCH_X, HDG_TOUCH_X + HDG_TOUCH_W,
                     HDG_TOUCH_Y, HDG_TOUCH_Y + HDG_TOUCH_H,
                     isTouchInHdgFrame(touch_x, touch_y) ? "DEDANS" : "DEHORS");
        
        Serial.printf("[Touch] =================================\n\n");
    }
    
    // ========================================
    // GESTION RELEASE (fin de touch)
    // ========================================
    if (!currently_touching && touch_detected) {
        touch_detected = false;
        last_touch_change = now;
    }
    
    // ========================================
    // SAUVEGARDE OFFSET NVS (v1.10e)
    // Timer 5s non-bloquant
    // ========================================
    if (offset_has_changed && (millis() - offset_change_time >= NVS_SAVE_DELAY)) {
        
        Preferences prefs;
        prefs.begin("mySettings", false);
        int nvs_value = prefs.getInt("utc_offset", 0);
        
        if (nvs_value != offset_pending_value) {
            // Sauvegarder seulement si diffÃ©rent
            prefs.putInt("utc_offset", offset_pending_value);
            Serial.printf("[PREFS] Offset sauvegardÃ©: %+d\n", offset_pending_value);
        } else {
            Serial.printf("[PREFS] Offset inchangÃ©: %+d\n", offset_pending_value);
        }
        
        prefs.end();
        offset_has_changed = false;  // Reset flag APRÃˆS if/else
    }
}
