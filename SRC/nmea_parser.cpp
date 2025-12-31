/*
 * nmea_parser.cpp - Version 1.13
 * 
 * Parser ASCII N2K et décodeurs PGN
 * Ajout PGN 129033: Time & Date (sync RTC ESP32 avec GPS pur, offset ignoré)
 */

#include "nmea_parser.h"
#include <sys/time.h>
#include <time.h>

// ========================================
// DÉCLARATIONS EXTERNES (depuis .ino)
// ========================================
extern void setRTCTime(int year, int month, int day, int hour, int minute, int second);
extern bool rtc_synced;

// ===== HELPERS PARSING HEXA =====

uint8_t NmeaParser::hexCharToNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

uint8_t NmeaParser::hexToByte(const char* hex) {
    return (hexCharToNibble(hex[0]) << 4) | hexCharToNibble(hex[1]);
}

uint32_t NmeaParser::hexToUint32(const char* hex, int len) {
    uint32_t result = 0;
    for (int i = 0; i < len; i++) {
        result = (result << 4) | hexCharToNibble(hex[i]);
    }
    return result;
}

// ===== PARSER ASCII N2K =====

bool NmeaParser::parseN2K_ASCII(const char* line, N2kMessage* msg) {
    // Format: A<hhmmss.ddd> <SS><DD><P> <PPPPP> <b0b1b2...> <CR><LF>
    
    // Vérifier type 'A'
    if (line[0] != 'A') return false;
    
    int pos = 1;
    
    // Parser timestamp (jusqu'à l'espace)
    while (line[pos] != ' ' && line[pos] != '\0') {
        msg->timestamp += line[pos];
        pos++;
    }
    if (line[pos] != ' ') return false;
    pos++; // Skip space
    
    // Skip les espaces multiples
    while (line[pos] == ' ') pos++;
    
    // Parser SSDDT (5 chars hexa)
    if (strlen(&line[pos]) < 5) return false;
    msg->source = hexToByte(&line[pos]);
    msg->destination = hexToByte(&line[pos+2]);
    msg->priority = hexCharToNibble(line[pos+4]);
    pos += 5;
    
    // Skip space
    while (line[pos] == ' ') pos++;
    
    // Parser PGN (5 chars hexa)
    if (strlen(&line[pos]) < 5) return false;
    msg->pgn = hexToUint32(&line[pos], 5);
    pos += 5;
    
    // Skip space
    while (line[pos] == ' ') pos++;
    
    // Parser les données (pairs de hexa jusqu'à CR/LF/fin)
    msg->dataLength = 0;
    while (line[pos] != '\r' && line[pos] != '\n' && line[pos] != '\0') {
        if (isxdigit(line[pos]) && isxdigit(line[pos+1])) {
            if (msg->dataLength < 223) {
                msg->data[msg->dataLength++] = hexToByte(&line[pos]);
            }
            pos += 2;
        } else {
            pos++; // Skip non-hex (spaces)
        }
    }
    
    msg->type = 'A';
    return true;
}

// ===== DECODEURS PGN =====

bool NmeaParser::decodeWindData(const N2kMessage* msg, NmeaData* data) {
    // PGN 130306 - Wind Data (6 bytes minimum)
    if (msg->dataLength < 6) return false;
    
    const uint8_t* d = msg->data;
    
    // Wind Speed (bytes 1-2) en cm/s
    uint16_t speedRaw = NmeaConversions::getUint16(&d[1]);
    float speedKts = NmeaConversions::cmsToKnots(speedRaw);
    
    // Wind Angle (bytes 3-4) en 0.0001 radians
    uint16_t angleRaw = NmeaConversions::getUint16(&d[3]);
    float angleDeg = NmeaConversions::radiansToDegreesUint16(angleRaw);
    
    // Reference (byte 5)
    uint8_t reference = d[5] & 0x07;
    
    if (reference == WIND_REF_APPARENT) {
        data->windSpeedApparent = speedKts;
        data->windAngleApparent = angleDeg;
        data->hasWindApparent = true;
    } else if (reference == WIND_REF_TRUE_NORTH || reference == WIND_REF_TRUE_BOAT) {
        data->windSpeedTrue = speedKts;
        data->windAngleTrue = angleDeg;
        data->hasWindTrue = true;
    }
    
    data->lastUpdate = millis();
    return true;
}

