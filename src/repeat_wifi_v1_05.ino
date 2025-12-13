// ========================================
// Fichier: repeat_wifi_v1_04.ino
// Version 1.04 - Affichage des donnees numeriques
// 
// Repeteur NMEA WiFi avec affichage graphique
// Reception UDP depuis Actisense W2K-1 en format ASCII N2K
// 
// Materiel: Waveshare ESP32-S3 Touch LCD 5B
// WiFi: Actisense W2K-1 (mode AP)
// Format: ASCII N2K (PGN complets)
// 
// CHANGEMENTS v1.04:
// - Ajout module display_values pour affichage donnees
// - Appel createDataLabels() et updateDataValues()
// 
// IMPORTANT: 
// 1. Modifier config.h avec votre SSID et mot de passe
// 2. Configurer le W2K-1 en mode ASCII N2K, port 60002
// 3. Fichier Splash_screen_vierge341x200TC.c requis pour le splash
// ========================================

// ========================================
// VÃƒâ€°RIFICATION VERSION LVGL
// ========================================
#include <lvgl.h>

#if LV_VERSION_CHECK(9, 0, 0)
#error "ERREUR: LVGL 9.x dÃƒÂ©tectÃƒÂ© ! Ce projet nÃƒÂ©cessite LVGL 8.4.0"
#endif

#if !LV_VERSION_CHECK(8, 4, 0)
#warning "Attention: Version LVGL diffÃƒÂ©rente de 8.4.0"
#endif

// ========================================
// INCLUSIONS BIBLIOTHÃƒË†QUES
// ========================================
#include <Arduino.h>
#include <ESP_IOExpander_Library.h>
#include <esp_display_panel.hpp>

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
#include "display_values.h"  // v1.04 - Affichage donnees

// ========================================
// NAMESPACES
// ========================================
using namespace esp_panel::board;
using namespace esp_panel::drivers;

// ========================================
// PERSONNALISATION
// Modifiez ces valeurs selon votre installation
// ========================================
const char* BOAT_NAME = "ALBA III";      // Nom du bateau affichÃƒÂ©
const char* FIRMWARE_VERSION = "v1.04";  // Version du firmware

// ========================================
// VARIABLES GLOBALES
// ========================================
Board *board = nullptr;      // Pointeur vers le board (ÃƒÂ©cran + tactile)
WiFiManager wifiMgr;         // Gestionnaire WiFi
NmeaData nmeaData;           // DonnÃƒÂ©es NMEA dÃƒÂ©codÃƒÂ©es

uint32_t lastDisplay = 0;    // Timer affichage sÃƒÂ©rie

