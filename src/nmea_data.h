/*
 * nmea_data.h - Version 1.01
 * 
 * Structures de données pour le répéteur NMEA
 * Ajout: COG, calcul TWS/TWA/GWD avec COG, gestion heure avec "?"
 */

#ifndef NMEA_DATA_H
#define NMEA_DATA_H

#include <Arduino.h>

// Structure des données NMEA décodées
struct NmeaData {
    // Vent apparent (en nœuds et degrés)
    float windSpeedApparent;      // kts (AWS)
    float windAngleApparent;      // 0-359° (AWA)
    
    // Vent réel (en nœuds et degrés)
    float windSpeedTrue;          // kts (TWS)
    float windAngleTrue;          // 0-359° (TWA)
    
    // Vent maximum (vitesses seulement)
    float windSpeedMaxApp;        // kts (MAXW apparent)
    float windSpeedMaxTrue;       // kts (MAXW réel)
    
    // Navigation (en nœuds et degrés)
    float heading;                // 0-359° (HDG)
    float cog;                    // 0-359° (COG - Course Over Ground)
    float sog;                    // kts (SOG)
    
    // Profondeur (en mètres)
    float depth;                  // m (Prof)
    
    // Batterie
    float batteryVoltage;         // V
    float batteryCurrent;         // A (AMP, négatif = décharge)
    uint8_t batterySOC;           // % (SOC)
    
    // Heure
    String utcTime;               // HH:MM:SS ou HH:MM:SS?
    String lastValidTime;         // Dernière heure valide mémorisée
    bool timeIsValid;             // true = heure valide, false = heure douteuse (afficher "?")
    
    // Flags de validité
    bool hasWindApparent;
    bool hasWindTrue;
    bool hasHeading;
    bool hasCOG;
    bool hasSOG;
    bool hasDepth;
    bool hasBattery;
    bool hasTime;
    
    uint32_t lastUpdate;          // millis()
    
    // Constructeur
    NmeaData() {
        reset();
    }
    
    // Reset des données
    void reset() {
        windSpeedApparent = 0.0f;
        windAngleApparent = 0.0f;
        windSpeedTrue = 0.0f;
        windAngleTrue = 0.0f;
        windSpeedMaxApp = 0.0f;
        windSpeedMaxTrue = 0.0f;
        heading = 0.0f;
        cog = 0.0f;
        sog = 0.0f;
        depth = 0.0f;
        batteryVoltage = 0.0f;
        batteryCurrent = 0.0f;
        batterySOC = 0;
        utcTime = "--:--:--";
        lastValidTime = "--:--:--";
        timeIsValid = true;
        
        hasWindApparent = false;
        hasWindTrue = false;
        hasHeading = false;
        hasCOG = false;
        hasSOG = false;
        hasDepth = false;
        hasBattery = false;
        hasTime = false;
        
        lastUpdate = 0;
    }
    
    // Calcul du vent réel (TWS/TWA/GWD) à partir du vent apparent
    // Méthode: loi des cosinus + trigonométrie
    // Utilise COG (Course Over Ground) au lieu de HDG
    void calculateTrueWind() {
        if (!hasWindApparent || !hasSOG || !hasCOG) {
            return; // Impossible de calculer sans AWS/AWA, SOG et COG
        }
        
        // Cas spécial: bateau à l'arrêt
        if (sog == 0) {
            windSpeedTrue = windSpeedApparent;
            windAngleTrue = windAngleApparent;
            hasWindTrue = true;
            return;
        }
        
        // ====================================== Calcul de TWS
        // Formule: TWS² = SOG² + AWS² - 2×SOG×AWS×cos(AWA)
        // Source: https://www.yachtd.com/news/trigonometry_and_encryption.html
        float awaRad = windAngleApparent * PI / 180.0f;
        windSpeedTrue = sqrt((sog * sog) + (windSpeedApparent * windSpeedApparent) - 
                             (2.0f * sog * windSpeedApparent * cos(awaRad)));
        
        // ====================================== Calcul de TWD (True Wind Direction)
        if (windSpeedTrue != 0) {
            float F = ((sog * sog) + (windSpeedTrue * windSpeedTrue) - 
                       (windSpeedApparent * windSpeedApparent)) / 
                      (2.0f * windSpeedTrue * sog);
            
            // Protection contre erreur d'arrondi (F doit être entre -1 et 1 pour acos)
            if (F < -1.0f) {
                F = -1.0f;
            }
            if (F > 1.0f) {
                F = 1.0f;
            }
            
            F = PI - acos(F);
            
            float twd;
            if (windAngleApparent > 180.0f) {
                twd = cog - (F * 180.0f / PI);
            } else {
                twd = cog + (F * 180.0f / PI);
            }
            
            // Normaliser TWD (0-359°)
            while (twd > 360.0f) twd -= 360.0f;
            while (twd < 0.0f) twd += 360.0f;
            
            // ====================================== Calcul de TWA
            windAngleTrue = twd - cog;
            if (windAngleTrue < 0.0f) {
                windAngleTrue += 360.0f;
            }
            
            hasWindTrue = true;
        }
    }
    
