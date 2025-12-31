/*
 * nmea_parser.h - Version 1.13
 * 
 * Parser ASCII N2K et décodeurs PGN
 * Ajout PGN 129033: Time & Date (sync RTC ESP32 avec GPS pur)
 */

#ifndef NMEA_PARSER_H
#define NMEA_PARSER_H

#include <Arduino.h>
#include "nmea_data.h"
#include "nmea_constants.h"

// Classe pour les conversions d'unités
class NmeaConversions {
public:
    // Vitesse: cm/s → nœuds
    static float cmsToKnots(uint16_t cms) {
        if (cms == INVALID_UINT16) return NAN;
        return cms * WIND_SPEED_RES * MS_TO_KNOTS;
    }
    
    // Vitesse: m/s → nœuds
    static float msToKnots(uint16_t ms_raw) {
        if (ms_raw == INVALID_UINT16) return NAN;
        return ms_raw * SOG_RES * MS_TO_KNOTS;
    }
    
    // Angle: radians → degrés (0-359)
    static float radiansToDegreesUint16(uint16_t rad_raw) {
        if (rad_raw == INVALID_UINT16) return NAN;
        float deg = rad_raw * HEADING_RES * RAD_TO_DEG;
        // Normaliser 0-359°
        while (deg < 0) deg += 360.0f;
        while (deg >= 360.0f) deg -= 360.0f;
        return deg;
    }
    
    // Voltage: raw → Volts
    static float rawToVoltage(uint16_t raw) {
        if (raw == INVALID_UINT16) return NAN;
        return raw * VOLTAGE_RES;
    }
    
    // Courant: raw → Ampères (signé)
    static float rawToCurrent(int16_t raw) {
        if (raw == INVALID_INT16) return NAN;
        return raw * CURRENT_RES;
    }
    
    // Profondeur: raw → mètres
    static float rawToDepthMeters(uint32_t raw) {
        if (raw == INVALID_UINT32) return NAN;
        return raw * DEPTH_RES;
    }
    
    // SOC: raw → %
    static uint8_t rawToSOC(uint8_t raw) {
        if (raw == INVALID_UINT8) return 0;
        return raw;
    }
    
    // Helpers pour extraire valeurs little-endian
    static uint16_t getUint16(const uint8_t* data) {
        return ((uint16_t)data[1] << 8) | data[0];
    }
    
    static int16_t getInt16(const uint8_t* data) {
        return (int16_t)(((uint16_t)data[1] << 8) | data[0]);
    }
    
    static uint32_t getUint32(const uint8_t* data) {
        return ((uint32_t)data[3] << 24) | 
               ((uint32_t)data[2] << 16) | 
               ((uint32_t)data[1] << 8) | 
               data[0];
    }
};

// Classe pour parser les messages ASCII N2K
class NmeaParser {
public:
    // Parser une ligne ASCII N2K
    static bool parseN2K_ASCII(const char* line, N2kMessage* msg);
    
    // Décoder les PGN
    static bool decodeWindData(const N2kMessage* msg, NmeaData* data);
    static bool decodeBatteryStatus(const N2kMessage* msg, NmeaData* data);
    static bool decodeDCStatus(const N2kMessage* msg, NmeaData* data);
    static bool decodeHeading(const N2kMessage* msg, NmeaData* data);
    static bool decodeCOGSOG(const N2kMessage* msg, NmeaData* data);
    static bool decodeDepth(const N2kMessage* msg, NmeaData* data);
    static bool decodeSystemTime(const N2kMessage* msg, NmeaData* data);
    
    // NOUVEAU v1.13: PGN 129033 - Time & Date (sync RTC avec GPS pur)
    static bool decodeTimeDate(const N2kMessage* msg, NmeaData* data);
    
    // Dispatcher selon le PGN
    static void processMessage(const N2kMessage* msg, NmeaData* data);

private:
    // Helpers pour parsing hexa
    static uint8_t hexCharToNibble(char c);
    static uint8_t hexToByte(const char* hex);
    static uint32_t hexToUint32(const char* hex, int len);
};

#endif // NMEA_PARSER_H