// ========================================
// SETUP
// ========================================
void setup() 
{
    // ========================================
    // CONFIGURATION SERIAL
    // ========================================
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
    Serial.println("========================================\n");
    
    // ========================================
    // AFFICHAGE INFO VERSIONS
    // ========================================
    Serial.printf("LVGL Version: %d.%d.%d\n", 
                  LVGL_VERSION_MAJOR, 
                  LVGL_VERSION_MINOR, 
                  LVGL_VERSION_PATCH);
    Serial.printf("PSRAM disponible: %d KB\n", 
                  heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024);
    Serial.println();
    
    // ========================================
    // Ãƒâ€°TAPE 1: INITIALISATION BOARD
    // ========================================
    Serial.println("1. Initialisation Board...");
    board = initBoard();
    if (board == nullptr) {
        Serial.println("[ERREUR] Ãƒâ€°chec initialisation Board!");
        while (true) { delay(1000); }
    }
    Serial.println("1. Ã¢Å“â€œ Board OK");
    
    // ========================================
    // Ãƒâ€°TAPE 2: INITIALISATION LVGL
    // ========================================
    Serial.println("2. Initialisation LVGL...");
    if (!initLVGL(board)) {
        Serial.println("[ERREUR] Ãƒâ€°chec initialisation LVGL!");
        while (true) { delay(1000); }
    }
    Serial.println("2. Ã¢Å“â€œ LVGL OK");
    
    // ========================================
    // Ãƒâ€°TAPE 3: AFFICHAGE SPLASH SCREEN (3 secondes)
    // ========================================
    Serial.println("3. Affichage Splash Screen...");
    displaySplash(BOAT_NAME, FIRMWARE_VERSION);
    Serial.println("3. Ã¢Å“â€œ Splash terminÃƒÂ©");
    
    // ========================================
    // Ãƒâ€°TAPE 4: CRÃƒâ€°ATION Ãƒâ€°CRAN PRINCIPAL
    // ========================================
    Serial.println("4. CrÃƒÂ©ation ÃƒÂ©cran donnÃƒÂ©es...");
    createDataScreen(BOAT_NAME, FIRMWARE_VERSION);
    Serial.println("4. Ecran donnees OK");

    // ========================================
    // ETAPE 4b: CREATION LABELS POUR DONNEES
    // ========================================
    Serial.println("4b. Creation labels valeurs...");
    createDataLabels();
/*
    // TEST: Données factices pour vérifier affichage
nmeaData.utcTime = "12:34:56";
nmeaData.hasTime = true;
nmeaData.timeIsValid = true;
nmeaData.depth = 12.5f;
nmeaData.hasDepth = true;
nmeaData.batteryCurrent = -15.3f;
nmeaData.batterySOC = 87;
nmeaData.hasBattery = true;
nmeaData.windSpeedApparent = 12.4f;
nmeaData.hasWindApparent = true;
nmeaData.sog = 5.8f;
nmeaData.hasSOG = true;
nmeaData.cog = 245.0f;
nmeaData.hasCOG = true;
nmeaData.windAngleTrue = 45.0f;
nmeaData.hasWindTrue = true;
updateDataValues(&nmeaData);  // Forcer l'affichage
*/
    Serial.println("4b. Labels valeurs OK");

    // ========================================
    // ETAPE 5: INITIALISATION DONNEES NMEA
    // ========================================
    Serial.println("5. Initialisation donnees NMEA...");
    nmeaData.reset();
    Serial.println("5. Ã¢Å“â€œ DonnÃƒÂ©es NMEA OK");
    
    // ========================================
    // Ãƒâ€°TAPE 6: INITIALISATION WIFI
    // ========================================
    Serial.println("6. Initialisation WiFi...");
    if (!wifiMgr.begin()) {
        Serial.println("[ERREUR] Ãƒâ€°chec initialisation WiFi");
        Serial.println("VÃƒÂ©rifiez config.h et redÃƒÂ©marrez");
        // Continue quand mÃƒÂªme pour afficher l'ÃƒÂ©cran
    } else {
        Serial.println("6. Ã¢Å“â€œ WiFi OK");
    }
    
    Serial.println("\n========================================");
    Serial.println("  SystÃƒÂ¨me initialisÃƒÂ©");
    Serial.println("========================================\n");
    
    // En-tÃƒÂªte pour affichage sÃƒÂ©rie des donnÃƒÂ©es
    Serial.println("HDG:xxx / COG:xxx / SOG:xx.x / AWS:xx.x / TWS:xx.x / AWA:xxx / TWA:xxx / GWD:xxx / MAXW:xx.x / SOC:xxx / AMP:xx.x / Prof:xxx.x / HH:MM:SS");
    Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------");
}

// ========================================
// LOOP
// ========================================
void loop() 
{
    // ========================================
    // TÃƒâ€šCHE 1: GESTIONNAIRE LVGL
    // Doit ÃƒÂªtre appelÃƒÂ© rÃƒÂ©guliÃƒÂ¨rement
    // ========================================
    lv_timer_handler();
    
    // ========================================
    // TÃƒâ€šCHE 2: VÃƒâ€°RIFICATION CONNEXION WIFI
    // ========================================
    if (!wifiMgr.isConnected()) {
        delay(100);
        return;
    }
    
    // ========================================
    // TÃƒâ€šCHE 3: RÃƒâ€°CEPTION DONNÃƒâ€°ES UDP
    // ========================================
    wifiMgr.receiveAndProcess(&nmeaData);
    
    // ========================================
    // TÃƒâ€šCHE 4: AFFICHAGE SÃƒâ€°RIE PÃƒâ€°RIODIQUE
    // ========================================
    if (millis() - lastDisplay >= DISPLAY_UPDATE_INTERVAL) {
        lastDisplay = millis();
        nmeaData.displaySerial();
    }
    
    // ========================================
    // TACHE 5: MISE A JOUR AFFICHAGE LCD
    // ========================================
    updateDataValues(&nmeaData);
    
    // ========================================
    // PAUSE COURTE
    // Ãƒâ€°vite saturation CPU, laisse temps aux tÃƒÂ¢ches systÃƒÂ¨me
    // ========================================
    delay(5);
}
