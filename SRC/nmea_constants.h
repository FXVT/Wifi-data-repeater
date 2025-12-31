#ifndef NMEA_CONSTANTS_H
#define NMEA_CONSTANTS_H

#include <Arduino.h>

// ===== CONVERSIONS D'UNITES =====
#define MS_TO_KNOTS     1.94384f    // m/s → nœuds
#define RAD_TO_DEG      57.2958f    // radians → degrés
#define KELVIN_TO_CELSIUS 273.15f   // Kelvin → Celsius

// ===== RESOLUTIONS NMEA2000 =====
#define WIND_SPEED_RES  0.01f       // cm/s → m/s
#define WIND_ANGLE_RES  0.0001f     // 0.0001 rad
#define VOLTAGE_RES     0.01f       // 0.01 V
#define CURRENT_RES     0.1f        // 0.1 A
#define DEPTH_RES       0.01f       // 0.01 m
#define SOG_RES         0.01f       // 0.01 m/s
#define HEADING_RES     0.0001f     // 0.0001 rad

// ===== VALEURS INVALIDES NMEA2000 =====
#define INVALID_UINT8   0xFF
#define INVALID_UINT16  0xFFFF
#define INVALID_UINT32  0xFFFFFFFF
#define INVALID_INT16   0x7FFF

// ===== PGN DEFINITIONS =====
#define PGN_SYSTEM_TIME         126992  // 0x1F010
#define PGN_VESSEL_HEADING      127250  // 0x1F112
#define PGN_DC_DETAILED_STATUS  127506  // 0x1F212
#define PGN_BATTERY_STATUS      127508  // 0x1F214
#define PGN_WATER_DEPTH         128267  // 0x1F50B
#define PGN_COG_SOG             129026  // 0x1F802
#define PGN_TIME_DATE           129033  // 0x1F809 - GNSS Position Data (v1.13)
#define PGN_WIND_DATA           130306  // 0x1FD02

// ===== WIND REFERENCE =====
#define WIND_REF_TRUE_NORTH     0
#define WIND_REF_MAGNETIC       1
#define WIND_REF_APPARENT       2
#define WIND_REF_TRUE_BOAT      3
#define WIND_REF_TRUE_WATER     4

// ===== LIMITES =====
#define MAX_WIND_SPEED  100.0f      // kts (limite réaliste)
#define MAX_SOG         50.0f       // kts
#define MIN_DEPTH       0.1f        // m
#define MAX_DEPTH       999.0f      // m

#endif // NMEA_CONSTANTS_H