bool NmeaParser::decodeBatteryStatus(const N2kMessage* msg, NmeaData* data) {
    // PGN 127508 - Battery Status (8 bytes minimum)
    if (msg->dataLength < 8) return false;
    
    const uint8_t* d = msg->data;
    
    // Voltage (bytes 1-2) en 0.01V
    uint16_t voltageRaw = NmeaConversions::getUint16(&d[1]);
    data->batteryVoltage = NmeaConversions::rawToVoltage(voltageRaw);
    
    // Current (bytes 3-4) en 0.1A, signé
    int16_t currentRaw = NmeaConversions::getInt16(&d[3]);
    data->batteryCurrent = NmeaConversions::rawToCurrent(currentRaw);
    
    data->hasBattery = true;
    data->lastUpdate = millis();
    return true;
}

bool NmeaParser::decodeDCStatus(const N2kMessage* msg, NmeaData* data) {
    // PGN 127506 - DC Detailed Status (5 bytes minimum)
    if (msg->dataLength < 5) return false;
    
    const uint8_t* d = msg->data;
    
    // State of Charge (byte 3) en %
    data->batterySOC = NmeaConversions::rawToSOC(d[3]);
    
    data->hasBattery = true;
    data->lastUpdate = millis();
    return true;
}

bool NmeaParser::decodeHeading(const N2kMessage* msg, NmeaData* data) {
    // PGN 127250 - Vessel Heading (3 bytes minimum)
    if (msg->dataLength < 3) return false;
    
    const uint8_t* d = msg->data;
    
    // Heading (bytes 1-2) en 0.0001 radians
    uint16_t headingRaw = NmeaConversions::getUint16(&d[1]);
    data->heading = NmeaConversions::radiansToDegreesUint16(headingRaw);
    
    data->hasHeading = true;
    data->lastUpdate = millis();
    return true;
}

bool NmeaParser::decodeCOGSOG(const N2kMessage* msg, NmeaData* data) {
    // PGN 129026 - COG & SOG (6 bytes minimum)
    if (msg->dataLength < 6) return false;
    
    const uint8_t* d = msg->data;
    
    // COG (bytes 2-3) en 0.0001 radians
    uint16_t cogRaw = NmeaConversions::getUint16(&d[2]);
    data->cog = NmeaConversions::radiansToDegreesUint16(cogRaw);
    data->hasCOG = true;
    
    // SOG (bytes 4-5) en 0.01 m/s
    uint16_t sogRaw = NmeaConversions::getUint16(&d[4]);
    data->sog = NmeaConversions::msToKnots(sogRaw);
    data->hasSOG = true;
    
    data->lastUpdate = millis();
    return true;
}

bool NmeaParser::decodeDepth(const N2kMessage* msg, NmeaData* data) {
    // PGN 128267 - Water Depth (5 bytes minimum)
    if (msg->dataLength < 5) return false;
    
    const uint8_t* d = msg->data;
    
    // Depth (bytes 1-4) en 0.01 m (32-bit)
    uint32_t depthRaw = NmeaConversions::getUint32(&d[1]);
    data->depth = NmeaConversions::rawToDepthMeters(depthRaw);
    
    data->hasDepth = true;
    data->lastUpdate = millis();
    return true;
}

bool NmeaParser::decodeSystemTime(const N2kMessage* msg, NmeaData* data) {
    // PGN 126992 - System Time (8 bytes minimum)
    if (msg->dataLength < 8) return false;
    
    const uint8_t* d = msg->data;
    
    // Days since epoch (bytes 2-3)
    uint16_t daysSinceEpoch = NmeaConversions::getUint16(&d[2]);
    
    // Time of day (bytes 4-7) en 0.0001 secondes
    uint32_t timeOfDay = NmeaConversions::getUint32(&d[4]);
    
    // Vérifier valeur invalide
    if (timeOfDay == INVALID_UINT32) {
        return false;
    }
    
    // Convertir en secondes
    float seconds = timeOfDay * 0.0001f;
    
    int hours = (int)(seconds / 3600) % 24;
    int minutes = (int)((seconds - hours * 3600) / 60);
    int secs = (int)(seconds - hours * 3600 - minutes * 60);
    
    // Validation des valeurs
    if (hours > 23 || minutes > 59 || secs > 59) {
        // Valeurs aberrantes: garder la dernière heure valide
        data->timeIsValid = false;
        data->utcTime = data->lastValidTime;
        data->hasTime = true;
        return false;
    }
    
    // Heure valide
    char timeStr[16];
    sprintf(timeStr, "%02d:%02d:%02d", hours, minutes, secs);
    data->utcTime = String(timeStr);
    data->lastValidTime = data->utcTime;
    data->timeIsValid = true;
    data->hasTime = true;
    data->lastUpdate = millis();
    return true;
}

