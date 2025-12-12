#include "wifi_manager.h"

WiFiManager::WiFiManager() 
    : packetCount(0), byteCount(0), lastReconnectAttempt(0), lineBufferIndex(0) {
    memset(lineBuffer, 0, sizeof(lineBuffer));
}

bool WiFiManager::begin() {
    Serial.println("\n========================================");
    Serial.println("  WiFi Manager - Initialisation");
    Serial.println("========================================\n");
    
    return connectWiFi();
}

bool WiFiManager::connectWiFi() {
    Serial.print("[WiFi] Connexion au W2K-1: ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    uint32_t startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_TIMEOUT) {
            Serial.println("\n[WiFi] ERREUR: Timeout de connexion");
            Serial.println("[WiFi] Verifiez:");
            Serial.println("  - Le SSID dans config.h");
            Serial.println("  - Le mot de passe dans config.h");
            Serial.println("  - Le W2K-1 est allume");
            Serial.println("  - Vous etes a portee WiFi");
            return false;
        }
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\n[WiFi] CONNECTE !");
    Serial.print("[WiFi] IP ESP32: ");
    Serial.println(WiFi.localIP());
    Serial.print("[WiFi] Signal (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    // Démarrage UDP
    if (udp.begin(UDP_PORT)) {
        Serial.print("[UDP] Ecoute sur port: ");
        Serial.println(UDP_PORT);
        Serial.println("[UDP] Format: ASCII N2K");
        Serial.println("[UDP] Pret a recevoir...\n");
        return true;
    } else {
        Serial.println("[UDP] ERREUR: Impossible de demarrer UDP");
        return false;
    }
}

bool WiFiManager::isConnected() {
    if (WiFi.status() != WL_CONNECTED) {
        // Tentative de reconnexion
        if (millis() - lastReconnectAttempt > RECONNECT_DELAY) {
            lastReconnectAttempt = millis();
            Serial.println("\n[WiFi] Connexion perdue, tentative de reconnexion...");
            return connectWiFi();
        }
        return false;
    }
    return true;
}

void WiFiManager::receiveAndProcess(NmeaData* data) {
    int packetSize = udp.parsePacket();
    
    if (packetSize > 0) {
        // Statistiques
        packetCount++;
        byteCount += packetSize;
        
        // Lecture du paquet
        int len = udp.read(udpBuffer, UDP_BUFFER_SIZE - 1);
        if (len > 0) {
            udpBuffer[len] = '\0';
            
            // Traiter chaque caractère
            for (int i = 0; i < len; i++) {
                processChar(udpBuffer[i], data);
            }
        }
    }
}

void WiFiManager::processChar(char c, NmeaData* data) {
    // Accumuler les caractères jusqu'à \n
    if (c == '\n' || c == '\r') {
        if (lineBufferIndex > 0) {
            lineBuffer[lineBufferIndex] = '\0';
            processLine(data);
            lineBufferIndex = 0;
        }
    } else {
        if (lineBufferIndex < NMEA_LINE_BUFFER - 1) {
            lineBuffer[lineBufferIndex++] = c;
        } else {
            // Buffer overflow, reset
            lineBufferIndex = 0;
        }
    }
}

void WiFiManager::processLine(NmeaData* data) {
    // Parser le message ASCII N2K
    N2kMessage msg;
    
    if (NmeaParser::parseN2K_ASCII(lineBuffer, &msg)) {
#if DEBUG_NMEA
        Serial.printf("[N2K] PGN %05X from %02X: %d bytes\n", 
                      msg.pgn, msg.source, msg.dataLength);
#endif
        // Traiter le message
        NmeaParser::processMessage(&msg, data);
    }
}
