// ========================================
// Fichier: repeat_wifi_v1_13.ino
// Version 1.13 - Ajout RTC ESP32 synchro GPS
// 
// CHANGEMENTS v1.13:
// - Ajout PGN 129033 (Time & Date GPS)
// - Sync RTC ESP32 avec heure GPS pure (UTC)
// - Affichage heure = RTC + offset manuel
// - Offset manuel conservé (boutons +/-)
// - Trace fréquence réception PGN 129033
// 
// BASE: v1.11 (architecture centralisée)
// ========================================

// ========================================
// VÉRIFICATION VERSION LVGL
// ========================================
#include <lvgl.h>

#if LV_VERSION_CHECK(9, 0, 0)
#error "ERREUR: LVGL 9.x détecté ! Ce projet nécessite LVGL 8.4.0"
#endif

#if !LV_VERSION_CHECK(8, 4, 0)
#warning "Attention: Version LVGL différente de 8.4.0"
#endif

// ========================================
// INCLUSIONS BIBLIOTHÈQUES
// ========================================
#include <Arduino.h>
#include <ESP_IOExpander_Library.h>
#include <esp_display_panel.hpp>
#include <Preferences.h>
#include <sys/time.h>    // v1.13: RTC ESP32
#include <time.h>        // v1.13: RTC ESP32

// ========================================
// INCLUSIONS MODULES PROJET
// ========================================
#include "config.h"
#include "nmea_data.h"
#include "nmea_parser.h"
#include "wifi_manager.h"
#include "display_init.h"
#include "display_splash.h"
#include "display_data.h"
#include "display_values.h"
#include "display_touch.h"

// ========================================
// NAMESPACES
// ========================================
using namespace esp_panel::board;
using namespace esp_panel::drivers;

// ========================================
// PERSONNALISATION
// ========================================
const char* BOAT_NAME = "ALBA III";
const char* FIRMWARE_VERSION = "v1.14";

// ========================================
// VARIABLES GLOBALES
// ========================================
Board *board = nullptr;
WiFiManager wifiMgr;
NmeaData nmeaData;
int decalage_Horaire = 0;  // Offset UTC manuel (ex: +2 pour UTC+2)

// ========================================
// VARIABLES RTC (v1.13)
// ========================================
bool rtc_synced = false;  // Flag: RTC synchronisée avec GPS ?

// ========================================
// FONCTION RTC: Synchronisation (v1.13)
// Appelée par nmea_parser.cpp lors réception PGN 129033
// ========================================
void setRTCTime(int year, int month, int day, int hour, int minute, int second) {
    struct tm timeinfo = {0};
    timeinfo.tm_year = year - 1900;  // Années depuis 1900
    timeinfo.tm_mon = month - 1;     // Mois 0-11
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    
    time_t t = mktime(&timeinfo);
    struct timeval now = { .tv_sec = t };
    settimeofday(&now, NULL);
    
    Serial.printf("[RTC] Synchronisation: %04d-%02d-%02d %02d:%02d:%02d UTC\n",
                  year, month, day, hour, minute, second);
}