    // Calcul du vent maximum (vitesse seulement)
    void updateWindMax() {
        // Calculer TWS/TWA d'abord
        calculateTrueWind();
        
        // Max apparent
        if (hasWindApparent && windSpeedApparent > windSpeedMaxApp) {
            windSpeedMaxApp = windSpeedApparent;
        }
        
        // Max réel
        if (hasWindTrue && windSpeedTrue > windSpeedMaxTrue) {
            windSpeedMaxTrue = windSpeedTrue;
        }
    }
    
    // Reset du max apparent
    void resetWindMaxApparent() {
        windSpeedMaxApp = windSpeedApparent;
    }
    
    // Reset du max réel
    void resetWindMaxTrue() {
        windSpeedMaxTrue = windSpeedTrue;
    }
    
    // Calcul de la direction du vent au sol (Ground Wind Direction)
    // GWD = COG + TWA
    float getGroundWindDirection() {
        if (!hasCOG || !hasWindTrue) {
            return NAN;
        }
        
        float gwd = cog + windAngleTrue;
        
        // Normaliser 0-359°
        while (gwd >= 360.0f) gwd -= 360.0f;
        while (gwd < 0.0f) gwd += 360.0f;
        
        return gwd;
    }
    
    // Affichage formaté sur Serial
    void displaySerial() {
        float gwd = getGroundWindDirection();
        
        // Format heure avec "?" si invalide
        String timeDisplay = utcTime;
        if (!timeIsValid && hasTime) {
            timeDisplay += "?";
        }
        
        Serial.printf("HDG:%03.0f / COG:%03.0f / SOG:%04.1f / AWS:%04.1f / TWS:%04.1f / AWA:%03.0f / TWA:%03.0f / GWD:%03.0f / MAXW:%04.1f / SOC:%03d / AMP:%+05.1f / Prof:%05.1f / %s\n",
            hasHeading ? heading : 0.0f,
            hasCOG ? cog : 0.0f,
            hasSOG ? sog : 0.0f,
            hasWindApparent ? windSpeedApparent : 0.0f,
            hasWindTrue ? windSpeedTrue : 0.0f,
            hasWindApparent ? windAngleApparent : 0.0f,
            hasWindTrue ? windAngleTrue : 0.0f,
            isnan(gwd) ? 0.0f : gwd,
            windSpeedMaxApp > windSpeedMaxTrue ? windSpeedMaxApp : windSpeedMaxTrue,
            hasBattery ? batterySOC : 0,
            hasBattery ? batteryCurrent : 0.0f,
            hasDepth ? depth : 0.0f,
            timeDisplay.c_str()
        );
    }
};

// Structure d'un message ASCII N2K parsé
struct N2kMessage {
    char type;                // 'A'
    String timestamp;         // hhmmss.ddd
    uint8_t source;           // SS
    uint8_t destination;      // DD
    uint8_t priority;         // P
    uint32_t pgn;             // PPPPP (hexa)
    uint8_t data[223];        // Données (max 223 bytes)
    int dataLength;           // Nombre de bytes
    
    N2kMessage() {
        type = 0;
        timestamp = "";
        source = 0;
        destination = 0;
        priority = 0;
        pgn = 0;
        dataLength = 0;
        memset(data, 0, sizeof(data));
    }
};

#endif // NMEA_DATA_H
