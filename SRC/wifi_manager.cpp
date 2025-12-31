// ========================================
// Fichier: wifi_manager.cpp
// Version 1.14 - Traces [N2K] commentées
// 
// CHANGEMENTS v1.14:
// - Commentaire des traces [N2K] PGN ...
// - Conservation ligne HDG:xxx / COG:xxx / ...
// - Conservation traces RTC et boot
// 
// CHANGEMENTS v1.09:
// - Simplification: suppression données factices
// - Affichage "---" au démarrage jusqu'à réception données
// - WiFi perdue → Conservation dernières valeurs affichées
// - Seul le message WiFi indique l'état de connexion
// 
// LOGIQUE SIMPLIFIÉE:
// 1. Démarrage → "---" partout
// 2. Données NMEA reçues → Affichage valeurs réelles
// 3. WiFi perdue → Garder dernières valeurs + message rouge
// ========================================
#include "wifi_manager.h"

// ========================================
// CONSTRUCTEUR
// ========================================
WiFiManager::WiFiManager() 
    : packetCount(0), 
      byteCount(0), 
      lastReconnectAttempt(0), 
      lineBufferIndex(0),
      currentStatus(WIFI_CONNECTING),
      lastStatusCheck(0),
      firstDataReceived(false),
      lastDebugDisplay(0),
      lastSerialDisplay(0)
{
    memset(lineBuffer, 0, sizeof(lineBuffer));
    memset(udpBuffer, 0, sizeof(udpBuffer));
    memset(statusMessage, 0, sizeof(statusMessage));
}

// ========================================
// INITIALISATION
// ========================================
bool WiFiManager::begin() {
    Serial.println("\n========================================");
    Serial.println("  WiFi Manager - Initialisation");
    Serial.println("========================================\n");
    
    // ========================================
    // MODE SIMULATION (v1.10b)
    // Shunt WiFi pour tester le tactile immédiatement
    // ========================================
    #if SIMULATION_MODE
        Serial.println("╔════════════════════════════════════╗");
        Serial.println("║   MODE SIMULATION ACTIVÉ            ║");
        Serial.println("║   WiFi shunté - Pas de connexion   ║");
        Serial.println("║   Tactile testable immédiatement    ║");
        Serial.println("╚════════════════════════════════════╝\n");
        
        currentStatus = WIFI_CONNECTED;
        snprintf(statusMessage, sizeof(statusMessage), 
                 "MODE SIMULATION - Pas de WiFi reel");
        firstDataReceived = true;  // Éviter attente données
        
        Serial.println("[WiFi] ✓ Mode simulation activé");
        Serial.println("[WiFi] ✓ Affichage '---' partout (pas de données)");
        Serial.println("[WiFi] ✓ Système tactile prêt à être testé\n");
        
        return true;  // Succès immédiat
    #endif
    
    // ========================================
    // MODE NORMAL - WiFi réel
    // ========================================
    currentStatus = WIFI_CONNECTING;
    updateStatusMessage();
    
    return connectWiFi();
}

// ========================================
// CONNEXION WIFI
// ========================================
bool WiFiManager::connectWiFi() {
    Serial.print("[WiFi] Connexion au W2K-1: ");
    Serial.println(WIFI_SSID);
    
    currentStatus = WIFI_CONNECTING;
    updateStatusMessage();
    
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
            currentStatus = WIFI_LOST;
            updateStatusMessage();
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
    
    currentStatus = WIFI_CONNECTED;
    updateStatusMessage();
    
    // Démarrage UDP
    if (udp.begin(UDP_PORT)) {
        Serial.print("[UDP] Ecoute sur port: ");
        Serial.println(UDP_PORT);
        Serial.println("[UDP] Format: ASCII N2K");
        Serial.println("[UDP] Pret a recevoir...\n");
        return true;
    } else {
        Serial.println("[UDP] ERREUR: Impossible de demarrer UDP");
        currentStatus = WIFI_LOST;
        updateStatusMessage();
        return false;
    }
}

// ========================================
// VÉRIFICATION ÉTAT WIFI
// ========================================
void WiFiManager::checkWifiStatus() {
    WifiStatus oldStatus = currentStatus;
    
    if (WiFi.status() != WL_CONNECTED) {
        currentStatus = WIFI_LOST;
        
        // Tentative de reconnexion toutes les 5 secondes
        if (millis() - lastReconnectAttempt > RECONNECT_DELAY) {
            lastReconnectAttempt = millis();
            Serial.println("\n[WiFi] Connexion perdue, tentative de reconnexion...");
            
            if (connectWiFi()) {
                currentStatus = WIFI_CONNECTED;
                Serial.println("[WiFi] Reconnexion reussie !");
            }
        }
    } else {
        currentStatus = WIFI_CONNECTED;
    }
    
    // MAJ message si changement d'état
    if (oldStatus != currentStatus) {
        updateStatusMessage();
    }
}

// ========================================
// MISE À JOUR MESSAGE STATUT
// ========================================
void WiFiManager::updateStatusMessage() {
    switch (currentStatus) {
        case WIFI_CONNECTING:
            snprintf(statusMessage, sizeof(statusMessage), 
                     "Connexion wifi en cours a %s", WIFI_SSID);
            break;
            
        case WIFI_CONNECTED:
            snprintf(statusMessage, sizeof(statusMessage), 
                     "Connexion a %s OK", WIFI_SSID);
            break;
            
        case WIFI_LOST:
            snprintf(statusMessage, sizeof(statusMessage), 
                     "Connexion wifi perdue. Tentative de reconnexion...");
            break;
    }
}

