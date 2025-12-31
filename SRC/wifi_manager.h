// ========================================
// Fichier: wifi_manager.h
// Version 1.08 - Gestion complète WiFi + données
// 
// CHANGEMENTS v1.08:
// - Ajout enum WifiStatus pour états WiFi
// - Méthode update() centralisée (remplace receiveAndProcess)
// - Gestion automatique factices/réelles/"---"
// - Gestion timeout 10s
// - Messages statut WiFi pour affichage
// - Debug Serial.print intégré
// ========================================
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "config.h"
#include "nmea_data.h"
#include "nmea_parser.h"

// ========================================
// ÉNUMÉRATION ÉTATS WIFI
// ========================================
enum WifiStatus {
    WIFI_CONNECTING,    // Connexion en cours
    WIFI_CONNECTED,     // Connecté et stable
    WIFI_LOST           // Connexion perdue
};

// ========================================
// CLASSE WIFI MANAGER
// ========================================
class WiFiManager {
public:
    WiFiManager();
    
    // Initialisation
    bool begin();
    
    // NOUVELLE MÉTHODE PRINCIPALE v1.08
    // Gère tout: WiFi, données, timeout, debug
    void update(NmeaData* data);
    
    // Getters statut
    WifiStatus getStatus() { return currentStatus; }
    const char* getStatusMessage() { return statusMessage; }
    bool isStatusError() { return (currentStatus == WIFI_LOST); }
    
    // Statistiques
    uint32_t getPacketCount() { return packetCount; }
    uint32_t getByteCount() { return byteCount; }
    int getRSSI() { return WiFi.RSSI(); }
    
    // Statut WiFi
   // const char* getStatusMessage() { return statusMessage; }
   // bool isStatusError() { return currentStatus == WIFI_LOST; }

private:
    // WiFi UDP
    WiFiUDP udp;
    char udpBuffer[UDP_BUFFER_SIZE];
    char lineBuffer[NMEA_LINE_BUFFER];
    int lineBufferIndex;
    
    // Statistiques
    uint32_t packetCount;
    uint32_t byteCount;
    
    // Gestion état WiFi
    WifiStatus currentStatus;
    char statusMessage[128];
    uint32_t lastReconnectAttempt;
    uint32_t lastStatusCheck;
    
    // Gestion données
    bool firstDataReceived;
    uint32_t lastDebugDisplay;
    uint32_t lastSerialDisplay;
    
    // Méthodes internes
    bool connectWiFi();
    void checkWifiStatus();
    void updateStatusMessage();
    void receiveAndProcess(NmeaData* data);
    void processChar(char c, NmeaData* data);
    void processLine(NmeaData* data);
    void printDebugInfo(NmeaData* data);
};

#endif // WIFI_MANAGER_H