// ========================================
// SETUP
// ========================================
void setup() 
{
    // CONFIGURATION SERIAL
    Serial.setRxBufferSize(2048);
    Serial.setTxBufferSize(2048);
    Serial.begin(SERIAL_BAUD);
    delay(1000);
    
    Serial.println("\n\n\n");
    Serial.println("========================================");
    Serial.printf("  REPEAT WIFI %s\n", FIRMWARE_VERSION);
    Serial.printf("  Bateau: %s\n", BOAT_NAME);
    Serial.println("  Waveshare ESP32-S3 Touch LCD 5B");
    Serial.println("  Format: ASCII N2K");
    Serial.println("  Nouveauté: RTC GPS (PGN 129033)");
    Serial.println("========================================\n");
    
    Serial.printf("LVGL Version: %d.%d.%d\n", 
                  LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
    Serial.printf("PSRAM disponible: %d KB\n", 
                  heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024);
    Serial.println();
    
    // ÉTAPE 1: INITIALISATION BOARD
    Serial.println("1. Initialisation Board...");
    board = initBoard();
    if (board == nullptr) {
        Serial.println("[ERREUR] Échec initialisation Board!");
        while (true) { delay(1000); }
    }
    Serial.println("1. ✓ Board OK");
    
    // ÉTAPE 2: INITIALISATION LVGL
    Serial.println("2. Initialisation LVGL...");
    if (!initLVGL(board)) {
        Serial.println("[ERREUR] Échec initialisation LVGL!");
        while (true) { delay(1000); }
    }
    Serial.println("2. ✓ LVGL OK");
    
    // ÉTAPE 2b: LECTURE OFFSET UTC DEPUIS NVS
    Serial.println("2b. Lecture offset UTC...");
    Preferences prefs;
    prefs.begin("mySettings", false);
    decalage_Horaire = prefs.getInt("utc_offset", 0);
    prefs.end();
    Serial.printf("[PREFS] Offset boot: %+d\n", decalage_Horaire);
    
    // ÉTAPE 3: AFFICHAGE SPLASH SCREEN
    Serial.println("3. Affichage Splash Screen...");
    displaySplash(BOAT_NAME, FIRMWARE_VERSION);
    Serial.println("3. ✓ Splash terminé");
    
    // ÉTAPE 4: CRÉATION ÉCRAN PRINCIPAL
    Serial.println("4. Création écran données...");
    createDataScreen(BOAT_NAME, FIRMWARE_VERSION);
    Serial.println("4. ✓ Écran données OK");

    // ÉTAPE 5: CRÉATION LABELS VALEURS
    Serial.println("5. Création labels valeurs...");
    createDataLabels();
    Serial.println("5. ✓ Labels valeurs OK");
    
    // ÉTAPE 5b: CRÉATION BOUTONS TACTILES
    Serial.println("5b. Création boutons tactiles...");
    createClockButtons();
    createTouchHandler(board);
    Serial.println("5b. ✓ Boutons tactiles créés");

    // ÉTAPE 6: INITIALISATION DONNÉES NMEA
    Serial.println("6. Initialisation données NMEA...");
    nmeaData.reset();
    Serial.println("6. ✓ Données NMEA OK");
    
    // ÉTAPE 7: INITIALISATION WIFI
    Serial.println("7. Initialisation WiFi...");
    if (!wifiMgr.begin()) {
        Serial.println("[ERREUR] Échec initialisation WiFi");
        Serial.println("Affichage '---' en attente de données");
    } else {
        Serial.println("7. ✓ WiFi OK");
    }
    
    // ÉTAPE 8: FORCER 1ER AFFICHAGE MESSAGE WIFI
    Serial.println("8. Forcer affichage message WiFi initial...");
    updateWifiStatus(wifiMgr.getStatusMessage(), wifiMgr.isStatusError());
    Serial.println("8. ✓ Message WiFi affiché");
    
    Serial.println("\n========================================");
    Serial.println("  Système initialisé - v1.13");
    Serial.println("========================================");
    Serial.println("  NOUVEAUTÉS v1.13:");
    Serial.println("  - RTC synchro GPS (PGN 129033)");
    Serial.println("  - Heure = RTC + offset manuel");
    Serial.println("  - Offset network ignoré");
    Serial.println("  - Trace fréquence PGN 129033");
    Serial.println("========================================\n");
    
    Serial.println("HDG:xxx / COG:xxx / SOG:xx.x / AWS:xx.x / TWS:xx.x / AWA:xxx / TWA:xxx / GWD:xxx / MAXW:xx.x / SOC:xxx / AMP:xx.x / Prof:xxx.x / HH:MM:SS");
    Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------");
}

// ========================================
// LOOP - VERSION v1.13 (identique v1.11)
// Architecture centralisée inchangée
// ========================================
void loop() 
{
    lv_timer_handler();
    wifiMgr.update(&nmeaData);
    updateDataValues(&nmeaData, decalage_Horaire);  // Lecture RTC ici
    updateTouchInput(board, &nmeaData, &decalage_Horaire);
    updateWifiStatus(wifiMgr.getStatusMessage(), wifiMgr.isStatusError());
    delay(5);
}
