// ========================================
// Fichier: repeat_wifi_v1_03.ino
// Version 1.03 - Écran principal avec 7 cadres ombrés
// 
// RÃ©pÃ©teur NMEA WiFi avec affichage graphique
// RÃ©ception UDP depuis Actisense W2K-1 en format ASCII N2K
// 
// MatÃ©riel: Waveshare ESP32-S3 Touch LCD 5B
// WiFi: Actisense W2K-1 (mode AP)
// Format: ASCII N2K (PGN complets)
// 
// IMPORTANT: 
// 1. Modifier config.h avec votre SSID et mot de passe
// 2. Configurer le W2K-1 en mode ASCII N2K, port 60002
// 3. Fichier Logo_Victron120x120TC.c requis pour le splash
// ========================================

// ========================================
// VÃ‰RIFICATION VERSION LVGL
// ========================================
#include <lvgl.h>

#if LV_VERSION_CHECK(9, 0, 0)
#error "ERREUR: LVGL 9.x dÃ©tectÃ© ! Ce projet nÃ©cessite LVGL 8.4.0"
#endif

#if !LV_VERSION_CHECK(8, 4, 0)
#warning "Attention: Version LVGL diffÃ©rente de 8.4.0"
#endif

// ========================================
// INCLUSIONS BIBLIOTHÃˆQUES
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

// ========================================
// NAMESPACES
// ========================================
using namespace esp_panel::board;
using namespace esp_panel::drivers;

// ========================================
// PERSONNALISATION
// Modifiez ces valeurs selon votre installation
// ========================================
const char* BOAT_NAME = "ALBA III";      // Nom du bateau affichÃ©
const char* FIRMWARE_VERSION = "v1.04";  // Version du firmware

// ========================================
// VARIABLES GLOBALES
// ========================================
Board *board = nullptr;      // Pointeur vers le board (Ã©cran + tactile)
WiFiManager wifiMgr;         // Gestionnaire WiFi
NmeaData nmeaData;           // DonnÃ©es NMEA dÃ©codÃ©es

uint32_t lastDisplay = 0;    // Timer affichage sÃ©rie

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
    // Ã‰TAPE 1: INITIALISATION BOARD
    // ========================================
    Serial.println("1. Initialisation Board...");
    board = initBoard();
    if (board == nullptr) {
        Serial.println("[ERREUR] Ã‰chec initialisation Board!");
        while (true) { delay(1000); }
    }
    Serial.println("1. âœ“ Board OK");
    
    // ========================================
    // Ã‰TAPE 2: INITIALISATION LVGL
    // ========================================
    Serial.println("2. Initialisation LVGL...");
    if (!initLVGL(board)) {
        Serial.println("[ERREUR] Ã‰chec initialisation LVGL!");
        while (true) { delay(1000); }
    }
    Serial.println("2. âœ“ LVGL OK");
    
    // ========================================
    // Ã‰TAPE 3: AFFICHAGE SPLASH SCREEN (3 secondes)
    // ========================================
    Serial.println("3. Affichage Splash Screen...");
    displaySplash(BOAT_NAME, FIRMWARE_VERSION);
    Serial.println("3. âœ“ Splash terminÃ©");
    
    // ========================================
    // Ã‰TAPE 4: CRÃ‰ATION Ã‰CRAN PRINCIPAL
    // ========================================
    Serial.println("4. CrÃ©ation Ã©cran donnÃ©es...");
    createDataScreen(BOAT_NAME, FIRMWARE_VERSION);
    Serial.println("4. âœ“ Ã‰cran donnÃ©es OK");
    
    // ========================================
    // Ã‰TAPE 5: INITIALISATION DONNÃ‰ES NMEA
    // ========================================
    Serial.println("5. Initialisation donnÃ©es NMEA...");
    nmeaData.reset();
    Serial.println("5. âœ“ DonnÃ©es NMEA OK");
    
    // ========================================
    // Ã‰TAPE 6: INITIALISATION WIFI
    // ========================================
    Serial.println("6. Initialisation WiFi...");
    if (!wifiMgr.begin()) {
        Serial.println("[ERREUR] Ã‰chec initialisation WiFi");
        Serial.println("VÃ©rifiez config.h et redÃ©marrez");
        // Continue quand mÃªme pour afficher l'Ã©cran
    } else {
        Serial.println("6. âœ“ WiFi OK");
    }
    
    Serial.println("\n========================================");
    Serial.println("  SystÃ¨me initialisÃ©");
    Serial.println("========================================\n");
    
    // En-tÃªte pour affichage sÃ©rie des donnÃ©es
    Serial.println("HDG:xxx / COG:xxx / SOG:xx.x / AWS:xx.x / TWS:xx.x / AWA:xxx / TWA:xxx / GWD:xxx / MAXW:xx.x / SOC:xxx / AMP:xx.x / Prof:xxx.x / HH:MM:SS");
    Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------");
}

// ========================================
// LOOP
// ========================================
void loop() 
{
    // ========================================
    // TÃ‚CHE 1: GESTIONNAIRE LVGL
    // Doit Ãªtre appelÃ© rÃ©guliÃ¨rement
    // ========================================
    lv_timer_handler();
    
    // ========================================
    // TÃ‚CHE 2: VÃ‰RIFICATION CONNEXION WIFI
    // ========================================
    if (!wifiMgr.isConnected()) {
        delay(100);
        return;
    }
    
    // ========================================
    // TÃ‚CHE 3: RÃ‰CEPTION DONNÃ‰ES UDP
    // ========================================
    wifiMgr.receiveAndProcess(&nmeaData);
    
    // ========================================
    // TÃ‚CHE 4: AFFICHAGE SÃ‰RIE PÃ‰RIODIQUE
    // ========================================
    if (millis() - lastDisplay >= DISPLAY_UPDATE_INTERVAL) {
        lastDisplay = millis();
        nmeaData.displaySerial();
    }
    
    // ========================================
    // PAUSE COURTE
    // Ã‰vite saturation CPU, laisse temps aux tÃ¢ches systÃ¨me
    // ========================================
    delay(5);
}
