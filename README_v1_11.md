# 📋 ALBA III - Version 1.11 - Liste complète des fichiers

**Date** : 22 décembre 2024  
**Projet** : Répéteur NMEA WiFi - Waveshare ESP32-S3 Touch LCD 5B  
**Architecture** : Centralisée et modulaire

---

## 🎯 Nouveautés v1.11

- ✅ **Affichage centralisé** : Heure + offset UTC dans `updateDataValues()`
- ✅ **Loop simplifiée** : 1 appel au lieu de 2 pour l'affichage
- ✅ **Architecture propre** : Parsing → Calcul → Affichage (séparation claire)
- ✅ **Code cohérent** : Plus de dispersion d'affichage
- ✅ **Mémorisation NVS** : Offset UTC sauvegardé (hérité de v1.10e)

---

## 📌 FICHIER PRINCIPAL

### **repeat_wifi_v1_11.ino** - v1.11
- **Rôle** : Point d'entrée du programme, orchestration générale
- **Setup** : Initialise board → LVGL → NVS (lecture offset) → splash → écran → labels → boutons → WiFi
- **Loop** : Appelle `wifiMgr.update()` → `updateDataValues()` → `updateTouchInput()` → `updateWifiStatus()`
- **Passe la main à** : Tous les modules (display_*, wifi_manager, nmea_*)

---

## 🔧 FICHIER DE CONFIGURATION

### **config.h** - v1.02
- **Rôle** : Constantes de configuration (WiFi SSID/password, ports UDP, timeouts, dimensions écran)
- **Tire infos de** : Aucun (fichier de configuration utilisateur)
- **Passe la main à** : Tous les modules (include global)
- **Contient** : `WIFI_SSID`, `WIFI_PASSWORD`, `UDP_PORT`, `SCREEN_WIDTH`, `SCREEN_HEIGHT`, etc.

---

## 📦 MODULES NMEA (Parsing et données)

### **nmea_constants.h** - v1.0
- **Rôle** : Constantes NMEA2000 (PGN, facteurs conversion, valeurs invalides, résolutions)
- **Tire infos de** : Spécifications NMEA2000
- **Passe la main à** : `nmea_parser.h`, `nmea_data.h`
- **Contient** : `PGN_WIND_DATA`, `PGN_BATTERY_STATUS`, facteurs conversion (m/s → kts, rad → deg)

---

### **nmea_data.h** - v1.01
- **Rôle** : Structure `NmeaData` (stockage données décodées + méthode `getTimeWithOffset()` pour calcul heure+offset)
- **Tire infos de** : `nmea_parser` (remplissage), `nmea_constants` (limites/conversions)
- **Passe la main à** : `display_values` (affichage), `wifi_manager` (réception)
- **Contient** : `utcTime`, `windSpeedApparent`, `cog`, `sog`, `depth`, `batterySOC`, etc.

---

### **nmea_parser.h** - v1.01
- **Rôle** : Déclarations classes `NmeaParser` et `NmeaConversions` (parsing + décodage PGN)
- **Tire infos de** : `nmea_data.h`, `nmea_constants.h`
- **Passe la main à** : `nmea_parser.cpp`
- **Déclare** : `parseN2K_ASCII()`, `decodeWindData()`, `decodeBatteryStatus()`, etc.

### **nmea_parser.cpp** - v1.01
- **Rôle** : Parse trames ASCII N2K (format `A<timestamp> <PGN> <data>`) et décode 7 PGN (vent, batterie, profondeur, COG/SOG, cap, heure)
- **Tire infos de** : Paquets UDP bruts (via `wifi_manager`)
- **Passe la main à** : `nmea_data` (remplissage structure via `data->windSpeedApparent = ...`)
- **PGN décodés** : 126992 (heure), 127250 (cap), 127506/127508 (batterie), 128267 (profondeur), 129026 (COG/SOG), 130306 (vent)

---

## 📡 MODULE WIFI

### **wifi_manager.h** - v1.09
- **Rôle** : Déclarations classe `WiFiManager` (états WiFi, gestion connexion/reconnexion, mode simulation)
- **Tire infos de** : `config.h` (SSID, password, ports)
- **Passe la main à** : `wifi_manager.cpp`
- **Déclare** : `begin()`, `update()`, `getStatusMessage()`, enum `WifiStatus`

### **wifi_manager.cpp** - v1.09
- **Rôle** : Connexion WiFi W2K-1, réception paquets UDP, parsing ligne par ligne, mode simulation (si `SIMULATION_MODE=1`)
- **Tire infos de** : W2K-1 (trames UDP ASCII N2K)
- **Passe la main à** : `nmea_parser` (appelle `parseN2K_ASCII()` puis `processMessage()`)
- **Gestion** : Auto-reconnexion (5s), timeout (10s sans données), messages statut WiFi

---

## 🖥️ MODULES DISPLAY (Initialisation)

