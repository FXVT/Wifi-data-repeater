// ========================================
// Fichier: display_values.cpp
// Version 1.13 - Lecture RTC + offset manuel
// 
// CHANGEMENTS v1.13:
// - Affichage heure depuis RTC ESP32 (au lieu de nmeaData)
// - Application offset manuel (decalage_Horaire)
// - Gestion débordement 24h
// - Indicateur "?" si RTC pas synchro
// 
// FORMATS PAR TYPE:
// - Heure: --:--:--
// - Décimales (depth, amp, vitesses): --.-
// - Entiers (SOC): ---
// - Angles: ---°
// ========================================
#include <Arduino.h>
#include <sys/time.h>  // v1.13: Lecture RTC
#include <time.h>      // v1.13: Lecture RTC
#include "display_values.h"
#include "display_data.h"
#include "config.h"

// ========================================
// DÉCLARATION EXTERNE (v1.13)
// ========================================
extern bool rtc_synced;  // Flag RTC synchronisée ?

// ========================================
// DECLARATION IMAGE BATEAU
// ========================================
LV_IMG_DECLARE(sil_boat180x54TCA);

// ========================================
// VARIABLES STATIQUES - LABELS
// ========================================

static lv_obj_t *label_clock = nullptr;
static lv_obj_t *label_clock_invalid = nullptr;
static lv_obj_t *label_clock_offset = nullptr;
static lv_obj_t *label_depth = nullptr;
static lv_obj_t *label_amp = nullptr;
static lv_obj_t *label_gwd = nullptr;
static lv_obj_t *label_soc = nullptr;
static lv_obj_t *label_aws = nullptr;
static lv_obj_t *label_sog = nullptr;

// HDG - Bateau tournant + valeur numerique
static lv_obj_t *label_hdg_boat = nullptr;
static lv_obj_t *label_hdg_frame = nullptr;
static lv_obj_t *label_hdg_value = nullptr;

// AWA / TWA - Valeurs dynamiques
static lv_obj_t *label_awa_value = nullptr;
static lv_obj_t *label_twa_value = nullptr;

// AWS Max - Valeur avec label intégré
static lv_obj_t *label_aws_max_value = nullptr;

