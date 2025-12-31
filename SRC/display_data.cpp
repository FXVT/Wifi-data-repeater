// ========================================
// Fichier: display_data.cpp
// Version 1.07 - Ajout pictogramme GWD
// Module reutilisable
//
// CHANGEMENTS v1.07:
// - Ajout pictogramme GWD dans cadre GWD
// - Cercles blancs pÃƒÂ©riphÃƒÂ©riques sur les 2 compas (rayon 128px, bordure 1px)
//
// CHANGEMENTS v1.06:
// - Graduations tous les 30Ã‚Â° au lieu de 10Ã‚Â° (12 au lieu de 36)
// - Lettres cardinales N/E/S/W en cyan (#00FFFF) au lieu de jaune
// - Ãƒâ€°conomie de 48 objets LVGL (72 Ã¢â€ â€™ 24 graduations)
// - Ajout triangle girouette (AWA) sur compas wind
// - Ajout disque central gris sur compas wind
// - Ajout labels et valeurs AWA/TWA
// ========================================
#include <Arduino.h>
#include "display_data.h"
#include "config.h"

// ========================================
// DECLARATION IMAGES
// ========================================
LV_IMG_DECLARE(triangle62x50TCA);
LV_IMG_DECLARE(picto_GWD2_80x80_TC);
LV_IMG_DECLARE(picto_voilier80x80TCA);
LV_IMG_DECLARE(picto_battery80x54TCA);
LV_IMG_DECLARE(picto_clock70x70TCA);
LV_IMG_DECLARE(picto_deepth66x70TCA);
LV_IMG_DECLARE(picto_current70x68TCA);

// ========================================
// VARIABLES STATIQUES
// Labels pour mise ÃƒÂ  jour future
// ========================================
// static lv_obj_t *label_placeholder = nullptr;
static lv_obj_t *label_version = nullptr;
static lv_obj_t *label_wifi_status = nullptr;  // v1.08c: Label statut WiFi

// Cadres pour donnÃƒÂ©es
static lv_obj_t *wind_frame = nullptr;
static lv_obj_t *hdg_frame = nullptr;
static lv_obj_t *clock_frame = nullptr;
static lv_obj_t *deepth_frame = nullptr;
static lv_obj_t *gwd_frame = nullptr;
static lv_obj_t *soc_frame = nullptr;
static lv_obj_t *amp_frame = nullptr;

// Triangle girouette (AWA)
static lv_obj_t *wind_vane_triangle = nullptr;