### **display_init.h** - v1.02
- **Rôle** : Déclarations initialisation board et LVGL
- **Tire infos de** : Aucun (interface matérielle)
- **Passe la main à** : `display_init.cpp`
- **Déclare** : `initBoard()`, `initLVGL()`, `getLCD()`

### **display_init.cpp** - v1.02
- **Rôle** : Initialise board Waveshare (LCD + touch), configure LVGL avec double buffering PSRAM, active backlight
- **Tire infos de** : Hardware (ESP32-S3, GT911 touch)
- **Passe la main à** : LVGL (via `lv_init()`), `display_splash`
- **Alloue** : 2× 1200 KB PSRAM pour buffers LVGL

---

### **display_splash.h** - v1.04
- **Rôle** : Déclarations écran splash démarrage
- **Tire infos de** : Aucun
- **Passe la main à** : `display_splash.cpp`
- **Déclare** : `displaySplash()`

### **display_splash.cpp** - v1.04
- **Rôle** : Affiche écran splash (image fond zoomée 3×, titre "Répéteur WiFi", nom bateau, version) pendant 3s
- **Tire infos de** : Images LV_IMG (splash_screen, logo Victron commenté)
- **Passe la main à** : `display_data` (après 3s, supprime objets splash)
- **Bloquant** : Boucle `lv_timer_handler()` pendant `SPLASH_DURATION_MS`

---

## 🖥️ MODULES DISPLAY (Écran principal)

### **display_data.h** - v1.08
- **Rôle** : Déclarations création écran principal (7 cadres, compas, pictogrammes)
- **Tire infos de** : `config.h` (dimensions), constantes positionnement
- **Passe la main à** : `display_data.cpp`
- **Déclare** : `createDataScreen()`, accesseurs `getWindFrame()`, `getWifiStatusLabel()`, etc.

### **display_data.cpp** - v1.07
- **Rôle** : Crée écran principal (7 cadres : WIND, COG, CLOCK, DEPTH, GWD, SOC, AMP), 2 compas (graduations 30°), arcs rouge/vert, triangle girouette
- **Tire infos de** : Images LV_IMG (pictogrammes), fonts Montserrat
- **Passe la main à** : `display_values` (création labels dynamiques), `display_touch` (zones tactiles)
- **Créé** : Bandeau titre, cadres avec ombres, cercles périphériques, réticules, labels statiques

---

## 🖥️ MODULES DISPLAY (Valeurs dynamiques)

### **display_values.h** - v1.11 ⭐
- **Rôle** : Déclarations affichage valeurs NMEA (centralisé v1.11 : heure + offset inclus)
- **Tire infos de** : `nmea_data.h`
- **Passe la main à** : `display_values.cpp`
- **Déclare** : `createDataLabels()`, `updateDataValues(data, decalage_Horaire)`, `updateWifiStatus()`

### **display_values.cpp** - v1.11 ⭐
- **Rôle** : Crée labels dynamiques (heure, depth, AWS, SOC, etc.) et met à jour affichage avec valeurs NMEA + offset UTC (centralisé v1.11)
- **Tire infos de** : `NmeaData` (via `data->hasTime`, `data->depth`, etc.), `getTimeWithOffset(decalage_Horaire)`
- **Passe la main à** : LVGL (via `lv_label_set_text()`)
- **Affiche** : Heure avec offset, "UTC +X", depth, SOC, AWS, COG (bateau tournant), AWA (triangle), TWA, GWD, profondeur, batterie

---

## 🖥️ MODULES DISPLAY (Tactile)

### **display_touch.h** - v1.10e
- **Rôle** : Déclarations gestion tactile (boutons +/-, veille, reset vent max)
- **Tire infos de** : `nmea_data.h`, `display_data.h`
- **Passe la main à** : `display_touch.cpp`
- **Déclare** : `createClockButtons()`, `createTouchHandler()`, `updateTouchInput()`, `isTouchIn*()`, `sleep_mode`

### **display_touch.cpp** - v1.10e
- **Rôle** : Gère boutons +/- offset UTC (zones élargies +10px), mode veille (overlay 90%), reset vent max, sauvegarde NVS (timer 5s non-bloquant)
- **Tire infos de** : Touch GT911 (via `board->getTouch()`), `decalage_Horaire` (pointeur)
- **Passe la main à** : NVS (Preferences, écriture si offset change), `display_values` (via feedback visuel boutons)
- **Logique NVS** : Timer 5s → lecture NVS → comparaison → écriture si différent → reset flag

---

## 🔄 FLUX DE DONNÉES