// ========================================
// CREATION DES LABELS (v1.09)
// Tous les labels créés avec format "---" cohérent
// ========================================
void createDataLabels()
{
    Serial.println("[Values] Creation des labels...");
    
    lv_obj_t *screen = lv_scr_act();
    
    // ========================================
    // CLOCK - Format: --:--:--
    // ========================================
    lv_obj_t *clock_frame = getClockFrame();
    label_clock = lv_label_create(clock_frame);
    lv_label_set_text(label_clock, "--:--:--");
    lv_obj_set_style_text_font(label_clock, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_clock, lv_color_hex(0x00FF00), 0);
    lv_obj_align(label_clock, LV_ALIGN_RIGHT_MID, -10, 0);
    
    label_clock_invalid = lv_label_create(clock_frame);
    lv_label_set_text(label_clock_invalid, " ?");
    lv_obj_set_style_text_font(label_clock_invalid, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_clock_invalid, lv_color_hex(0xFF0000), 0);
    lv_obj_add_flag(label_clock_invalid, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align_to(label_clock_invalid, label_clock, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    // ========================================
    // CLOCK OFFSET - Format: UTC +X (v1.10e)
    // Sous l'heure, aligné à gauche du texte heure, 5px vertical
    // ========================================
    label_clock_offset = lv_label_create(clock_frame);
    lv_label_set_text(label_clock_offset, "UTC");
    lv_obj_set_style_text_font(label_clock_offset, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_clock_offset, lv_color_hex(0x888888), 0);
    lv_obj_align_to(label_clock_offset, label_clock, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    
    Serial.println("[Values] Label clock offset créé (aligné gauche)");
    
    // ========================================
    // DEPTH - Format: ---.- m
    // v1.13b: Activation recolor pour valeur verte + unité blanche
    // ========================================
    lv_obj_t *depth_frame = getDepthFrame();
    label_depth = lv_label_create(depth_frame);
    lv_label_set_text(label_depth, "---.- m");
    lv_obj_set_style_text_font(label_depth, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_depth, lv_color_hex(0x00FF00), 0);
    lv_label_set_recolor(label_depth, true);  // v1.13b: Activer recolor
    lv_obj_align(label_depth, LV_ALIGN_RIGHT_MID, -15, 0);
    
    // ========================================
    // AMP - Format: --.- A
    // v1.13b: Activation recolor pour valeur verte/orange + unité blanche
    // ========================================
    lv_obj_t *amp_frame = getAmpFrame();
    label_amp = lv_label_create(amp_frame);
    lv_label_set_text(label_amp, "--.- A");
    lv_obj_set_style_text_font(label_amp, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_amp, lv_color_hex(0x00FF00), 0);
    lv_label_set_recolor(label_amp, true);  // v1.13b: Activer recolor
    lv_obj_align(label_amp, LV_ALIGN_RIGHT_MID, -15, 0);
    
    // ========================================
    // GWD - Format: ---°
    // v1.13b: Activation recolor pour valeur verte + unité blanche
    // ========================================
    lv_obj_t *gwd_frame = getGwdFrame();
    label_gwd = lv_label_create(gwd_frame);
    lv_label_set_text(label_gwd, "---\xC2\xB0");  // v1.13b: UTF-8 complet
    lv_obj_set_style_text_font(label_gwd, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_gwd, lv_color_hex(0x00FF00), 0);
    lv_label_set_recolor(label_gwd, true);  // v1.13b: Activer recolor
    lv_obj_align(label_gwd, LV_ALIGN_RIGHT_MID, -15, 0);
    
    // ========================================
    // SOC - Format: --- %
    // v1.13b: Activation recolor pour valeur verte + unité blanche
    // ========================================
    lv_obj_t *soc_frame = getSocFrame();
    label_soc = lv_label_create(soc_frame);
    lv_label_set_text(label_soc, "--- %");
    lv_obj_set_style_text_font(label_soc, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_soc, lv_color_hex(0x00FF00), 0);
    lv_label_set_recolor(label_soc, true);  // v1.13b: Activer recolor
    lv_obj_align(label_soc, LV_ALIGN_RIGHT_MID, -15, 0);
    
    // ========================================
    // AWS - Format: --.- kts
    // v1.13b: Activation recolor pour valeur jaune + unité blanche
    // ========================================
    lv_obj_t *wind_frame = getWindFrame();
    label_aws = lv_label_create(wind_frame);
    lv_label_set_text(label_aws, "--.- kts");
    lv_obj_set_style_text_font(label_aws, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(label_aws, lv_color_hex(0xFFFF00), 0);
    lv_label_set_recolor(label_aws, true);  // v1.13b: Activer recolor
    lv_obj_align(label_aws, LV_ALIGN_BOTTOM_MID, 0, 5);
    
    // ========================================
    // AWS Max - Format: --.- ktsMAX
    // ========================================
    label_aws_max_value = lv_label_create(wind_frame);
    lv_label_set_text(label_aws_max_value, "--.- ktsMAX");
    lv_obj_set_style_text_font(label_aws_max_value, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(label_aws_max_value, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_aws_max_value, LV_ALIGN_BOTTOM_MID, 0, -30);
    
    // ========================================
    // SOG - Format: --.- kts
    // v1.13b: Activation recolor pour valeur verte + unité blanche
    // ========================================
    lv_obj_t *cog_frame = getHdgFrame();
    label_sog = lv_label_create(cog_frame);
    lv_label_set_text(label_sog, "--.- kts");
    lv_obj_set_style_text_font(label_sog, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_sog, lv_color_hex(0x00FF00), 0);
    lv_label_set_recolor(label_sog, true);  // v1.13b: Activer recolor
    lv_obj_align(label_sog, LV_ALIGN_BOTTOM_RIGHT, -25, -10);
    
    // ========================================
    // HDG - IMAGE BATEAU TOURNANT
    // ========================================
    label_hdg_boat = lv_img_create(screen);
    lv_img_set_src(label_hdg_boat, &sil_boat180x54TCA);
    lv_obj_set_pos(label_hdg_boat, COMPASS_COG_CENTER_X - 27, COMPASS_COG_CENTER_Y - 90);
    lv_img_set_pivot(label_hdg_boat, 27, 90);
    lv_img_set_angle(label_hdg_boat, 0);
    
    Serial.println("[Values] Image bateau HDG creee");
    
    // ========================================
    // HDG - CADRE AUTOUR VALEUR NUMERIQUE
    // ========================================
    label_hdg_frame = lv_obj_create(screen);
    lv_obj_set_size(label_hdg_frame, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(label_hdg_frame, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(label_hdg_frame, 2, 0);
    lv_obj_set_style_border_color(label_hdg_frame, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(label_hdg_frame, 8, 0);
    lv_obj_set_style_pad_all(label_hdg_frame, 2, 0);
    lv_obj_align(label_hdg_frame, LV_ALIGN_TOP_MID, 
                 COMPASS_COG_CENTER_X - SCREEN_WIDTH / 2, 
                 COMPASS_COG_CENTER_Y - 18);
    
    Serial.println("[Values] Cadre HDG cree");
    
    // ========================================
    // HDG - VALEUR NUMERIQUE - Format: ---° (v1.13b: UTF-8 complet)
    // ========================================
    label_hdg_value = lv_label_create(label_hdg_frame);
    lv_label_set_text(label_hdg_value, "---\xC2\xB0");  // v1.13b: UTF-8 complet
    lv_obj_set_style_text_font(label_hdg_value, &lv_font_montserrat_38, 0);
    lv_obj_set_style_text_color(label_hdg_value, lv_color_hex(0x00FFFF), 0);
    lv_obj_center(label_hdg_value);
    
    Serial.println("[Values] Label HDG value cree");
    
    // ========================================
    // AWA / TWA - Format: ---° (v1.13b: UTF-8 complet)
    // ========================================
    label_awa_value = lv_label_create(screen);
    lv_label_set_text(label_awa_value, "---\xC2\xB0");  // v1.13b: UTF-8 complet
    lv_obj_set_style_text_font(label_awa_value, &lv_font_montserrat_38, 0);
    lv_obj_set_style_text_color(label_awa_value, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_pos(label_awa_value, COMPASS_WIND_CENTER_X - 35, COMPASS_WIND_CENTER_Y - 50);
    
    label_twa_value = lv_label_create(screen);
    lv_label_set_text(label_twa_value, "---\xC2\xB0");  // v1.13b: UTF-8 complet
    lv_obj_set_style_text_font(label_twa_value, &lv_font_montserrat_38, 0);
    lv_obj_set_style_text_color(label_twa_value, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(label_twa_value, COMPASS_WIND_CENTER_X - 35, COMPASS_WIND_CENTER_Y + 38);
    
    Serial.println("[Values] Valeurs AWA/TWA creees");
    
    lv_refr_now(NULL);
    
    Serial.println("[Values] Labels crees avec formats '---' coherents");
}

// ========================================
// MISE A JOUR DES VALEURS (v1.13)
// NOUVEAU: Lecture RTC + offset manuel pour l'heure
// Affichage uniquement si has* = true
// Sinon retour au format "---" approprié
// ========================================
void updateDataValues(const NmeaData* data, int decalage_Horaire)
{
    if (data == nullptr) return;
    
    // ========================================
    // CLOCK - Heure RTC + offset manuel (v1.13)
    // ========================================
    if (rtc_synced && label_clock != nullptr) {
        // Lire RTC ESP32 (heure GPS pure)
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        
        // Appliquer offset MANUEL
        int h = timeinfo.tm_hour + decalage_Horaire;
        int m = timeinfo.tm_min;
        int s = timeinfo.tm_sec;
        
        // Gérer débordement 24h
        if (h >= 24) h -= 24;
        if (h < 0) h += 24;
        
        // Afficher
        lv_label_set_text_fmt(label_clock, "%02d:%02d:%02d", h, m, s);
        
        // Masquer "?" (RTC synchro OK)
        if (label_clock_invalid != nullptr) {
            lv_obj_add_flag(label_clock_invalid, LV_OBJ_FLAG_HIDDEN);
        }
    } else if (label_clock != nullptr) {
        // RTC pas encore synchro
        lv_label_set_text(label_clock, "--:--:--");
        
        // Afficher "?" (RTC pas synchro)
        if (label_clock_invalid != nullptr) {
            lv_obj_clear_flag(label_clock_invalid, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    // ========================================
    // CLOCK OFFSET - Label "UTC +X" (v1.13)
    // ========================================
    if (label_clock_offset != nullptr) {
        char offset_text[16];
        snprintf(offset_text, sizeof(offset_text), "UTC %+d", decalage_Horaire);
        lv_label_set_text(label_clock_offset, offset_text);
    }
    
    // ========================================
    // AUTRES VALEURS (INCHANGÉES v1.11)
    // ========================================
    
    // DEPTH - Valeur verte + unité blanche (v1.13d: snprintf + set_text)
    if (data->hasDepth && label_depth != nullptr) {
        char depth_text[32];
        snprintf(depth_text, sizeof(depth_text), "#00FF00 %.1f# #FFFFFF m#", data->depth);
        lv_label_set_text(label_depth, depth_text);
    } else if (label_depth != nullptr) {
        lv_label_set_text(label_depth, "---.- m");
    }
    
    // AMP - Valeur verte/orange + unité blanche (v1.13d: snprintf + set_text)
    if (data->hasBattery && label_amp != nullptr) {
        char amp_text[32];
        if (data->batteryCurrent < 0) {
            // Courant négatif (décharge) → Orange
            snprintf(amp_text, sizeof(amp_text), "#FFA500 %.1f# #FFFFFF A#", data->batteryCurrent);
        } else {
            // Courant positif (charge) → Vert
            snprintf(amp_text, sizeof(amp_text), "#00FF00 %.1f# #FFFFFF A#", data->batteryCurrent);
        }
        lv_label_set_text(label_amp, amp_text);
    } else if (label_amp != nullptr) {
        lv_label_set_text(label_amp, "--.- A");
    }
    
    // GWD - Valeur verte + symbole ° blanc (v1.13d: snprintf + set_text)
    float gwd = data->getGroundWindDirection();
    if (!isnan(gwd) && label_gwd != nullptr) {
        char gwd_text[32];
        snprintf(gwd_text, sizeof(gwd_text), "#00FF00 %03.0f# #FFFFFF \xC2\xB0#", gwd);
        lv_label_set_text(label_gwd, gwd_text);
    } else if (label_gwd != nullptr) {
        lv_label_set_text(label_gwd, "---\xC2\xB0");
    }
    
    // SOC - Valeur verte + symbole % blanc (v1.13d: snprintf + set_text)
    if (data->hasBattery && label_soc != nullptr) {
        char soc_text[32];
        snprintf(soc_text, sizeof(soc_text), "#00FF00 %d# #FFFFFF %%#", data->batterySOC);
        lv_label_set_text(label_soc, soc_text);
    } else if (label_soc != nullptr) {
        lv_label_set_text(label_soc, "--- %");
    }
    
    // AWS - Valeur jaune + unité blanche (v1.13d: snprintf + set_text)
    if (data->hasWindApparent && label_aws != nullptr) {
        char aws_text[32];
        snprintf(aws_text, sizeof(aws_text), "#FFFF00 %.1f# #FFFFFF kts#", data->windSpeedApparent);
        lv_label_set_text(label_aws, aws_text);
    } else if (label_aws != nullptr) {
        lv_label_set_text(label_aws, "--.- kts");
    }
    
    // AWS Max
    if (label_aws_max_value != nullptr) {
        char aws_max_text[32];
        if (data->windSpeedMaxApp > 0) {
            snprintf(aws_max_text, sizeof(aws_max_text), "%.1f ktsMAX", data->windSpeedMaxApp);
        } else {
            snprintf(aws_max_text, sizeof(aws_max_text), "--.- ktsMAX");
        }
        lv_label_set_text(label_aws_max_value, aws_max_text);
    }
    
    // SOG - Valeur verte + unité blanche (v1.13d: snprintf + set_text)
    if (data->hasSOG && label_sog != nullptr) {
        char sog_text[32];
        snprintf(sog_text, sizeof(sog_text), "#00FF00 %.1f# #FFFFFF kts#", data->sog);
        lv_label_set_text(label_sog, sog_text);
    } else if (label_sog != nullptr) {
        lv_label_set_text(label_sog, "--.- kts");
    }
    
    // AWA - VALEUR NUMERIQUE (v1.13c: snprintf pour éviter bug printf)
    if (data->hasWindApparent && label_awa_value != nullptr) {
        char awa_text[16];
        snprintf(awa_text, sizeof(awa_text), "%.0f\xC2\xB0", data->windAngleApparent);
        lv_label_set_text(label_awa_value, awa_text);
    } else if (label_awa_value != nullptr) {
        lv_label_set_text(label_awa_value, "---\xC2\xB0");
    }
    
    // TWA - VALEUR NUMERIQUE (v1.13c: snprintf pour éviter bug printf)
    if (data->hasWindTrue && label_twa_value != nullptr) {
        char twa_text[16];
        snprintf(twa_text, sizeof(twa_text), "%.0f\xC2\xB0", data->windAngleTrue);
        lv_label_set_text(label_twa_value, twa_text);
    } else if (label_twa_value != nullptr) {
        lv_label_set_text(label_twa_value, "---\xC2\xB0");
    }
    
    // AWA - ROTATION ET DEPLACEMENT ORBITAL DU TRIANGLE GIROUETTE
    if (data->hasWindApparent) {
        lv_obj_t *triangle = getWindVaneTriangle();
        if (triangle != nullptr) {
            float awa = data->windAngleApparent;
            int16_t angle_lvgl = (int16_t)(awa * 10);
            lv_img_set_angle(triangle, angle_lvgl);
            
            float angle_rad = (awa - 90.0f) * PI / 180.0f;
            int pos_x = COMPASS_WIND_CENTER_X + (int)(103 * cos(angle_rad)) - 31;
            int pos_y = COMPASS_WIND_CENTER_Y + (int)(103 * sin(angle_rad)) - 25;
            lv_obj_set_pos(triangle, pos_x, pos_y);
        }
    }
    
    // V1.14: HDG - ROTATION BATEAU + VALEUR NUMERIQUE (cap compas)
    if (data->hasHeading) {
        if (label_hdg_boat != nullptr) {
            int16_t angle_lvgl = (int16_t)(data->heading * 10);
            lv_img_set_angle(label_hdg_boat, angle_lvgl);
        }
        if (label_hdg_value != nullptr) {
            char hdg_text[16];
            snprintf(hdg_text, sizeof(hdg_text), "%03.0f\xC2\xB0", data->heading);
            lv_label_set_text(label_hdg_value, hdg_text);
        }
    } else {
        if (label_hdg_value != nullptr) {
            lv_label_set_text(label_hdg_value, "---\xC2\xB0");
        }
    }
}

// ========================================
// MISE A JOUR STATUT WIFI (v1.09)
// ========================================
void updateWifiStatus(const char* message, bool isError) {
    lv_obj_t *label = getWifiStatusLabel();
    
    if (label == nullptr) {
        Serial.println("[ERROR] Label WiFi status est NULL!");
        return;
    }
    
    lv_label_set_text(label, message);
    lv_obj_update_layout(label);
    
    // Couleur selon état
    if (isError) {
        lv_obj_set_style_text_color(label, lv_color_hex(0xFF0000), 0);
    } else {
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    }
    
    // FORCER LE RAFRAICHISSEMENT IMMEDIAT
    lv_refr_now(NULL);
}