// ========================================
// CRÃƒâ€°ATION DE L'Ãƒâ€°CRAN PRINCIPAL
// ========================================
void createDataScreen(const char *boat_name, const char *version) {
  Serial.println("[Data] CrÃƒÂ©ation ÃƒÂ©cran principal...");

  lv_obj_t *screen = lv_scr_act();

  // ========================================
  // FOND: DÃƒÂ©gradÃƒÂ© noir vers gris foncÃƒÂ© avec dithering
  // ========================================
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x333333), 0);
  lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
  lv_obj_set_style_bg_dither_mode(screen, LV_DITHER_ORDERED, 0);

  // ========================================
  // BANDEAU TITRE (bleu Victron)
  // Hauteur: 34px (28px police Montserrat + 2Ãƒâ€”3px marge)
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
  // TITRE: "RÃƒÂ©pÃƒÂ©teur WiFi" + nom du bateau
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
  // Ãƒâ€°cran: 1024Ãƒâ€”600, bandeau: 34px
  // Ombre: 20px width, offset 8px
  // Espacement: 25px (pour voir l'ombrage)
  // Marges: 10px gauche/droite
  // Largeur disponible: 1024 - 20 (marges) - 50 (2 gaps) = 954px
  // Largeur par cadre: 954/3 = 318px
  // Bordure: 3px gris clair (0x333333)
  // Ligne d'ÃƒÂ©tat: 26px en bas pour version/messages
  //
  // RÃƒË†GLES HAUTEURS:
  // - wind_frame = hdg_frame = 390px
  // - clock_frame + gap + deepth_frame = 390px Ã¢â€ â€™ 183px + 25px + 182px
  // - gwd_frame = soc_frame = amp_frame = 100px
  // ========================================
  const int frame_width = 318;        // Largeur zone utile
  const int shadow_offset = 8;        // Offset ombre
  const int gap = 25;                 // Espacement entre cadres
  const int margin_x = 10;            // Marge gauche/droite
  const int border_width = 3;         // Bordure grise
  const int status_line_height = 26;  // Hauteur ligne d'ÃƒÂ©tat (version/messages)

  // Position Y aprÃƒÂ¨s bandeau
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
  lv_obj_set_style_clip_corner(wind_frame, false, 0);     // Pas de clip aux coins
  lv_obj_set_style_shadow_width(wind_frame, 20, 0);
  lv_obj_set_style_shadow_color(wind_frame, lv_color_black(), 0);
  lv_obj_set_style_shadow_ofs_x(wind_frame, shadow_offset, 0);
  lv_obj_set_style_shadow_ofs_y(wind_frame, shadow_offset, 0);
  lv_obj_set_style_shadow_opa(wind_frame, LV_OPA_40, 0);

  // ========================================
  // COMPAS DANS WIND_FRAME
  // Centre: (margin_x + frame_width/2, top_y + 155) - descendu de 5px
  // Rayon graduations/arcs: 128px (rÃƒÂ©duit pour ÃƒÂ©cart 15px avec textes)
  // Rayon textes: 143px (fixe)
  // 12 graduations tous les 30Ã‚Â°
  // ========================================
  const int compass_center_x = margin_x + frame_width / 2;
  const int compass_center_y = top_y + 155;
  const int compass_radius = 128;
  const int text_radius = 143;  // Fixe ÃƒÂ  143px (ÃƒÂ©cart de 15px avec graduations)

  // ========================================
  // ARCS DE COULEUR (arriÃƒÂ¨re-plan des graduations)
  // Arc rouge: 300Ã‚Â° ÃƒÂ  0Ã‚Â° (secteur babord)
  // Arc vert: 0Ã‚Â° ÃƒÂ  60Ã‚Â° (secteur tribord)
  // Rotation 270Ã‚Â° pour avoir 0Ã‚Â° en haut
  // ========================================

  // Arc rouge (300Ã‚Â° ÃƒÂ  360Ã‚Â°/0Ã‚Â°)
  lv_obj_t *arc_red = lv_arc_create(screen);
  lv_obj_set_size(arc_red, compass_radius * 2, compass_radius * 2);
  lv_obj_set_pos(arc_red, compass_center_x - compass_radius, compass_center_y - compass_radius);
  lv_arc_set_rotation(arc_red, 270);                          // 270Ã‚Â° pour avoir 0Ã‚Â° en haut
  lv_arc_set_bg_angles(arc_red, 300, 360);                    // De 300Ã‚Â° ÃƒÂ  0Ã‚Â° (360Ã‚Â°)
  lv_arc_set_range(arc_red, 0, 60);                           // Range pour que value=60 = 100% de l'arc
  lv_arc_set_value(arc_red, 60);                              // Arc plein (60Ã‚Â° de 300 ÃƒÂ  360)
  lv_obj_set_style_arc_width(arc_red, 6, LV_PART_INDICATOR);  // Ãƒâ€°paisseur 6px
  lv_obj_set_style_arc_color(arc_red, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(arc_red, 0, LV_PART_MAIN);  // Pas d'arc de fond
  lv_obj_clear_flag(arc_red, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_bg_opa(arc_red, LV_OPA_TRANSP, 0);  // Fond transparent
  lv_obj_remove_style(arc_red, NULL, LV_PART_KNOB);    // Supprimer complÃƒÂ¨tement le knob

  // Arc vert (0Ã‚Â° ÃƒÂ  60Ã‚Â°)
  lv_obj_t *arc_green = lv_arc_create(screen);
  lv_obj_set_size(arc_green, compass_radius * 2, compass_radius * 2);
  lv_obj_set_pos(arc_green, compass_center_x - compass_radius, compass_center_y - compass_radius);
  lv_arc_set_rotation(arc_green, 270);                          // 270Ã‚Â° pour avoir 0Ã‚Â° en haut
  lv_arc_set_bg_angles(arc_green, 0, 60);                       // De 0Ã‚Â° ÃƒÂ  60Ã‚Â°
  lv_arc_set_range(arc_green, 0, 60);                           // Range pour que value=60 = 100% de l'arc
  lv_arc_set_value(arc_green, 60);                              // Arc plein (60Ã‚Â° de 0 ÃƒÂ  60)
  lv_obj_set_style_arc_width(arc_green, 6, LV_PART_INDICATOR);  // Ãƒâ€°paisseur 6px
  lv_obj_set_style_arc_color(arc_green, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(arc_green, 0, LV_PART_MAIN);  // Pas d'arc de fond
  lv_obj_clear_flag(arc_green, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_bg_opa(arc_green, LV_OPA_TRANSP, 0);  // Fond transparent
  lv_obj_remove_style(arc_green, NULL, LV_PART_KNOB);    // Supprimer complÃƒÂ¨tement le knob

  // ========================================
  // GRADUATIONS RADIALES (12 traits tous les 30Ã‚Â°)
  // Par-dessus les arcs de couleur
  // ========================================
  // Allocation statique pour les 12 lignes
  static lv_point_t compass_lines[12][2];

  for (int i = 0; i < 12; i++) {
    int angle = i * 30;                            // 0, 30, 60, 90...330
    float angle_rad = (angle - 90) * PI / 180.0f;  // -90 pour commencer ÃƒÂ  0Ã‚Â° en haut

    // Toutes les graduations sont majeures (tous les 30Ã‚Â°)
    int line_length = 20;
    int line_width = 4;

    // Point de dÃƒÂ©part (extÃƒÂ©rieur)
    int x1 = compass_center_x + (int)(compass_radius * cos(angle_rad));
    int y1 = compass_center_y + (int)(compass_radius * sin(angle_rad));

    // Point d'arrivÃƒÂ©e (vers le centre)
    int x2 = compass_center_x + (int)((compass_radius - line_length) * cos(angle_rad));
    int y2 = compass_center_y + (int)((compass_radius - line_length) * sin(angle_rad));

    // Stocker les points
    compass_lines[i][0].x = x1;
    compass_lines[i][0].y = y1;
    compass_lines[i][1].x = x2;
    compass_lines[i][1].y = y2;

    // CrÃƒÂ©er la ligne
    lv_obj_t *line = lv_line_create(screen);
    lv_line_set_points(line, compass_lines[i], 2);
    lv_obj_set_style_line_color(line, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_line_width(line, line_width, 0);
  }

  // Nombres 0-330 (par pas de 30Ã‚Â°)
  for (int angle = 0; angle < 360; angle += 30) {
    float angle_rad = (angle - 90) * PI / 180.0f;  // -90 pour commencer ÃƒÂ  0Ã‚Â° en haut

    // Position du texte
    int text_x = compass_center_x + (int)(text_radius * cos(angle_rad));
    int text_y = compass_center_y + (int)(text_radius * sin(angle_rad));

    // CrÃƒÂ©er le label
    lv_obj_t *label = lv_label_create(screen);
    char text[8];
    snprintf(text, sizeof(text), "%d", angle);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);

    // Centrer le texte sur la position calculÃƒÂ©e
    lv_obj_set_pos(label, text_x - 10, text_y - 7);  // Approximation pour centrer
  }

  // ========================================
  // CERCLE BLANC PÃƒâ€°RIPHÃƒâ€°RIQUE (rayon 128px)
  // Bordure 1px blanche, fond transparent
  // Z-order: Au-dessus des graduations, en dessous du triangle
  // ========================================
  lv_obj_t *wind_circle = lv_obj_create(screen);
  lv_obj_set_size(wind_circle, 256, 256);                                       // DiamÃƒÂ¨tre 256px (rayon 128px)
  lv_obj_set_pos(wind_circle, compass_center_x - 128, compass_center_y - 128);  // CentrÃƒÂ©
  lv_obj_set_style_radius(wind_circle, LV_RADIUS_CIRCLE, 0);                    // Cercle parfait
  lv_obj_set_style_bg_opa(wind_circle, LV_OPA_TRANSP, 0);                       // Fond transparent
  lv_obj_set_style_border_width(wind_circle, 1, 0);                             // Bordure 1px
  lv_obj_set_style_border_color(wind_circle, lv_color_hex(0xFFFFFF), 0);        // Blanc
  lv_obj_set_style_pad_all(wind_circle, 0, 0);
  lv_obj_clear_flag(wind_circle, LV_OBJ_FLAG_SCROLLABLE);

  Serial.println("[Data] Cercle blanc pÃƒÂ©riphÃƒÂ©rique WIND crÃƒÂ©ÃƒÂ©");

  // ========================================
  // TRIANGLE GIROUETTE (AWA) - ENTRE GRADUATIONS ET RÃƒâ€°TICULE
  // Image: 62px (largeur) Ãƒâ€” 50px (hauteur)
  // Position: pointe tangente au cercle (128px), base vers le centre
  // Pivot: centre de l'image (31, 25)
  // Z-order: Par-dessus arcs/graduations, en dessous du rÃƒÂ©ticule
  //
  // CALCUL POSITION:
  // - Pointe du triangle doit ÃƒÂªtre ÃƒÂ  128px du centre (tangente au cercle)
  // - Centre de l'image est ÃƒÂ  25px de la pointe
  // - Donc centre de l'image ÃƒÂ  128 - 25 = 103px du centre de la rose
  // ========================================

  // *** ANGLE DE TEST MODIFIABLE ICI ***
  const float test_awa = 00.0f;  // Modifier cette valeur pour tester diffÃƒÂ©rents angles
  // *************************************

  wind_vane_triangle = lv_img_create(screen);
  lv_img_set_src(wind_vane_triangle, &triangle62x50TCA);

  // Calcul position orbitale (centre image ÃƒÂ  103px du centre rose)
  float angle_rad = (test_awa - 90.0f) * PI / 180.0f;
  int pos_x = compass_center_x + (int)(103 * cos(angle_rad)) - 31;
  int pos_y = compass_center_y + (int)(103 * sin(angle_rad)) - 25;

  lv_obj_set_pos(wind_vane_triangle, pos_x, pos_y);

  // Pivot au centre de l'image pour rotation
  lv_img_set_pivot(wind_vane_triangle, 31, 25);

  // Rotation de l'image sur elle-mÃƒÂªme
  lv_img_set_angle(wind_vane_triangle, (int16_t)(test_awa * 10));

  Serial.println("[Data] Triangle girouette AWA crÃƒÂ©ÃƒÂ©");
  Serial.printf("[Data] Position test: x=%d, y=%d, angle=%.0fÃ‚Â°\n", pos_x, pos_y, test_awa);

  // ========================================
  // DISQUE CENTRAL (masque base du triangle)
  // Rayon: 92px, Couleur: gris 0x404040
  // Bordure: 1px gris 0x333333
  // Z-order: Par-dessus triangle, en dessous du rÃƒÂ©ticule
  // ========================================
  lv_obj_t *wind_center_disc = lv_obj_create(screen);
  lv_obj_set_size(wind_center_disc, 184, 184);                                     // DiamÃƒÂ¨tre 184px (rayon 92px)
  lv_obj_set_pos(wind_center_disc, compass_center_x - 92, compass_center_y - 92);  // CentrÃƒÂ©
  lv_obj_set_style_bg_color(wind_center_disc, lv_color_hex(0x404040), 0);
  lv_obj_set_style_radius(wind_center_disc, LV_RADIUS_CIRCLE, 0);  // Cercle parfait
  lv_obj_set_style_border_width(wind_center_disc, 1, 0);
  lv_obj_set_style_border_color(wind_center_disc, lv_color_hex(0x333333), 0);
  lv_obj_set_style_pad_all(wind_center_disc, 0, 0);
  lv_obj_clear_flag(wind_center_disc, LV_OBJ_FLAG_SCROLLABLE);
  /*
    // Ombre (mÃƒÂªme style que les cadres) ATTENTION: provoque un blocage sur l'ecran de splash
    lv_obj_set_style_shadow_width(wind_center_disc, 20, 0);
    lv_obj_set_style_shadow_color(wind_center_disc, lv_color_black(), 0);
    lv_obj_set_style_shadow_ofs_x(wind_center_disc, shadow_offset, 0);  // 8px
    lv_obj_set_style_shadow_ofs_y(wind_center_disc, shadow_offset, 0);  // 8px
    lv_obj_set_style_shadow_opa(wind_center_disc, LV_OPA_40, 0);
	*/


  Serial.println("[Data] Disque central rose des vents crÃƒÂ©ÃƒÂ© (sans ombre)");

  // ========================================
  // RÃƒâ€°TICULE HORIZONTAL (par-dessus le disque central)
  // Trait horizontal, ÃƒÂ©paisseur 1px
  // S'arrÃƒÂªte ÃƒÂ  50px des graduations (longueur branche: 78px)
  // ========================================
  static lv_point_t reticule_wind_h[2];  // Trait horizontal (renommÃƒÂ© pour ÃƒÂ©viter conflit)

  const int reticule_length = compass_radius - 50;  // S'arrÃƒÂªte ÃƒÂ  50px des graduations

  // Trait horizontal (270Ã‚Â° ÃƒÂ  90Ã‚Â°)
  reticule_wind_h[0].x = compass_center_x - reticule_length;
  reticule_wind_h[0].y = compass_center_y;
  reticule_wind_h[1].x = compass_center_x + reticule_length;
  reticule_wind_h[1].y = compass_center_y;

  lv_obj_t *line_wind_h = lv_line_create(screen);
  lv_line_set_points(line_wind_h, reticule_wind_h, 2);
  lv_obj_set_style_line_color(line_wind_h, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_line_width(line_wind_h, 1, 0);

  Serial.println("[Data] RÃƒÂ©ticule rose des vents crÃƒÂ©ÃƒÂ©");

  // ========================================
  // LABELS AWA / TWA (au-dessus et en-dessous du rÃƒÂ©ticule)
  // Positionnement ÃƒÂ©quilibrÃƒÂ© entre le disque (rayon 92) et le rÃƒÂ©ticule
  // Zone supÃƒÂ©rieure: 92px ÃƒÂ  rÃƒÂ©partir entre AWA label + valeur
  // Zone infÃƒÂ©rieure: symÃƒÂ©trie pour TWA
  // ========================================

  // Label "AWA" (au-dessus du rÃƒÂ©ticule) - JAUNE
  lv_obj_t *label_awa_title = lv_label_create(screen);
  lv_label_set_text(label_awa_title, "AWA");
  lv_obj_set_style_text_font(label_awa_title, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(label_awa_title, lv_color_hex(0xFFFF00), 0);        // Jaune
  lv_obj_set_pos(label_awa_title, compass_center_x - 20, compass_center_y - 75);  // Descendu de 4px

  // Label "TWA" (en-dessous du rÃƒÂ©ticule) - BLANC
  lv_obj_t *label_twa_title = lv_label_create(screen);
  lv_label_set_text(label_twa_title, "TWA");
  lv_obj_set_style_text_font(label_twa_title, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(label_twa_title, lv_color_hex(0xFFFFFF), 0);        // Blanc
  lv_obj_set_pos(label_twa_title, compass_center_x - 20, compass_center_y + 13);  // CentrÃƒÂ© approximatif

  Serial.println("[Data] Labels AWA/TWA crÃƒÂ©ÃƒÂ©s");

  // ========================================
  // CADRE HDG (haut centre)
  // ========================================
  hdg_frame = lv_obj_create(screen);
  lv_obj_set_size(hdg_frame, frame_width, top_frame_height);
  lv_obj_set_pos(hdg_frame, margin_x + frame_width + gap, top_y);
  lv_obj_set_style_bg_color(hdg_frame, lv_color_hex(0x1a1a1a), 0);
  lv_obj_set_style_border_width(hdg_frame, border_width, 0);
  lv_obj_set_style_border_color(hdg_frame, lv_color_hex(0x333333), 0);
  lv_obj_set_style_radius(hdg_frame, 20, 0);
  lv_obj_set_style_pad_all(hdg_frame, 10, 0);
  lv_obj_clear_flag(hdg_frame, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_clip_corner(hdg_frame, false, 0);
  lv_obj_set_style_shadow_width(hdg_frame, 20, 0);
  lv_obj_set_style_shadow_color(hdg_frame, lv_color_black(), 0);
  lv_obj_set_style_shadow_ofs_x(hdg_frame, shadow_offset, 0);
  lv_obj_set_style_shadow_ofs_y(hdg_frame, shadow_offset, 0);
  lv_obj_set_style_shadow_opa(hdg_frame, LV_OPA_40, 0);

  // ========================================
  // PICTOGRAMME VOILIER (ÃƒÂ  gauche du cadre HDG)
  // Image: 80x80 pixels
  // Position: 25px du bord gauche, descendu de 15px
  // ========================================
  lv_obj_t *hdg_picto = lv_img_create(hdg_frame);
  lv_img_set_src(hdg_picto, &picto_voilier80x80TCA);
  lv_obj_align(hdg_picto, LV_ALIGN_BOTTOM_LEFT, 25, 5);  // Descendu de 15px (-10 + 15 = 5)

  Serial.println("[Data] Pictogramme voilier HDG crÃƒÂ©ÃƒÂ©");

  // ========================================
  // COMPAS DANS COG_FRAME (sans arcs rouge/vert)
  // Centre: (margin_x + frame_width + gap + frame_width/2, top_y + 155)
  // MÃƒÂªme dimensions que la rose des vents
  // ========================================
  const int compass2_center_x = margin_x + frame_width + gap + frame_width / 2;
  const int compass2_center_y = top_y + 155;

  // ========================================
  // RÃƒâ€°TICULE COMPAS HDG
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
  // GRADUATIONS RADIALES COMPAS HDG (12 traits tous les 30Ã‚Â°)
  // SAUF 0Ã‚Â°, 90Ã‚Â°, 180Ã‚Â°, 270Ã‚Â° remplacÃƒÂ©s par lettres cardinales
  // ========================================
  static lv_point_t compass2_lines[12][2];

  for (int i = 0; i < 12; i++) {
    int angle = i * 30;

    // Sauter les graduations 0Ã‚Â°, 90Ã‚Â°, 180Ã‚Â°, 270Ã‚Â° (remplacÃƒÂ©es par N, E, S, W)
    if (angle == 0 || angle == 90 || angle == 180 || angle == 270) {
      continue;
    }

    float angle_rad = (angle - 90) * PI / 180.0f;

    // Toutes les graduations sont majeures
    int line_length = 20;
    int line_width = 4;

    // Point de dÃƒÂ©part (extÃƒÂ©rieur)
    int x1 = compass2_center_x + (int)(compass_radius * cos(angle_rad));
    int y1 = compass2_center_y + (int)(compass_radius * sin(angle_rad));

    // Point d'arrivÃƒÂ©e (vers le centre)
    int x2 = compass2_center_x + (int)((compass_radius - line_length) * cos(angle_rad));
    int y2 = compass2_center_y + (int)((compass_radius - line_length) * sin(angle_rad));

    // Stocker les points
    compass2_lines[i][0].x = x1;
    compass2_lines[i][0].y = y1;
    compass2_lines[i][1].x = x2;
    compass2_lines[i][1].y = y2;

    // CrÃƒÂ©er la ligne
    lv_obj_t *line = lv_line_create(screen);
    lv_line_set_points(line, compass2_lines[i], 2);
    lv_obj_set_style_line_color(line, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_line_width(line, line_width, 0);
  }

  // ========================================
  // CERCLE BLANC PÃƒâ€°RIPHÃƒâ€°RIQUE COG (rayon 128px)
  // Bordure 1px blanche, fond transparent
  // Z-order: Au-dessus des graduations, en dessous du rÃƒÂ©ticule
  // ========================================
  lv_obj_t *hdg_circle = lv_obj_create(screen);
  lv_obj_set_size(hdg_circle, 256, 256);                                         // DiamÃƒÂ¨tre 256px (rayon 128px)
  lv_obj_set_pos(hdg_circle, compass2_center_x - 128, compass2_center_y - 128);  // CentrÃƒÂ©
  lv_obj_set_style_radius(hdg_circle, LV_RADIUS_CIRCLE, 0);                      // Cercle parfait
  lv_obj_set_style_bg_opa(hdg_circle, LV_OPA_TRANSP, 0);                         // Fond transparent
  lv_obj_set_style_border_width(hdg_circle, 1, 0);                               // Bordure 1px
  lv_obj_set_style_border_color(hdg_circle, lv_color_hex(0xFFFFFF), 0);          // Blanc
  lv_obj_set_style_pad_all(hdg_circle, 0, 0);
  lv_obj_clear_flag(hdg_circle, LV_OBJ_FLAG_SCROLLABLE);

  Serial.println("[Data] Cercle blanc pÃƒÂ©riphÃƒÂ©rique COG crÃƒÂ©ÃƒÂ©");

  // ========================================
  // LETTRES CARDINALES N, E, S, W
  // Police Montserrat 24, CYAN (#00FFFF), positionnÃƒÂ©es ÃƒÂ  120px du centre (8px plus proches)
  // ========================================
  struct Cardinal {
    int angle;
    const char *letter;
  };

  Cardinal cardinals[4] = {
    { 0, "N" },    // Nord (haut)
    { 90, "E" },   // Est (droite)
    { 180, "S" },  // Sud (bas)
    { 270, "W" }   // Ouest (gauche)
  };

  const int cardinal_radius = compass_radius - 8;  // 120px (8px plus proche du centre)

  for (int i = 0; i < 4; i++) {
    float angle_rad = (cardinals[i].angle - 90) * PI / 180.0f;

    // Position au rayon des graduations - 8px
    int text_x = compass2_center_x + (int)(cardinal_radius * cos(angle_rad));
    int text_y = compass2_center_y + (int)(cardinal_radius * sin(angle_rad));

    // CrÃƒÂ©er le label
    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, cardinals[i].letter);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x00FFFF), 0);  // CYAN au lieu de jaune

    // Centrer le texte (police 24, ajuster dÃƒÂ©calage)
    lv_obj_set_pos(label, text_x - 10, text_y - 12);
  }

  // ========================================
  // NOMBRES 0-330 COMPAS HDG (par pas de 30Ã‚Â°)
  // Tous les nombres y compris 0, 90, 180, 270
  // ========================================
  for (int angle = 0; angle < 360; angle += 30) {
    float angle_rad = (angle - 90) * PI / 180.0f;

    // Position du texte
    int text_x = compass2_center_x + (int)(text_radius * cos(angle_rad));
    int text_y = compass2_center_y + (int)(text_radius * sin(angle_rad));

    // CrÃƒÂ©er le label
    lv_obj_t *label = lv_label_create(screen);
    char text[8];
    snprintf(text, sizeof(text), "%d", angle);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);

    // Centrer le texte sur la position calculÃƒÂ©e
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
  // PICTOGRAMME CLOCK (ÃƒÂ  gauche du cadre CLOCK)
  // Image: 70x70 pixels
  // Position: 20px du bord gauche (dÃƒÂ©calÃƒÂ© 5px), remontÃƒÂ© de 15px
  // ========================================
  lv_obj_t *clock_picto = lv_img_create(clock_frame);
  lv_img_set_src(clock_picto, &picto_clock70x70TCA);
  lv_obj_set_pos(clock_picto, 5, 42);  // 20px gauche (25-5), 42px haut (57-15)
  Serial.println("[Data] Pictogramme clock crÃƒÂ©ÃƒÂ©");

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
  // PICTOGRAMME DEPTH (ÃƒÂ  gauche du cadre DEPTH)
  // Image: 66x70 pixels
  // Position: 25px du bord gauche, remontÃƒÂ© de 15px
  // ========================================
  lv_obj_t *depth_picto = lv_img_create(deepth_frame);
  lv_img_set_src(depth_picto, &picto_deepth66x70TCA);
  lv_obj_set_pos(depth_picto, 25, 41);  // 25px gauche, 41px haut (56-15)

  Serial.println("[Data] Pictogramme depth crÃƒÂ©ÃƒÂ©");

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
  // PICTOGRAMME GWD (ÃƒÂ  gauche du cadre)
  // Image: 80x80 pixels
  // Position: 25px du bord gauche, remontÃƒÂ© au maximum
  // ========================================
  lv_obj_t *gwd_picto = lv_img_create(gwd_frame);
  lv_img_set_src(gwd_picto, &picto_GWD2_80x80_TC);
  lv_obj_set_pos(gwd_picto, 25, 0);  // 25px gauche, 0px haut

  Serial.println("[Data] Pictogramme GWD crÃƒÂ©ÃƒÂ©");

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
  // PICTOGRAMME BATTERY (ÃƒÂ  gauche du cadre SOC)
  // Image: 80x54 pixels
  // Position: 25px du bord gauche, remontÃƒÂ© de 15px
  // ========================================
  lv_obj_t *soc_picto = lv_img_create(soc_frame);
  lv_img_set_src(soc_picto, &picto_battery80x54TCA);
  lv_obj_set_pos(soc_picto, 25, 8);  // 25px gauche, 8px haut (23-15)

  Serial.println("[Data] Pictogramme battery SOC crÃƒÂ©ÃƒÂ©");

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
  // PICTOGRAMME CURRENT (ÃƒÂ  gauche du cadre AMP)
  // Image: 70x68 pixels
  // Position: 25px du bord gauche, remontÃƒÂ© de 15px
  // ========================================
  lv_obj_t *amp_picto = lv_img_create(amp_frame);
  lv_img_set_src(amp_picto, &picto_current70x68TCA);
  lv_obj_set_pos(amp_picto, 25, 1);  // 25px gauche, 1px haut (16-15)

  Serial.println("[Data] Pictogramme current AMP crÃƒÂ©ÃƒÂ©");

  // ========================================
  // PLACEHOLDER: "Affichage donnÃƒÂ©es" (EN COMMENTAIRE)
  // CentrÃƒÂ© sur l'ÃƒÂ©cran
  // ========================================
  // label_placeholder = lv_label_create(screen);
  // lv_label_set_text(label_placeholder, "Affichage donnees");
  // lv_obj_set_style_text_font(label_placeholder, &lv_font_montserrat_48, 0);
  // lv_obj_set_style_text_color(label_placeholder, lv_color_hex(0x888888), 0);
  // lv_obj_align(label_placeholder, LV_ALIGN_CENTER, 0, 0);

  // ========================================
  // LIGNE D'Ãƒâ€°TAT (bas - version et messages)
  // ========================================
  label_version = lv_label_create(screen);
  lv_label_set_text(label_version, version);
  lv_obj_set_style_text_font(label_version, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(label_version, lv_color_hex(0x666666), 0);
  lv_obj_align(label_version, LV_ALIGN_BOTTOM_LEFT, 10, -5);

  // ========================================
  // LABEL WIFI STATUS (bas droite)
  // v1.08c: TEXTE INITIAL NON VIDE pour que LVGL alloue la taille
  // ========================================
  label_wifi_status = lv_label_create(screen);
  lv_label_set_text(label_wifi_status, "Initialisation WiFi...");
  lv_obj_set_style_text_font(label_wifi_status, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(label_wifi_status, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(label_wifi_status, LV_ALIGN_BOTTOM_RIGHT, -10, -5);

  Serial.println("[Data] Labels version et WiFi status crÃ©Ã©s");

  // RafraÃ®chissement
  lv_refr_now(NULL);

  Serial.println("[Data] âœ“ Ã‰cran principal crÃ©Ã© (7 cadres, graduations 30Â°)");
}

// ========================================
// ACCESSEURS POUR LES FRAMES
// ========================================
lv_obj_t *getWindFrame() {
  return wind_frame;
}
lv_obj_t *getHdgFrame() {
  return hdg_frame;
}
lv_obj_t *getClockFrame() {
  return clock_frame;
}
lv_obj_t *getDepthFrame() {
  return deepth_frame;
}
lv_obj_t *getGwdFrame() {
  return gwd_frame;
}
lv_obj_t *getSocFrame() {
  return soc_frame;
}
lv_obj_t *getAmpFrame() {
  return amp_frame;
}
lv_obj_t *getWindVaneTriangle() {
  return wind_vane_triangle;
}
lv_obj_t *getWifiStatusLabel() {
  return label_wifi_status;
}
