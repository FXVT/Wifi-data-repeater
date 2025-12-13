// ========================================
// Fichier: display_values.cpp
// Version 1.05 - Ajout COG avec bateau tournant
// 
// CHANGEMENTS v1.05:
// - Ajout image bateau tournant sur compas COG
// - Ajout valeur COG numerique (cyan, police 32)
// 
// CHANGEMENTS v1.04:
// - Police Montserrat 48
// - Placeholders avec valeurs visibles
// - lv_refr_now pour affichage immediat
// ========================================
#include <Arduino.h>
#include "display_values.h"
#include "display_data.h"
#include "config.h"

// ========================================
// DECLARATION IMAGE BATEAU
// ========================================
LV_IMG_DECLARE(sil_boat180x54TCA);

// ========================================
// VARIABLES STATIQUES - LABELS
// ========================================

static lv_obj_t *label_clock = nullptr;
static lv_obj_t *label_clock_invalid = nullptr;
static lv_obj_t *label_depth = nullptr;
static lv_obj_t *label_amp = nullptr;
static lv_obj_t *label_gwd = nullptr;
static lv_obj_t *label_soc = nullptr;
static lv_obj_t *label_aws = nullptr;
static lv_obj_t *label_sog = nullptr;

// COG - Bateau tournant + valeur numerique
static lv_obj_t *label_cog_boat = nullptr;   // Image bateau
static lv_obj_t *label_cog_frame = nullptr;  // Cadre autour valeur COG
static lv_obj_t *label_cog_value = nullptr;  // Valeur COG