```
┌─────────────────────────────────────────────────────────────┐
│                    BOOT (setup)                             │
│  1. initBoard() → Hardware OK                               │
│  2. initLVGL() → PSRAM buffers OK                           │
│  3. NVS.read("utc_offset") → decalage_Horaire = +2          │
│  4. displaySplash() → 3s                                    │
│  5. createDataScreen() → 7 cadres + compas                  │
│  6. createDataLabels() → Labels dynamiques                  │
│  7. createClockButtons() + createTouchHandler() → Tactile   │
│  8. wifiMgr.begin() → Connexion W2K-1 ou simulation         │
└─────────────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                    LOOP (runtime)                           │
│                                                             │
│  1. lv_timer_handler() → LVGL                               │
│  2. wifiMgr.update(&nmeaData)                               │
│     ├─ Réception UDP                                        │
│     ├─ parseN2K_ASCII() → N2kMessage                        │
│     ├─ processMessage() → Décode PGN                        │
│     └─ Remplit nmeaData.utcTime, .cog, .sog, etc.          │
│                                                             │
│  3. updateDataValues(&nmeaData, decalage_Horaire) ⭐ v1.11  │
│     ├─ getTimeWithOffset(decalage_Horaire) → "14:34:56"    │
│     ├─ Affiche heure avec offset                           │
│     ├─ Affiche "UTC +2"                                     │
│     └─ Affiche depth, SOC, AWS, COG, etc.                  │
│                                                             │
│  4. updateTouchInput(board, &nmeaData, &decalage_Horaire)   │
│     ├─ Détecte touch (+ / - / COG / WIND)                  │
│     ├─ Incrémente/décrémente decalage_Horaire              │
│     ├─ Timer 5s → Sauvegarde NVS si changement             │
│     └─ Gestion veille (overlay)                            │
│                                                             │
│  5. updateWifiStatus(message, isError)                      │
│     └─ Affiche statut WiFi (blanc/rouge)                   │
│                                                             │
│  6. delay(5) → Loop ~200Hz                                 │
└─────────────────────────────────────────────────────────────┘
```

---

## 📂 STRUCTURE PROJET

```
repeat_wifi_v1_11/
├── repeat_wifi_v1_11.ino       ⭐ v1.11 (MODIFIÉ)
├── config.h                     v1.02
├── nmea_constants.h             v1.0
├── nmea_data.h                  v1.01
├── nmea_parser.h                v1.01
├── nmea_parser.cpp              v1.01
├── wifi_manager.h               v1.09
├── wifi_manager.cpp             v1.09
├── display_init.h               v1.02
├── display_init.cpp             v1.02
├── display_splash.h             v1.04
├── display_splash.cpp           v1.04
├── display_data.h               v1.08
├── display_data.cpp             v1.07
├── display_values.h             ⭐ v1.11 (MODIFIÉ)
├── display_values.cpp           ⭐ v1.11 (MODIFIÉ)
├── display_touch.h              v1.10e
├── display_touch.cpp            v1.10e
├── esp_panel_board_supported_conf.h
├── esp_utils_conf.h
└── [Images .c]
    ├── triangle62x50TCA.c
    ├── picto_*.c
    ├── sil_boat180x54TCA.c
    └── Splash_screen_vierge341x200TC.c
```

---

## 🎯 FICHIERS MODIFIÉS v1.11 (par rapport à v1.10e)

| Fichier | v1.10e | v1.11 | Changement |
|---------|--------|-------|------------|
| **repeat_wifi_v1_11.ino** | v1.10e | **v1.11** | Loop simplifiée (1 appel au lieu de 2) |
| **display_values.h** | v1.08 | **v1.11** | Signature `updateDataValues(data, offset)` |
| **display_values.cpp** | v1.09 | **v1.11** | Affichage heure+offset centralisé, suppression `updateClockWithOffset()` |

**Tous les autres fichiers** : Inchangés (compatibles v1.11)

---

## 🚀 COMPILATION

### **Étapes**
1. Ouvrir `repeat_wifi_v1_11.ino` dans Arduino IDE
2. Vérifier présence de tous les fichiers dans le dossier
3. Sélectionner carte : **ESP32S3 Dev Module**
4. Compiler et uploader

### **Dépendances Arduino**
- **LVGL** 8.4.0
- **ESP_Panel** (Waveshare)
- **Preferences** (ESP32 - inclus)

---

## 📊 MÉMOIRE

- **PSRAM** : 2× 1200 KB (buffers LVGL) + ~4.5 MB libre
- **Flash** : Code ~500 KB + images ~200 KB
- **NVS** : 1 clé `utc_offset` (4 bytes)

---

## 🎉 FONCTIONNALITÉS v1.11

✅ Affichage 7 types de données (heure, depth, SOC, AWS, COG, GWD, AMP)  
✅ 2 compas (rose des vents + COG avec lettres cardinales)  
✅ Offset UTC mémorisé en NVS (survit aux reboots)  
✅ Boutons +/- tactiles (zones élargies)  
✅ Mode veille (touch cadre COG)  
✅ Reset vent max (touch cadre WIND)  
✅ WiFi auto-reconnexion  
✅ Mode simulation (test sans WiFi)  
✅ Architecture centralisée et propre ⭐

---

**Version** : 1.11  
**Date** : 22 décembre 2024  
**Auteur** : François-Xavier  
**Projet** : ALBA III - Répéteur NMEA WiFi
