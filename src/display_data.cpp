// ========================================
// Fichier: display_data.cpp
// Version 1.04 - Ecran principal avec 7 cadres ombres
// Module reutilisable
// 
// CHANGEMENTS v1.04:
// - Constantes exportees vers display_data.h
// - Preparation pour display_values.cpp
// ========================================
#include <Arduino.h>
#include "display_data.h"
#include "config.h"

// ========================================
// VARIABLES STATIQUES
// Labels pour mise Ã  jour future
// ========================================
// static lv_obj_t *label_placeholder = nullptr;
static lv_obj_t *label_version = nullptr;

// Cadres pour donnÃ©es
static lv_obj_t *wind_frame = nullptr;
static lv_obj_t *cog_frame = nullptr;
static lv_obj_t *clock_frame = nullptr;
static lv_obj_t *deepth_frame = nullptr;
static lv_obj_t *gwd_frame = nullptr;
static lv_obj_t *soc_frame = nullptr;
static lv_obj_t *amp_frame = nullptr;

// ========================================
// CRÃ‰ATION DE L'Ã‰CRAN PRINCIPAL
// ========================================
void createDataScreen(const char* boat_name, const char* version)
{
    Serial.println("[Data] CrÃ©ation Ã©cran principal...");
    
    lv_obj_t *screen = lv_scr_act();
    
    // ========================================
    // FOND: DÃ©gradÃ© noir vers gris foncÃ© avec dithering
    // ========================================
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_dither_mode(screen, LV_DITHER_ORDERED, 0);
    
    // ========================================
    // BANDEAU TITRE (bleu Victron)
    // Hauteur: 34px (28px police Montserrat + 2Ã—3px marge)
    // ========================================
    const int banner_height = 34;
    lv_obj_t *title_banner = lv_obj_create(screen);
    lv_obj_set_size(title_banner, SCREEN_WIDTH, banner_height);
    lv_obj_set_pos(title_banner, 0, 0);
    lv_obj_set_style_bg_color(title_banner, lv_color_hex(0x005FBE), 0);
    lv_obj_set_style_border_width(title_banner, 0, 0);
    lv_obj_set_style_radius(title_banner, 0, 0);
    lv_obj_set_style_pad_all(title_banner, 0, 0);
    
    // ========================================
    // TITRE: "RÃ©pÃ©teur WiFi" + nom du bateau
    // ========================================
    char title_text[100];
    snprintf(title_text, sizeof(title_text), "Repeteur WiFi - %s", boat_name);
    
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, title_text);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 3);
    
    // ========================================
    // CALCULS DIMENSIONS CADRES
    // Ã‰cran: 1024Ã—600, bandeau: 34px
    // Ombre: 20px width, offset 8px
    // Espacement: 25px (pour voir l'ombrage)
    // Marges: 10px gauche/droite
    // Largeur disponible: 1024 - 20 (marges) - 50 (2 gaps) = 954px
    // Largeur par cadre: 954/3 = 318px
    // Bordure: 3px gris clair (0x333333)
    // Ligne d'Ã©tat: 26px en bas pour version/messages
    // 
    // RÃˆGLES HAUTEURS:
    // - wind_frame = cog_frame = 390px
    // - clock_frame + gap + deepth_frame = 390px â†’ 183px + 25px + 182px
    // - gwd_frame = soc_frame = amp_frame = 100px
    // ========================================
    const int frame_width = 318;        // Largeur zone utile
    const int shadow_offset = 8;        // Offset ombre
    const int gap = 25;                 // Espacement entre cadres
    const int margin_x = 10;            // Marge gauche/droite
    const int border_width = 3;         // Bordure grise
    const int status_line_height = 26;  // Hauteur ligne d'Ã©tat (version/messages)
    
    // Position Y aprÃ¨s bandeau
    const int top_y = banner_height + gap;
    
    // Hauteur cadres hauts gauche/centre
    const int top_frame_height = 390;
    
    // Hauteur cadres droite
    const int clock_frame_height = 183;
    const int deepth_frame_height = 182;
    
    // Hauteur cadres bas
    const int bottom_frame_height = 100;
    
    // ========================================
    // CADRE WIND (haut gauche)
    // ========================================
    wind_frame = lv_obj_create(screen);
    lv_obj_set_size(wind_frame, frame_width, top_frame_height);
    lv_obj_set_pos(wind_frame, margin_x, top_y);
    lv_obj_set_style_bg_color(wind_frame, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(wind_frame, border_width, 0);
    lv_obj_set_style_border_color(wind_frame, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(wind_frame, 20, 0);
    lv_obj_set_style_pad_all(wind_frame, 10, 0);
    lv_obj_clear_flag(wind_frame, LV_OBJ_FLAG_SCROLLABLE);  // Pas de scroll
    lv_obj_set_style_clip_corner(wind_frame, false, 0);      // Pas de clip aux coins
    lv_obj_set_style_shadow_width(wind_frame, 20, 0);
    lv_obj_set_style_shadow_color(wind_frame, lv_color_black(), 0);
    lv_obj_set_style_shadow_ofs_x(wind_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_ofs_y(wind_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_opa(wind_frame, LV_OPA_40, 0);
    
    // ========================================
    // COMPAS DANS WIND_FRAME
    // Centre: (margin_x + frame_width/2, top_y + 155) - descendu de 5px
    // Rayon graduations/arcs: 128px (rÃ©duit pour Ã©cart 15px avec textes)
    // Rayon textes: 143px (fixe)
    // 36 graduations tous les 10Â°
    // ========================================
    const int compass_center_x = margin_x + frame_width / 2;
    const int compass_center_y = top_y + 155;
    const int compass_radius = 128;
    const int text_radius = 143;  // Fixe Ã  143px (Ã©cart de 15px avec graduations)
    
    // ========================================
    // ARCS DE COULEUR (arriÃ¨re-plan des graduations)
    // Arc rouge: 300Â° Ã  0Â° (secteur babord)
    // Arc vert: 0Â° Ã  60Â° (secteur tribord)
    // Rotation 270Â° pour avoir 0Â° en haut
    // ========================================
    
    // Arc rouge (300Â° Ã  360Â°/0Â°)
    lv_obj_t *arc_red = lv_arc_create(screen);
    lv_obj_set_size(arc_red, compass_radius * 2, compass_radius * 2);
    lv_obj_set_pos(arc_red, compass_center_x - compass_radius, compass_center_y - compass_radius);
    lv_arc_set_rotation(arc_red, 270);  // 270Â° pour avoir 0Â° en haut
    lv_arc_set_bg_angles(arc_red, 300, 360);  // De 300Â° Ã  0Â° (360Â°)
    lv_arc_set_range(arc_red, 0, 60);  // Range pour que value=60 = 100% de l'arc
    lv_arc_set_value(arc_red, 60);  // Arc plein (60Â° de 300 Ã  360)
    lv_obj_set_style_arc_width(arc_red, 6, LV_PART_INDICATOR);  // Ã‰paisseur 6px
    lv_obj_set_style_arc_color(arc_red, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc_red, 0, LV_PART_MAIN);  // Pas d'arc de fond
    lv_obj_clear_flag(arc_red, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(arc_red, LV_OPA_TRANSP, 0);  // Fond transparent
    lv_obj_remove_style(arc_red, NULL, LV_PART_KNOB);  // Supprimer complÃ¨tement le knob
    
    // Arc vert (0Â° Ã  60Â°)
    lv_obj_t *arc_green = lv_arc_create(screen);
    lv_obj_set_size(arc_green, compass_radius * 2, compass_radius * 2);
    lv_obj_set_pos(arc_green, compass_center_x - compass_radius, compass_center_y - compass_radius);
    lv_arc_set_rotation(arc_green, 270);  // 270Â° pour avoir 0Â° en haut
    lv_arc_set_bg_angles(arc_green, 0, 60);  // De 0Â° Ã  60Â°
    lv_arc_set_range(arc_green, 0, 60);  // Range pour que value=60 = 100% de l'arc
    lv_arc_set_value(arc_green, 60);  // Arc plein (60Â° de 0 Ã  60)
    lv_obj_set_style_arc_width(arc_green, 6, LV_PART_INDICATOR);  // Ã‰paisseur 6px
    lv_obj_set_style_arc_color(arc_green, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc_green, 0, LV_PART_MAIN);  // Pas d'arc de fond
    lv_obj_clear_flag(arc_green, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(arc_green, LV_OPA_TRANSP, 0);  // Fond transparent
    lv_obj_remove_style(arc_green, NULL, LV_PART_KNOB);  // Supprimer complÃ¨tement le knob
    
    // ========================================
    // RÃ‰TICULE (croix au centre) - BARRE HORIZONTALE UNIQUEMENT
    // Trait horizontal, Ã©paisseur 1px
    // S'arrÃªte Ã  50px des graduations (longueur branche: 78px)
    // ========================================
    static lv_point_t reticule_h[2];  // Trait horizontal
    
    const int reticule_length = compass_radius - 50;  // S'arrÃªte Ã  50px des graduations
    
    // Trait horizontal (270Â° Ã  90Â°)
    reticule_h[0].x = compass_center_x - reticule_length;
    reticule_h[0].y = compass_center_y;
    reticule_h[1].x = compass_center_x + reticule_length;
    reticule_h[1].y = compass_center_y;
    
    lv_obj_t *line_h = lv_line_create(screen);
    lv_line_set_points(line_h, reticule_h, 2);
    lv_obj_set_style_line_color(line_h, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_line_width(line_h, 1, 0);
    
    // ========================================
    // GRADUATIONS RADIALES (36 traits tous les 10Â°)
    // Par-dessus les arcs de couleur et le rÃ©ticule
    // ========================================
    // Allocation statique pour les 36 lignes
    static lv_point_t compass_lines[36][2];
    
    for (int i = 0; i < 36; i++) {
        int angle = i * 10;  // 0, 10, 20, 30...350
        float angle_rad = (angle - 90) * PI / 180.0f;  // -90 pour commencer Ã  0Â° en haut
        
        bool is_major = (angle % 30 == 0);  // Graduation principale tous les 30Â°
        
        int line_length = is_major ? 20 : 15;
        int line_width = is_major ? 4 : 2;
        
        // Point de dÃ©part (extÃ©rieur)
        int x1 = compass_center_x + (int)(compass_radius * cos(angle_rad));
        int y1 = compass_center_y + (int)(compass_radius * sin(angle_rad));
        
        // Point d'arrivÃ©e (vers le centre)
        int x2 = compass_center_x + (int)((compass_radius - line_length) * cos(angle_rad));
        int y2 = compass_center_y + (int)((compass_radius - line_length) * sin(angle_rad));
        
        // Stocker les points
        compass_lines[i][0].x = x1;
        compass_lines[i][0].y = y1;
        compass_lines[i][1].x = x2;
        compass_lines[i][1].y = y2;
        
        // CrÃ©er la ligne
        lv_obj_t *line = lv_line_create(screen);
        lv_line_set_points(line, compass_lines[i], 2);
        lv_obj_set_style_line_color(line, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_line_width(line, line_width, 0);
    }
    
    // Nombres 0-330 (par pas de 30Â°)
    for (int angle = 0; angle < 360; angle += 30) {
        float angle_rad = (angle - 90) * PI / 180.0f;  // -90 pour commencer Ã  0Â° en haut
        
        // Position du texte
        int text_x = compass_center_x + (int)(text_radius * cos(angle_rad));
        int text_y = compass_center_y + (int)(text_radius * sin(angle_rad));
        
        // CrÃ©er le label
        lv_obj_t *label = lv_label_create(screen);
        char text[8];
        snprintf(text, sizeof(text), "%d", angle);
        lv_label_set_text(label, text);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
        
        // Centrer le texte sur la position calculÃ©e
        lv_obj_set_pos(label, text_x - 10, text_y - 7);  // Approximation pour centrer
    }
    
    // ========================================
    // CADRE COG (haut centre)
    // ========================================
    cog_frame = lv_obj_create(screen);
    lv_obj_set_size(cog_frame, frame_width, top_frame_height);
    lv_obj_set_pos(cog_frame, margin_x + frame_width + gap, top_y);
    lv_obj_set_style_bg_color(cog_frame, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(cog_frame, border_width, 0);
    lv_obj_set_style_border_color(cog_frame, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(cog_frame, 20, 0);
    lv_obj_set_style_pad_all(cog_frame, 10, 0);
    lv_obj_clear_flag(cog_frame, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_clip_corner(cog_frame, false, 0);
    lv_obj_set_style_shadow_width(cog_frame, 20, 0);
    lv_obj_set_style_shadow_color(cog_frame, lv_color_black(), 0);
    lv_obj_set_style_shadow_ofs_x(cog_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_ofs_y(cog_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_opa(cog_frame, LV_OPA_40, 0);
    
    // ========================================
    // COMPAS DANS COG_FRAME (sans arcs rouge/vert)
    // Centre: (margin_x + frame_width + gap + frame_width/2, top_y + 155)
    // MÃªme dimensions que la rose des vents
    // ========================================
    const int compass2_center_x = margin_x + frame_width + gap + frame_width / 2;
    const int compass2_center_y = top_y + 155;
    
    // ========================================
    // RÃ‰TICULE COMPAS COG
    // ========================================
    static lv_point_t reticule2_h[2];
    static lv_point_t reticule2_v[2];
    
    // Trait horizontal
    reticule2_h[0].x = compass2_center_x - reticule_length;
    reticule2_h[0].y = compass2_center_y;
    reticule2_h[1].x = compass2_center_x + reticule_length;
    reticule2_h[1].y = compass2_center_y;
    
    lv_obj_t *line2_h = lv_line_create(screen);
    lv_line_set_points(line2_h, reticule2_h, 2);
    lv_obj_set_style_line_color(line2_h, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_line_width(line2_h, 1, 0);
    
    // Trait vertical
    reticule2_v[0].x = compass2_center_x;
    reticule2_v[0].y = compass2_center_y - reticule_length;
    reticule2_v[1].x = compass2_center_x;
    reticule2_v[1].y = compass2_center_y + reticule_length;
    
    lv_obj_t *line2_v = lv_line_create(screen);
    lv_line_set_points(line2_v, reticule2_v, 2);
    lv_obj_set_style_line_color(line2_v, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_line_width(line2_v, 1, 0);
    
    // ========================================
    // GRADUATIONS RADIALES COMPAS COG (36 traits tous les 10Â°)
    // SAUF 0Â°, 90Â°, 180Â°, 270Â° remplacÃ©s par lettres cardinales
    // ========================================
    static lv_point_t compass2_lines[36][2];
    
    for (int i = 0; i < 36; i++) {
        int angle = i * 10;
        
        // Sauter les graduations 0Â°, 90Â°, 180Â°, 270Â° (remplacÃ©es par N, E, S, W)
        if (angle == 0 || angle == 90 || angle == 180 || angle == 270) {
            continue;
        }
        
        float angle_rad = (angle - 90) * PI / 180.0f;
        
        bool is_major = (angle % 30 == 0);
        
        int line_length = is_major ? 20 : 15;
        int line_width = is_major ? 4 : 2;
        
        // Point de dÃ©part (extÃ©rieur)
        int x1 = compass2_center_x + (int)(compass_radius * cos(angle_rad));
        int y1 = compass2_center_y + (int)(compass_radius * sin(angle_rad));
        
        // Point d'arrivÃ©e (vers le centre)
        int x2 = compass2_center_x + (int)((compass_radius - line_length) * cos(angle_rad));
        int y2 = compass2_center_y + (int)((compass_radius - line_length) * sin(angle_rad));
        
        // Stocker les points
        compass2_lines[i][0].x = x1;
        compass2_lines[i][0].y = y1;
        compass2_lines[i][1].x = x2;
        compass2_lines[i][1].y = y2;
        
        // CrÃ©er la ligne
        lv_obj_t *line = lv_line_create(screen);
        lv_line_set_points(line, compass2_lines[i], 2);
        lv_obj_set_style_line_color(line, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_line_width(line, line_width, 0);
    }
    
    // ========================================
    // LETTRES CARDINALES N, E, S, W
    // Police Montserrat 24, jaune pur (0xFFFF00), positionnÃ©es Ã  120px du centre (8px plus proches)
    // ========================================
    struct Cardinal {
        int angle;
        const char* letter;
    };
    
    Cardinal cardinals[4] = {
        {0, "N"},    // Nord (haut)
        {90, "E"},   // Est (droite)
        {180, "S"},  // Sud (bas)
        {270, "W"}   // Ouest (gauche)
    };
    
    const int cardinal_radius = compass_radius - 8;  // 120px (8px plus proche du centre)
    
    for (int i = 0; i < 4; i++) {
        float angle_rad = (cardinals[i].angle - 90) * PI / 180.0f;
        
        // Position au rayon des graduations - 8px
        int text_x = compass2_center_x + (int)(cardinal_radius * cos(angle_rad));
        int text_y = compass2_center_y + (int)(cardinal_radius * sin(angle_rad));
        
        // CrÃ©er le label
        lv_obj_t *label = lv_label_create(screen);
        lv_label_set_text(label, cardinals[i].letter);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFF00), 0);  // Jaune pur
        
        // Centrer le texte (police 24, ajuster dÃ©calage)
        lv_obj_set_pos(label, text_x - 10, text_y - 12);
    }
    
    // ========================================
    // NOMBRES 0-330 COMPAS COG (par pas de 30Â°)
    // Tous les nombres y compris 0, 90, 180, 270
    // ========================================
    for (int angle = 0; angle < 360; angle += 30) {
        float angle_rad = (angle - 90) * PI / 180.0f;
        
        // Position du texte
        int text_x = compass2_center_x + (int)(text_radius * cos(angle_rad));
        int text_y = compass2_center_y + (int)(text_radius * sin(angle_rad));
        
        // CrÃ©er le label
        lv_obj_t *label = lv_label_create(screen);
        char text[8];
        snprintf(text, sizeof(text), "%d", angle);
        lv_label_set_text(label, text);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
        
        // Centrer le texte sur la position calculÃ©e
        lv_obj_set_pos(label, text_x - 10, text_y - 7);
    }
    
    // ========================================
    // CADRE CLOCK (haut droite)
    // ========================================
    clock_frame = lv_obj_create(screen);
    lv_obj_set_size(clock_frame, frame_width, clock_frame_height);
    lv_obj_set_pos(clock_frame, margin_x + (frame_width + gap) * 2, top_y);
    lv_obj_set_style_bg_color(clock_frame, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(clock_frame, border_width, 0);
    lv_obj_set_style_border_color(clock_frame, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(clock_frame, 20, 0);
    lv_obj_set_style_pad_all(clock_frame, 10, 0);
    lv_obj_clear_flag(clock_frame, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_clip_corner(clock_frame, false, 0);
    lv_obj_set_style_shadow_width(clock_frame, 20, 0);
    lv_obj_set_style_shadow_color(clock_frame, lv_color_black(), 0);
    lv_obj_set_style_shadow_ofs_x(clock_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_ofs_y(clock_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_opa(clock_frame, LV_OPA_40, 0);
    
    // ========================================
    // CADRE DEPTH (milieu droite)
    // ========================================
    deepth_frame = lv_obj_create(screen);
    lv_obj_set_size(deepth_frame, frame_width, deepth_frame_height);
    lv_obj_set_pos(deepth_frame, margin_x + (frame_width + gap) * 2, top_y + clock_frame_height + gap);
    lv_obj_set_style_bg_color(deepth_frame, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(deepth_frame, border_width, 0);
    lv_obj_set_style_border_color(deepth_frame, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(deepth_frame, 20, 0);
    lv_obj_set_style_pad_all(deepth_frame, 10, 0);
    lv_obj_clear_flag(deepth_frame, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_clip_corner(deepth_frame, false, 0);
    lv_obj_set_style_shadow_width(deepth_frame, 20, 0);
    lv_obj_set_style_shadow_color(deepth_frame, lv_color_black(), 0);
    lv_obj_set_style_shadow_ofs_x(deepth_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_ofs_y(deepth_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_opa(deepth_frame, LV_OPA_40, 0);
    
    // ========================================
    // CADRE GWD (bas gauche)
    // ========================================
    gwd_frame = lv_obj_create(screen);
    lv_obj_set_size(gwd_frame, frame_width, bottom_frame_height);
    lv_obj_set_pos(gwd_frame, margin_x, top_y + top_frame_height + gap);
    lv_obj_set_style_bg_color(gwd_frame, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(gwd_frame, border_width, 0);
    lv_obj_set_style_border_color(gwd_frame, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(gwd_frame, 20, 0);
    lv_obj_set_style_pad_all(gwd_frame, 10, 0);
    lv_obj_clear_flag(gwd_frame, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_clip_corner(gwd_frame, false, 0);
    lv_obj_set_style_shadow_width(gwd_frame, 20, 0);
    lv_obj_set_style_shadow_color(gwd_frame, lv_color_black(), 0);
    lv_obj_set_style_shadow_ofs_x(gwd_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_ofs_y(gwd_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_opa(gwd_frame, LV_OPA_40, 0);
    
    // ========================================
    // CADRE SOC (bas centre)
    // ========================================
    soc_frame = lv_obj_create(screen);
    lv_obj_set_size(soc_frame, frame_width, bottom_frame_height);
    lv_obj_set_pos(soc_frame, margin_x + frame_width + gap, top_y + top_frame_height + gap);
    lv_obj_set_style_bg_color(soc_frame, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(soc_frame, border_width, 0);
    lv_obj_set_style_border_color(soc_frame, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(soc_frame, 20, 0);
    lv_obj_set_style_pad_all(soc_frame, 10, 0);
    lv_obj_clear_flag(soc_frame, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_clip_corner(soc_frame, false, 0);
    lv_obj_set_style_shadow_width(soc_frame, 20, 0);
    lv_obj_set_style_shadow_color(soc_frame, lv_color_black(), 0);
    lv_obj_set_style_shadow_ofs_x(soc_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_ofs_y(soc_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_opa(soc_frame, LV_OPA_40, 0);
    
    // ========================================
    // CADRE AMP (bas droite)
    // ========================================
    amp_frame = lv_obj_create(screen);
    lv_obj_set_size(amp_frame, frame_width, bottom_frame_height);
    lv_obj_set_pos(amp_frame, margin_x + (frame_width + gap) * 2, top_y + top_frame_height + gap);
    lv_obj_set_style_bg_color(amp_frame, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(amp_frame, border_width, 0);
    lv_obj_set_style_border_color(amp_frame, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(amp_frame, 20, 0);
    lv_obj_set_style_pad_all(amp_frame, 10, 0);
    lv_obj_clear_flag(amp_frame, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_clip_corner(amp_frame, false, 0);
    lv_obj_set_style_shadow_width(amp_frame, 20, 0);
    lv_obj_set_style_shadow_color(amp_frame, lv_color_black(), 0);
    lv_obj_set_style_shadow_ofs_x(amp_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_ofs_y(amp_frame, shadow_offset, 0);
    lv_obj_set_style_shadow_opa(amp_frame, LV_OPA_40, 0);
    
    // ========================================
    // PLACEHOLDER: "Affichage donnÃ©es" (EN COMMENTAIRE)
    // CentrÃ© sur l'Ã©cran
    // ========================================
    // label_placeholder = lv_label_create(screen);
    // lv_label_set_text(label_placeholder, "Affichage donnees");
    // lv_obj_set_style_text_font(label_placeholder, &lv_font_montserrat_48, 0);
    // lv_obj_set_style_text_color(label_placeholder, lv_color_hex(0x888888), 0);
    // lv_obj_align(label_placeholder, LV_ALIGN_CENTER, 0, 0);
    
    // ========================================
    // LIGNE D'Ã‰TAT (bas - version et messages)
    // ========================================
    label_version = lv_label_create(screen);
    lv_label_set_text(label_version, version);
    lv_obj_set_style_text_font(label_version, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label_version, lv_color_hex(0x666666), 0);
    lv_obj_align(label_version, LV_ALIGN_BOTTOM_LEFT, 10, -5);
    
    // RafraÃ®chissement
    lv_refr_now(NULL);
    
    Serial.println("[Data] âœ“ Ã‰cran principal crÃ©Ã© (7 cadres)");
}

// ========================================
// ACCESSEURS POUR LES FRAMES
// ========================================
lv_obj_t* getWindFrame() { return wind_frame; }
lv_obj_t* getCogFrame() { return cog_frame; }
lv_obj_t* getClockFrame() { return clock_frame; }
lv_obj_t* getDepthFrame() { return deepth_frame; }
lv_obj_t* getGwdFrame() { return gwd_frame; }
lv_obj_t* getSocFrame() { return soc_frame; }
lv_obj_t* getAmpFrame() { return amp_frame; }