// ========================================
// CREATION DES LABELS
// ========================================
void createDataLabels()
{
    Serial.println("[Values] Creation des labels...");
    
    lv_obj_t *screen = lv_scr_act();
    
    // CLOCK
    lv_obj_t *clock_frame = getClockFrame();
    label_clock = lv_label_create(clock_frame);
    lv_label_set_text(label_clock, "12:00:00");  // Valeur visible
    lv_obj_set_style_text_font(label_clock, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_clock, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_clock, LV_ALIGN_RIGHT_MID, -15, 0);
    
    label_clock_invalid = lv_label_create(clock_frame);
    lv_label_set_text(label_clock_invalid, " ?");
    lv_obj_set_style_text_font(label_clock_invalid, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_clock_invalid, lv_color_hex(0xFF0000), 0);
    lv_obj_add_flag(label_clock_invalid, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align_to(label_clock_invalid, label_clock, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    // DEPTH
    lv_obj_t *depth_frame = getDepthFrame();
    label_depth = lv_label_create(depth_frame);
    lv_label_set_text(label_depth, "12.5 m");  // Valeur visible
    lv_obj_set_style_text_font(label_depth, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_depth, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_depth, LV_ALIGN_RIGHT_MID, -15, 0);
    
    // AMP
    lv_obj_t *amp_frame = getAmpFrame();
    label_amp = lv_label_create(amp_frame);
    lv_label_set_text(label_amp, "5.2 A");  // Valeur visible
    lv_obj_set_style_text_font(label_amp, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_amp, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_amp, LV_ALIGN_RIGHT_MID, -15, 0);
    
    // GWD
    lv_obj_t *gwd_frame = getGwdFrame();
    label_gwd = lv_label_create(gwd_frame);
    lv_label_set_text(label_gwd, "245°");  // Valeur visible
    lv_obj_set_style_text_font(label_gwd, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_gwd, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_gwd, LV_ALIGN_RIGHT_MID, -15, 0);
    
    // SOC
    lv_obj_t *soc_frame = getSocFrame();
    label_soc = lv_label_create(soc_frame);
    lv_label_set_text(label_soc, "87 %");  // Valeur visible
    lv_obj_set_style_text_font(label_soc, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_soc, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_soc, LV_ALIGN_RIGHT_MID, -15, 0);
    
    // AWS
    lv_obj_t *wind_frame = getWindFrame();
    label_aws = lv_label_create(wind_frame);
    lv_label_set_text(label_aws, "12.4 kts");  // Valeur visible
    lv_obj_set_style_text_font(label_aws, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_aws, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_aws, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    // SOG
    lv_obj_t *cog_frame = getCogFrame();
    label_sog = lv_label_create(cog_frame);
    lv_label_set_text(label_sog, "5.8 kts");  // Valeur visible
    lv_obj_set_style_text_font(label_sog, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_sog, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_sog, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    // ========================================
    // COG - IMAGE BATEAU TOURNANT
    // Position: centre du compas COG
    // Pivot: X=27, Y=90 (centre du bateau)
    // ========================================
    label_cog_boat = lv_img_create(screen);
    lv_img_set_src(label_cog_boat, &sil_boat180x54TCA);
    
    // Position au centre du compas COG
    lv_obj_set_pos(label_cog_boat, COMPASS_COG_CENTER_X - 27, COMPASS_COG_CENTER_Y - 90);
    
    // Point de pivot pour la rotation (centre du bateau)
    lv_img_set_pivot(label_cog_boat, 27, 90);
    
    // Angle initial (sera mis a jour par updateDataValues)
    lv_img_set_angle(label_cog_boat, 0);
    
    Serial.println("[Values] Image bateau COG creee");
    
    // ========================================
    // COG - CADRE AUTOUR VALEUR NUMERIQUE
    // Fond gris fonce + bordure grise (comme les frames)
    // ========================================
    label_cog_frame = lv_obj_create(screen);
    lv_obj_set_size(label_cog_frame, LV_SIZE_CONTENT, LV_SIZE_CONTENT);  // Taille auto
    lv_obj_set_style_bg_color(label_cog_frame, lv_color_hex(0x1a1a1a), 0);  // Fond frames
    lv_obj_set_style_border_width(label_cog_frame, 2, 0);  // Bordure 2px
    lv_obj_set_style_border_color(label_cog_frame, lv_color_hex(0x333333), 0);  // Gris frames
    lv_obj_set_style_radius(label_cog_frame, 8, 0);  // Coins arrondis
    lv_obj_set_style_pad_all(label_cog_frame, 2, 0);  // Padding 2px
    lv_obj_align(label_cog_frame, LV_ALIGN_TOP_MID, 
                 COMPASS_COG_CENTER_X - SCREEN_WIDTH / 2, 
                 COMPASS_COG_CENTER_Y - 18);  // Centré sur compas
    
    Serial.println("[Values] Cadre COG cree");
    
    // ========================================
    // COG - VALEUR NUMERIQUE
    // Enfant du cadre, Police: Montserrat 36, Cyan
    // ========================================
    label_cog_value = lv_label_create(label_cog_frame);  // ENFANT du cadre
    lv_label_set_text(label_cog_value, "000°");
    lv_obj_set_style_text_font(label_cog_value, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(label_cog_value, lv_color_hex(0x00FFFF), 0);  // Cyan
    lv_obj_center(label_cog_value);  // Centré dans le cadre
    
    Serial.println("[Values] Label COG value cree");
    
    // FORCER LE RAFRAICHISSEMENT IMMEDIAT
    lv_refr_now(NULL);
    
    Serial.println("[Values] Labels crees");
}

// ========================================
// MISE A JOUR DES VALEURS
// ========================================
void updateDataValues(const NmeaData* data)
{
    if (data == nullptr) return;
    
    // CLOCK
    if (data->hasTime && label_clock != nullptr) {
        lv_label_set_text(label_clock, data->utcTime.c_str());
        
        if (label_clock_invalid != nullptr) {
            if (data->timeIsValid) {
                lv_obj_add_flag(label_clock_invalid, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(label_clock_invalid, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    
    // DEPTH
    if (data->hasDepth && label_depth != nullptr) {
        lv_label_set_text_fmt(label_depth, "%.1f m", data->depth);
    }
    
    // AMP
    if (data->hasBattery && label_amp != nullptr) {
        lv_label_set_text_fmt(label_amp, "%.1f A", data->batteryCurrent);
        
        if (data->batteryCurrent < 0) {
            lv_obj_set_style_text_color(label_amp, lv_color_hex(0xFFA500), 0);
        } else {
            lv_obj_set_style_text_color(label_amp, lv_color_hex(0xFFFFFF), 0);
        }
    }
    
    // GWD
    float gwd = data->getGroundWindDirection();
    if (!isnan(gwd) && label_gwd != nullptr) {
        lv_label_set_text_fmt(label_gwd, "%03.0f", gwd);
    }
    
    // SOC
    if (data->hasBattery && label_soc != nullptr) {
        lv_label_set_text_fmt(label_soc, "%d %%", data->batterySOC);
    }
    
    // AWS
    if (data->hasWindApparent && label_aws != nullptr) {
        lv_label_set_text_fmt(label_aws, "%.1f kts", data->windSpeedApparent);
    }
    
    // SOG
    if (data->hasSOG && label_sog != nullptr) {
        lv_label_set_text_fmt(label_sog, "%.1f kts", data->sog);
    }
    
    // ========================================
    // COG - ROTATION BATEAU + VALEUR NUMERIQUE
    // ========================================
    if (data->hasCOG) {
        // Rotation de l'image bateau
        if (label_cog_boat != nullptr) {
            // LVGL utilise des dixiemes de degre
            int16_t angle_lvgl = (int16_t)(data->cog * 10);
            lv_img_set_angle(label_cog_boat, angle_lvgl);
        }
        
        // Mise a jour de la valeur numerique
        if (label_cog_value != nullptr) {
            lv_label_set_text_fmt(label_cog_value, "%03.0f°", data->cog);
        }
    }
}
