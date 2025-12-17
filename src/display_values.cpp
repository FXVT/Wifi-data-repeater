// ========================================
// Fichier: display_values.cpp
// Version 1.06 - Ajout rotation triangle girouette AWA
// 
// CHANGEMENTS v1.06:
// - Ajout rotation triangle girouette selon AWA
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

// AWA / TWA - Valeurs dynamiques
static lv_obj_t *label_awa_value = nullptr;  // Valeur AWA (jaune)
static lv_obj_t *label_twa_value = nullptr;  // Valeur TWA (blanc)

// AWS Max - Valeur avec label intégré
static lv_obj_t *label_aws_max_value = nullptr;  // Valeur + "ktsMAX" (blanc)

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
    lv_obj_set_style_text_color(label_clock, lv_color_hex(0x00FF00), 0);  // Vert pur
    //lv_obj_align(label_clock, LV_ALIGN_RIGHT_MID, -30, 0);  // 30px du bord droit (25+5 pour pictogramme décalé)
        lv_obj_align(label_clock, LV_ALIGN_RIGHT_MID, -10, 0);  // 30px du bord droit (25+5 pour pictogramme décalé)
    
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
    lv_obj_set_style_text_color(label_depth, lv_color_hex(0x00FF00), 0);  // Vert pur
    lv_obj_align(label_depth, LV_ALIGN_RIGHT_MID, -15, 0);
    
    // AMP
    lv_obj_t *amp_frame = getAmpFrame();
    label_amp = lv_label_create(amp_frame);
    lv_label_set_text(label_amp, "5.2 A");  // Valeur visible
    lv_obj_set_style_text_font(label_amp, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_amp, lv_color_hex(0x00FF00), 0);  // Vert pur
    lv_obj_align(label_amp, LV_ALIGN_RIGHT_MID, -15, 0);
    
    // GWD
    lv_obj_t *gwd_frame = getGwdFrame();
    label_gwd = lv_label_create(gwd_frame);
    lv_label_set_text(label_gwd, "245°");  // Valeur visible
    lv_obj_set_style_text_font(label_gwd, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_gwd, lv_color_hex(0x00FF00), 0);  // Vert pur
    lv_obj_align(label_gwd, LV_ALIGN_RIGHT_MID, -15, 0);
    
    // SOC
    lv_obj_t *soc_frame = getSocFrame();
    label_soc = lv_label_create(soc_frame);
    lv_label_set_text(label_soc, "87 %");  // Valeur visible
    lv_obj_set_style_text_font(label_soc, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_soc, lv_color_hex(0x00FF00), 0);  // Vert pur
    lv_obj_align(label_soc, LV_ALIGN_RIGHT_MID, -15, 0);
    
    // AWS
    lv_obj_t *wind_frame = getWindFrame();
    label_aws = lv_label_create(wind_frame);
    lv_label_set_text(label_aws, "12.4 kts");  // Valeur + label sur même ligne
    lv_obj_set_style_text_font(label_aws, &lv_font_montserrat_36, 0);  // Police 36
    lv_obj_set_style_text_color(label_aws, lv_color_hex(0xFFFF00), 0);  // Jaune
    lv_obj_align(label_aws, LV_ALIGN_BOTTOM_MID, 0, 5);  // Descendu de 3px (2 + 3 = 5)
    
    // AWS Max (blanc)
    label_aws_max_value = lv_label_create(wind_frame);
    lv_label_set_text(label_aws_max_value, "--- ktsMAX");  // Valeur + label sur même ligne
    lv_obj_set_style_text_font(label_aws_max_value, &lv_font_montserrat_36, 0);  // Police 36
    lv_obj_set_style_text_color(label_aws_max_value, lv_color_hex(0xFFFFFF), 0);  // Blanc
    lv_obj_align(label_aws_max_value, LV_ALIGN_BOTTOM_MID, 0, -30);  // Descendu de 3px (-33 + 3 = -30)
    
    // SOG
    lv_obj_t *cog_frame = getCogFrame();
    label_sog = lv_label_create(cog_frame);
    lv_label_set_text(label_sog, "5.8 kts");  // Valeur visible
    lv_obj_set_style_text_font(label_sog, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_sog, lv_color_hex(0x00FF00), 0);  // Vert pur
    lv_obj_align(label_sog, LV_ALIGN_BOTTOM_RIGHT, -25, -10);  // 25px du bord droit (pour pictogramme)
    
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
    // Enfant du cadre, Police: Montserrat 38, Cyan
    // ========================================
    label_cog_value = lv_label_create(label_cog_frame);  // ENFANT du cadre
    lv_label_set_text(label_cog_value, "000°");
    lv_obj_set_style_text_font(label_cog_value, &lv_font_montserrat_38, 0);  // Police 38
    lv_obj_set_style_text_color(label_cog_value, lv_color_hex(0x00FFFF), 0);  // Cyan
    lv_obj_center(label_cog_value);  // Centré dans le cadre
    
    Serial.println("[Values] Label COG value cree");
    
    // ========================================
    // AWA / TWA - VALEURS DYNAMIQUES SUR ROSE DES VENTS
    // Position équilibrée entre disque central et réticule
    // Ajustements: AWA/TWA décalées à gauche et TWA remontée
    // ========================================
    
    // Valeur AWA (au-dessus du réticule) - JAUNE
    label_awa_value = lv_label_create(screen);
    lv_label_set_text(label_awa_value, "---");  // Valeur par défaut
    lv_obj_set_style_text_font(label_awa_value, &lv_font_montserrat_38, 0);
    lv_obj_set_style_text_color(label_awa_value, lv_color_hex(0xFFFF00), 0);  // Jaune
    lv_obj_set_pos(label_awa_value, COMPASS_WIND_CENTER_X - 35, COMPASS_WIND_CENTER_Y - 50);  // Plus à gauche
    
    // Valeur TWA (en-dessous du réticule) - BLANC
    label_twa_value = lv_label_create(screen);
    lv_label_set_text(label_twa_value, "---");  // Valeur par défaut
    lv_obj_set_style_text_font(label_twa_value, &lv_font_montserrat_38, 0);
    lv_obj_set_style_text_color(label_twa_value, lv_color_hex(0xFFFFFF), 0);  // Blanc
    lv_obj_set_pos(label_twa_value, COMPASS_WIND_CENTER_X - 35, COMPASS_WIND_CENTER_Y + 38);  // Plus à gauche + remonté 4px
    
    Serial.println("[Values] Valeurs AWA/TWA creees");
    
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
    
    // AWS (valeur + label sur même ligne)
    if (data->hasWindApparent && label_aws != nullptr) {
        char aws_text[32];
        snprintf(aws_text, sizeof(aws_text), "%.1f kts", data->windSpeedApparent);
        lv_label_set_text(label_aws, aws_text);
    }
    
    // AWS Max (valeur + label sur même ligne)
    if (label_aws_max_value != nullptr) {
        char aws_max_text[32];
        if (data->windSpeedMaxApp > 0) {
            snprintf(aws_max_text, sizeof(aws_max_text), "%.1f ktsMAX", data->windSpeedMaxApp);
        } else {
            snprintf(aws_max_text, sizeof(aws_max_text), "--- ktsMAX");
        }
        lv_label_set_text(label_aws_max_value, aws_max_text);
    }
    
    // SOG
    if (data->hasSOG && label_sog != nullptr) {
        lv_label_set_text_fmt(label_sog, "%.1f kts", data->sog);
    }
    
    // ========================================
    // AWA - VALEUR NUMERIQUE (sans zéros non significatifs)
    // ========================================
    if (data->hasWindApparent && label_awa_value != nullptr) {
        lv_label_set_text_fmt(label_awa_value, "%.0f°", data->windAngleApparent);
    }
    
    // ========================================
    // TWA - VALEUR NUMERIQUE (sans zéros non significatifs)
    // ========================================
    if (data->hasWindTrue && label_twa_value != nullptr) {
        lv_label_set_text_fmt(label_twa_value, "%.0f°", data->windAngleTrue);
    }
    
    // ========================================
    // AWA - ROTATION ET DEPLACEMENT ORBITAL DU TRIANGLE GIROUETTE
    // Le triangle doit à la fois:
    // 1. Tourner sur lui-même (rotation de l'image)
    // 2. Se déplacer autour du cercle (position orbital)
    // 
    // CALCUL POSITION:
    // - Pointe du triangle doit être à 128px du centre (tangente au cercle)
    // - Centre de l'image est à 25px de la pointe (hauteur/2)
    // - Donc centre de l'image à 128 - 25 = 103px du centre de la rose
    // ========================================
    if (data->hasWindApparent) {
        lv_obj_t *triangle = getWindVaneTriangle();
        if (triangle != nullptr) {
            float awa = data->windAngleApparent;
            
            // LVGL utilise des dixiemes de degre pour la rotation
            int16_t angle_lvgl = (int16_t)(awa * 10);
            lv_img_set_angle(triangle, angle_lvgl);
            
            // Calcul position orbitale autour du centre de la rose
            // -90° car 0° est en haut (pas à droite comme en trigo standard)
            float angle_rad = (awa - 90.0f) * PI / 180.0f;
            
            // Position du centre du triangle sur le cercle (rayon 103px pour pointe à 128px)
            int pos_x = COMPASS_WIND_CENTER_X + (int)(103 * cos(angle_rad)) - 31;  // -31 = centre horizontal image
            int pos_y = COMPASS_WIND_CENTER_Y + (int)(103 * sin(angle_rad)) - 25;  // -25 = centre vertical image
            
            // Appliquer la nouvelle position
            lv_obj_set_pos(triangle, pos_x, pos_y);
        }
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
