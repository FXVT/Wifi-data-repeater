#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "config.h"
#include "nmea_data.h"
#include "nmea_parser.h"

class WiFiManager {
public:
    WiFiManager();
    
    // Initialisation
    bool begin();
    
    // Connexion WiFi
    bool connectWiFi();
    
    // Vérification de la connexion
    bool isConnected();
    
    // Réception et traitement des données UDP
    void receiveAndProcess(NmeaData* data);
    
    // Statistiques
    uint32_t getPacketCount() { return packetCount; }
    uint32_t getByteCount() { return byteCount; }
    int getRSSI() { return WiFi.RSSI(); }

private:
    WiFiUDP udp;
    char udpBuffer[UDP_BUFFER_SIZE];
    char lineBuffer[NMEA_LINE_BUFFER];
    int lineBufferIndex;
    
    uint32_t packetCount;
    uint32_t byteCount;
    uint32_t lastReconnectAttempt;
    
    // Traitement des caractères reçus
    void processChar(char c, NmeaData* data);
    void processLine(NmeaData* data);
};

#endif // WIFI_MANAGER_H