// ========================================
// NOUVEAU v1.13: PGN 129033 - Time & Date
// Sync RTC avec GPS PUR (offset ignoré)
// ========================================
bool NmeaParser::decodeTimeDate(const N2kMessage* msg, NmeaData* data) {
    // PGN 129033 - Time & Date (8 bytes minimum)
    if (msg->dataLength < 8) {
        Serial.println("[PGN 129033] ERREUR: Longueur données insuffisante");
        return false;
    }
    
    const uint8_t* d = msg->data;
    
    // ========================================
    // 1. EXTRACTION DONNÉES GPS
    // ========================================
    
    // Jours depuis epoch (bytes 0-1) - 1970-01-01
    uint16_t daysSinceEpoch = NmeaConversions::getUint16(&d[0]);
    if (daysSinceEpoch == INVALID_UINT16) {
        Serial.println("[PGN 129033] ERREUR: Date invalide");
        return false;
    }
    
    // Temps du jour (bytes 2-5) en 0.0001 secondes depuis minuit
    uint32_t timeOfDay = NmeaConversions::getUint32(&d[2]);
    if (timeOfDay == INVALID_UINT32) {
        Serial.println("[PGN 129033] ERREUR: Temps invalide");
        return false;
    }
    
    // Offset local (bytes 6-7) en minutes (signé)
    // V1.13: LU mais IGNORÉ (toujours zéro dans ton cas)
    int16_t offsetMinutes = NmeaConversions::getInt16(&d[6]);
    
    // ========================================
    // 2. CONVERSION TEMPS DU JOUR
    // ========================================
    
    float seconds = timeOfDay * 0.0001f;
    int hours = (int)(seconds / 3600);
    int minutes = (int)((seconds - hours * 3600) / 60);
    int secs = (int)(seconds - hours * 3600 - minutes * 60);
    
    // Validation
    if (hours > 23 || minutes > 59 || secs > 59) {
        Serial.printf("[PGN 129033] ERREUR: Heure invalide: %02d:%02d:%02d\n", 
                      hours, minutes, secs);
        return false;
    }
    
    // ========================================
    // 3. CONVERSION DATE (epoch → année/mois/jour)
    // ========================================
    
    time_t epoch = (time_t)daysSinceEpoch * 86400L;
    struct tm timeinfo;
    gmtime_r(&epoch, &timeinfo);
    
    int year = timeinfo.tm_year + 1900;
    int month = timeinfo.tm_mon + 1;
    int day = timeinfo.tm_mday;
    
    // ========================================
    // 4. SYNC RTC ESP32 avec GPS PUR (UTC)
    // V1.13: PAS d'application d'offset
    // ========================================
    
    setRTCTime(year, month, day, hours, minutes, secs);
    rtc_synced = true;
    
    // ========================================
    // 5. TRACE FRÉQUENCE RÉCEPTION (v1.13)
    // ========================================
    
    static unsigned long last_pgn_time = 0;
    unsigned long now = millis();
    unsigned long delta = (last_pgn_time == 0) ? 0 : (now - last_pgn_time);
    
    Serial.printf("[PGN 129033] Reçu à T+%lu ms (delta: %lu ms) → %04d-%02d-%02d %02d:%02d:%02d UTC (offset network: %+d min ignoré)\n",
                  now, delta, year, month, day, hours, minutes, secs, offsetMinutes);
    
    last_pgn_time = now;
    
    data->lastUpdate = millis();
    return true;
}

// ===== DISPATCHER =====

void NmeaParser::processMessage(const N2kMessage* msg, NmeaData* data) {
    switch (msg->pgn) {
        case PGN_WIND_DATA:
            decodeWindData(msg, data);
            data->updateWindMax();
            break;
            
        case PGN_BATTERY_STATUS:
            decodeBatteryStatus(msg, data);
            break;
            
        case PGN_DC_DETAILED_STATUS:
            decodeDCStatus(msg, data);
            break;
            
        case PGN_VESSEL_HEADING:
            decodeHeading(msg, data);
            break;
            
        case PGN_COG_SOG:
            decodeCOGSOG(msg, data);
            break;
            
        case PGN_WATER_DEPTH:
            decodeDepth(msg, data);
            break;
            
        case PGN_SYSTEM_TIME:
            decodeSystemTime(msg, data);
            break;
            
        case PGN_TIME_DATE:  // NOUVEAU v1.13
            decodeTimeDate(msg, data);
            break;
            
        default:
            // PGN non géré
            break;
    }
}