// ========================================
// RÉCEPTION ET TRAITEMENT UDP
// ========================================
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

// ========================================
// TRAITEMENT CARACTÈRE
// ========================================
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

// ========================================
// TRAITEMENT LIGNE
// ========================================
void WiFiManager::processLine(NmeaData* data) {
    // Parser le message ASCII N2K
    N2kMessage msg;
    
    if (NmeaParser::parseN2K_ASCII(lineBuffer, &msg)) {
// V1.14: Traces [N2K] commentées pour alléger le Serial (garder ligne HDG:xxx / COG:xxx)
// #if DEBUG_NMEA
//         Serial.printf("[N2K] PGN %05X from %02X: %d bytes\n", 
//                       msg.pgn, msg.source, msg.dataLength);
// #endif
        // Traiter le message
        NmeaParser::processMessage(&msg, data);
        
        // Détecter première donnée reçue
        if (!firstDataReceived && data->lastUpdate > 0) {
            firstDataReceived = true;
            Serial.println("\n[WiFi] ✓ Premiere donnee NMEA recue -> affichage valeurs reelles");
            Serial.println("------------------------------------------------------------------------\n");
        }
    }
}

// ========================================
// DEBUG INFO
// ========================================
void WiFiManager::printDebugInfo(NmeaData* data) {
    // Debug détaillé toutes les 2 secondes
    if (millis() - lastDebugDisplay >= 2000) {
        lastDebugDisplay = millis();
        
        Serial.println("\n[DEBUG FLAGS]");
        Serial.printf("  hasTime=%d hasDepth=%d hasBattery=%d\n", 
                      data->hasTime, data->hasDepth, data->hasBattery);
        Serial.printf("  hasWindApparent=%d hasWindTrue=%d\n",
                      data->hasWindApparent, data->hasWindTrue);
        Serial.printf("  hasSOG=%d hasCOG=%d hasHeading=%d\n",
                      data->hasSOG, data->hasCOG, data->hasHeading);
        
        Serial.println("[DEBUG VALEURS]");
        Serial.printf("  utcTime='%s' depth=%.1f SOC=%d\n",
                      data->utcTime.c_str(), data->depth, data->batterySOC);
        Serial.printf("  AWS=%.1f AWA=%.0f SOG=%.1f COG=%.0f\n",
                      data->windSpeedApparent, data->windAngleApparent,
                      data->sog, data->cog);
        
        if (data->lastUpdate > 0) {
            Serial.printf("  lastUpdate=%lu (il y a %lu ms)\n",
                          data->lastUpdate, millis() - data->lastUpdate);
        } else {
            Serial.println("  lastUpdate=0 (aucune donnee recue)");
        }
        
        Serial.printf("[DEBUG WIFI] Status=%d Message='%s'\n\n",
                      currentStatus, statusMessage);
    }
    
    // Affichage série données (toutes les 1 secondes)
    if (millis() - lastSerialDisplay >= DISPLAY_UPDATE_INTERVAL) {
        lastSerialDisplay = millis();
        data->displaySerial();
    }
}

// ========================================
// MÉTHODE PRINCIPALE UPDATE (v1.09)
// LOGIQUE ULTRA-SIMPLIFIÉE:
// - WIFI_CONNECTING: Attendre connexion
// - WIFI_CONNECTED: Recevoir données UDP
// - WIFI_LOST: Ne rien faire (garder dernières valeurs)
// ========================================
void WiFiManager::update(NmeaData* data) {
    // ========================================
    // MODE SIMULATION (v1.10b)
    // Shunt WiFi - Ne rien faire, juste afficher debug
    // ========================================
    #if SIMULATION_MODE
        // Debug périodique réduit en mode simulation
        static unsigned long last_sim_msg = 0;
        if (millis() - last_sim_msg >= 5000) {
            last_sim_msg = millis();
            Serial.println("[WiFi SIMULATION] Mode actif - Pas de données WiFi");
            Serial.println("[WiFi SIMULATION] Affichage '---' partout");
            Serial.println("[WiFi SIMULATION] Testez le tactile maintenant!\n");
        }
        return;  // Pas de traitement WiFi
    #endif
    
    // ========================================
    // MODE NORMAL - Gestion WiFi réelle
    // ========================================
    
    // Vérification état WiFi toutes les 10 secondes
    if (millis() - lastStatusCheck >= 10000) {
        lastStatusCheck = millis();
        checkWifiStatus();
    }
    
    // Gestion selon état WiFi
    switch (currentStatus) {
        case WIFI_CONNECTING:
            // Attente connexion → ne rien faire
            break;
            
        case WIFI_CONNECTED:
            // Connecté → recevoir données UDP
            receiveAndProcess(data);
            break;
            
        case WIFI_LOST:
            // v1.09: Connexion perdue → GARDER dernières valeurs affichées
            // Le message WiFi rouge suffit comme indication
            break;
    }
    
    // Debug périodique
    printDebugInfo(data);
}
