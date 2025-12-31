// ========================================
// Fichier: config.h
// Version 1.10b - Ajout MODE SIMULATION
// ========================================
#ifndef CONFIG_H
#define CONFIG_H

// ===== CONFIGURATION WiFi W2K-1 =====
// À MODIFIER avec vos valeurs réelles
#define WIFI_SSID "w2k-300353"           // Format: w2k-<numéro de série>
#define WIFI_PASSWORD "Albaalba.03"      // Mot de passe au dos du W2K-1 (8 caractères)
#define W2K1_IP "192.168.4.1"            // IP fixe du W2K-1 en mode AP
#define UDP_PORT 60002                   // Port UDP (même que votre tablette)

// ===== CONFIGURATION DEBUG =====
#define SERIAL_BAUD 115200               // Vitesse moniteur série
#define DEBUG_NMEA 1                     // 1 = afficher trames reçues, 0 = silencieux
#define WIFI_TIMEOUT 20000               // Timeout connexion WiFi (ms)
#define RECONNECT_DELAY 5000             // Délai entre tentatives de reconnexion (ms)

// ===== MODE SIMULATION (v1.10b) =====
// Pour tester le tactile sans attendre la connexion WiFi
#define SIMULATION_MODE 0                // 1 = WiFi shunté (test tactile), 0 = WiFi réel
                                         // Changez à 1 pour bypasser connexion WiFi

// ===== CONFIGURATION BUFFER =====
#define UDP_BUFFER_SIZE 512              // Taille buffer UDP
#define NMEA_LINE_BUFFER 512             // Buffer pour lignes NMEA complètes

// ===== CONFIGURATION AFFICHAGE SERIE =====
#define DISPLAY_UPDATE_INTERVAL 1000     // Affichage toutes les 1000ms (1Hz)
#define DATA_TIMEOUT 5000                // Timeout données (ms)

// ===== CONFIGURATION DISPLAY LVGL =====
#define SCREEN_WIDTH 1024                // Largeur écran Waveshare 5"
#define SCREEN_HEIGHT 600                // Hauteur écran Waveshare 5"
#define SPLASH_DURATION_MS 3000          // Durée splash screen (ms)

#endif // CONFIG_H
