**[File Name]: INFO-AI.md**
**[File Content Begin]**

# 🚤 ALBA III - NMEA WiFi Repeater

Current Version: **v1.15** (January 2025)

>>> AI generated document <<<
---

## 📋 Table of Contents

1.  [Project Objective](#-project-objective)
2.  [Operating Principle](#-operating-principle)
3.  [Required Hardware](#-required-hardware)
4.  [Displayed Variables - Usage](#-displayed-variables---usage)
5.  [Installation and Configuration](#-installation-and-configuration)
6.  [Touch Features](#-touch-features)
7.  [Constraints and Versions](#-constraints-and-versions)
8.  [Customization](#-customization)
9.  [Usage Tips](#-usage-tips)
10. [Troubleshooting](#-troubleshooting)

---

## 🎯 Project Objective

To have a view of the boat's main parameters in one of the cabins without having to go to the chart table or the cockpit to read the instruments.

### General Vision

Create an **autonomous nautical instrument repeater** for the sailboat ALBA III, allowing real-time display of critical navigation data on a 5-inch touch screen placed in the cockpit or cabin.

### Advantages Over Commercial Solutions

-   ✅ **Reduced Cost**: ~150€ vs 500-800€ for a Raymarine/Garmin repeater
-   ✅ **Customizable**: Interface tailored to your specific needs
-   ✅ **Evolvable**: Easy addition of new features
-   ✅ **Autonomous**: Works independently of the chartplotter
-   ✅ **WiFi**: No additional NMEA2000 cabling required

### Typical Use Cases

1.  **Rest During Night Watch**: Display HDG, AWS, AWA, SOC, SOG...
2.  **At Anchor**: Monitor depth, GWD, SOC...
3.  **Energy Management**: Battery SOC, charge/discharge current
4.  **Navigation**: COG, SOG, TWA, GWD
5.  **Backup**: Emergency repeater if main chartplotter fails

---

## ⚙️ Operating Principle

### System Architecture

```
┌─────────────────┐
│  Instruments    │
│  (wind, GPS,    │  NMEA2000   ┌──────────────┐
│   sounder, VHF, ├────────────►│  Actisense   │
│   battery...)   │   (CAN bus) │   W2K-1      │
└─────────────────┘             │  Gateway     │
                                └──────┬───────┘
                                       │ WiFi
                                       │ (UDP ASCII N2K)
                                       ▼
                              ┌─────────────────┐
                              │  ESP32-S3       │
                              │  Waveshare      │
                              │  Touch LCD 5B   │
                              │  (ALBA III)     │
                              └─────────────────┘
```

### Data Flow

1.  **Sensors** → Send data on NMEA2000 bus (250 kbps)
2.  **W2K-1** → Converts NMEA2000 to ASCII N2K and broadcasts via WiFi (UDP port 60002).
    The data output has been filtered in the W2K-1 to send only necessary PGNs and not overload the UDP stream.
3.  **ESP32** → Receives UDP packets, parses PGNs, extracts data
4.  **Calculations** → TWS/TWA calculated from AWS/AWA/SOG/COG
5.  **Display** → LVGL refreshes the screen (~60 FPS)

### Received Data Format

**Example ASCII N2K frame**:
```
A123456.789 FF01 1FD02 0100C8005A03
│           │    │     └─ Data (hex)
│           │    └─────── PGN (130306 = Wind)
│           └──────────── Source/Dest/Prio
└──────────────────────── Timestamp
```

---

## 🖥️ Required Hardware

### Essential Components

| Component | Reference | Estimated Price | Link |
|-----------|-----------|----------------|------|
| **ESP32-S3 Screen** | Waveshare ESP32-S3 Touch LCD 5B | ~80€ | [Waveshare](https://www.waveshare.com/esp32-s3-touch-lcd-5.htm) |
| **WiFi Gateway** | Actisense W2K-1 | ~200€ | [Actisense](https://actisense.com/products/nmea-2000/w2k-1/) |
| **3D Printed Enclosure** | IP65 optional | ~30€ | - |

### Waveshare ESP32-S3 Touch LCD 5B Specifications

-   **Screen**: 5" 1024x600 IPS LCD
-   **Touch**: GT911 capacitive multi-touch
-   **CPU**: ESP32-S3 Dual-core 240 MHz
-   **RAM**: 512 KB SRAM + 8 MB PSRAM
-   **Flash**: 16 MB
-   **WiFi**: 802.11 b/g/n 2.4 GHz
-   **GPIO**: Many available (future sensors)

---

## 📊 Displayed Variables - Usage

### 🧭 **HDG - Heading (Compass Heading)**

**Display**: Rotating boat + numeric value (e.g., `245°`)
**Source**: PGN 127250 (Vessel Heading)

**Usage**:
-   **Rest During Night Watch**: Follow intended course
-   **At Anchor**: Monitor swing

---

### 🌊 **AWS - Apparent Wind Speed**

**Display**: Large yellow value at bottom of WIND compass (e.g., `12.4 kts`)
**Source**: PGN 130306 (Wind Data)

**Usage**:
-   **Rest During Night Watch**: Assess wind strength outside
-   **At Anchor**: Detect if anchor line needs to be lengthened.

**MAXW**: Maximum gust value reached since last reset (touch WIND frame)

---

### 🧭 **AWA - Apparent Wind Angle**

**Display**: Rotating yellow triangle + value (e.g., `45°`)
**Source**: PGN 130306 (Wind Data)

**Color Code**:
-   **Red arc** (300-360°): Port
-   **Green arc** (0-60°): Starboard

---

### 🌬️ **TWS / TWA - True Wind Speed/Angle**

**Display**: TWA in white below reticle (e.g., `38°`)
**Source**: Calculated from AWS, AWA, SOG, COG

**Calculation**:
```
TWS² = SOG² + AWS² - 2×SOG×AWS×cos(AWA)
TWA = arccos((SOG² + TWS² - AWS²) / (2×TWS×SOG))
```

**Usage**:
-   **Tactics**: True wind for VMG and polars
-   **Weather**: Wind direction/strength independent of boat speed

---

### 🧭 **GWD - Ground Wind Direction**

**Display**: Green value (e.g., `245°`)
**Source**: Calculated `GWD = HDG + TWA`

**Usage**:
-   **Tactics**: Know exact wind origin
-   **Weather**: Compare GWD with weather forecast
-   **At Anchor**: Check if wind shifts (front passing)

**Example**: HDG=200°, TWA=45° → GWD=245° (wind from SW)

---

### 🚤 **/ HDG - /Heading**

**Display**: Heading on a compass rose, materialized by a rotating boat silhouette
**Source**: PGN 127250 (Vessel Heading)

---

### 🚤 **/ SOG - /Speed Over Ground**

**Display**: SOG=bottom right green (e.g., `5.8 kts`)
**Source**: PGN 129026 (COG & SOG, Rapid Update)

---

### ⚡ **SOC - State of Charge**

**Display**: Green percentage (e.g., `87%`)
**Source**: PGN 127506 (DC Detailed Status)

**Usage**:
-   **At Anchor**: Monitor autonomy
-   **Navigation**: Verify solar panel/alternator charging
-   **Night**: Avoid deep discharge

**Tip**: Reliable SOC only with calibrated BMV-712 or SmartShunt

---

### 🔋 **AMP - Battery Current**

**Display**: Amperes with color (green=charge, orange=discharge)
**Source**: PGN 127508 (Battery Status)

**Usage**:
-   **Charge**: Positive AMP → panels/alternator charging
-   **Discharge**: Negative AMP → consumption (fridge, instruments...)
-   **Balance**: AMP near 0 = production ≈ consumption

**Examples**:
-   `+15.5 A`: Alternator charging (engine ON)
-   `-3.2 A`: Moderate consumption (instruments) - Display in orange
-   `-8.7 A`: High discharge (autopilot + instruments + fridge) - Display in orange

---

### 🌊 **Depth**

**Display**: Green meters (e.g., `12.5 m`)
**Source**: PGN 128267 (Water Depth)

**Usage**:
-   **At Anchor**: Verify anchor holding (stable depth = good)

**Tip**: Depth = under keel (add draft for total depth)

---

### 🕐 **UTC Time + Offset**

**Display**: `14:35:22` + `UTC +1`
**Source**: PGN 129033 (Time & Date GPS)

**Synchronization**:
1.  GPS sends pure UTC time
2.  ESP32 RTC synchronized with GPS
3.  Manual offset applied for local time
4.  Offset saved in NVS (survives reboots)

**Usage**:
-   **Navigation**: Exact time
-   **Logbook**: Timestamping
-   **Watch**: Crew change at precise time

**Indicator**: Red `?` if RTC not yet synced with GPS

---

## 🔧 Installation and Configuration

### Software Prerequisites

1.  **Arduino IDE 2.3.x** (tested with 2.3.6)
2.  **ESP32 Libraries** (via Board Manager)
3.  **LVGL 8.4.0** (⚠️ **NOT 9.x!**)
4.  **ESP_Panel** (Waveshare) from Espressif
5.  **ESP_IOExpander_Library** from Espressif

### Step-by-Step Installation

#### 1. Install Arduino IDE

-   Download: [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)
-   Install version **2.3.6** minimum

#### 2. Add ESP32 Board Manager

**File → Preferences → Additional Boards Manager URLs**:
```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

**Tools → Board → Boards Manager**:
-   Search "ESP32"
-   Install **"esp32 by Espressif Systems"** version **3.0.x**

#### 3. Install LVGL 8.4.0

**⚠️ IMPORTANT**: Version **8.4.0** mandatory (9.x incompatible)

**Sketch → Include Library → Manage Libraries**:
-   Search "lvgl"
-   Install **"lvgl" version 8.4.0** exactly

#### 4. Install Waveshare Libraries

**Library Manager**:
-   `ESP_Panel` (latest version)
-   `ESP_IOExpander_Library` (latest version)

Important: At this stage, it is highly recommended to test the display on the Waveshare with one of the LVGL examples.
#### 5. Download the ALBA III Project

Copy all source files to a `repeat_wifi/` folder:

```
repeat_wifi/
├── repeat_wifi.ino          ← Main file
├── config.h                 ← WiFi/SSID configuration
├── nmea_*.h / .cpp          ← NMEA parsing
├── wifi_manager.h / .cpp    ← WiFi management
├── display_*.h / .cpp       ← LVGL display
├── [images .c]              ← Pictograms
└── [other .h]               ← Configurations
```

#### 6. Configure the Project

**Open `config.h` and modify**:

```cpp
// WiFi W2K-1 (MANDATORY) Examples:
#define WIFI_SSID "w2k-300356"        // Format: w2k-<serial number>
#define WIFI_PASSWORD "Albaxxxx"   // W2K-1 password (8 characters)
#define UDP_PORT 60002                // UDP Port (Based on W2K-1 data server settings)
```

**Open `repeat_wifi.ino` and modify**:

```cpp
// Boat personalization (lines 60-61)
const char* BOAT_NAME = "ALBA III";      // Your boat name
const char* FIRMWARE_VERSION = "v1.15";  // Current version
```

#### 7. Configure Arduino IDE

**Tools**:
-   **Board**: ESP32S3 Dev Module
-   **USB CDC On Boot**: Enabled
-   **USB DFU On Boot**: Disabled
-   **Flash Size**: 16MB (128Mb)
-   **Partition Scheme**: 16M Flash (3M APP/9.9M FATFS)
-   **PSRAM**: OPI PSRAM
-   **Upload Speed**: 921600

#### 8. Compile and Upload

1.  Connect ESP32 via USB-C
2.  **Sketch → Verify/Compile** (Ctrl+R)
3.  **Sketch → Upload** (Ctrl+U)
4.  Open **Serial Monitor** (115200 baud)

---

## 🖐️ Touch Features

### Touch Zones Overview

```
┌─────────────────────────────────────────────┐
│         WIND          │   HDG   │   CLOCK   │
│   [Reset Max AWS]     │ [Sleep] │  [+] [-]  │
│                       │         │           │
│                       │         │           │
├───────────────────────┴─────────┼───────────┤
│                                 │   DEPTH   │
│                                 │           │
├─────────┬─────────┬─────────────┴───────────┤
│   GWD   │   SOC   │      AMP                │
└─────────┴─────────┴─────────────────────────┘
```

---

### 1️⃣ **WIND Frame - Reset Max Wind**

**Action**: Touch anywhere inside the WIND frame (left)

**Effect**:
-   Reset `AWS Max` to current value
-   Visual feedback: Value flashes briefly
-   Debounce: 500 ms between resets

**Usage**:
-   After squall: Reset to monitor next peak
-   Start of navigation: Reset daily counter
-   Sail change: New wind reference

**Serial Log**:
```
[Touch] WIND zone detected
[Touch] AWS Max reset performed
[Touch] Old value: 18.5 kts
[Touch] New value: 12.4 kts
```

---

### 2️⃣ **HDG Frame - Sleep Mode**

**Action**: Touch anywhere inside the HDG frame (center)

**Effect**:
-   Enter sleep mode (90% black overlay)
-   Message `Sleeping - Touch to restore`
-   Reduces screen consumption

**Wake up**: Touch anywhere on the screen

**Usage**:
-   Night: Save battery when instruments unused
-   Glare: Reduce screen light at night in the cabin
-   At Anchor: Deactivate screen but keep system active

**Debounce**:
-   Enter sleep: 300 ms
-   Exit sleep: Immediate

**⚠️ Known Limitation**: Touch may be frozen during WiFi reconnection (20s)

---

### 3️⃣ **+/- Buttons - UTC Offset**

**Position**: In CLOCK frame, above and below clock pictogram: Sets time zone offset, not time itself.

**`+` Button**: Increment UTC offset (+1 hour)
**`-` Button**: Decrement UTC offset (-1 hour)

**Limits**:
-   Minimum: UTC-12
-   Maximum: UTC+14

**Save**:
-   Timer 5 seconds after last change
-   NVS write only if value different
-   Survives reboots

**Visual feedback**:
-   Button flash (light gray 100ms)
-   Immediate display time update
-   Update label `UTC +X`

**Usage**:
-   Local time zone
-   Daylight saving time
-   International navigation

**Enlarged touch zones**: +10px in all directions (easier touch)

**Serial Log**:
```
[Touch] CLOCK+ zone detected
[Touch] Increment offset → offset = +2
[Touch] New UTC offset: +2 hours
[PREFS] Offset saved: +2
```

---

### 🎯 **Precise Touch Zones**

| Zone | X (pixels) | Y (pixels) | Width | Height |
|------|------------|------------|---------|---------|
| WIND | 10-328 | 59-449 | 318 | 390 |
| HDG | 353-671 | 59-449 | 318 | 390 |
| CLOCK+ | 696-766 (+10) | 59-97 (+10) | 70 (+20) | 35 (+20) |
| CLOCK- | 696-766 (+10) | 175-210 (+10) | 70 (+20) | 35 (+20) |

**Note**: (+10) = enlarged margin for easier touch

---

## 📚 Constraints and Versions

### ⚠️ Critical Versions

| Library | MANDATORY Version | Reason |
|--------------|---------------------|--------|
| **LVGL** | **8.4.0** exactly | LVGL 9.x incompatible (API changed) |
| **ESP32** | 3.0.x recommended | Optimized Waveshare drivers |
| **Arduino IDE** | 2.3.x minimum | ESP32-S3 support |

### ❌ Frequent Errors

**1. LVGL 9.x Installed by Mistake**

```cpp
#if LV_VERSION_CHECK(9, 0, 0)
#error "ERROR: LVGL 9.x detected! This project requires LVGL 8.4.0"
#endif
```

**Solution**:
-   Uninstall LVGL 9.x
-   Install LVGL 8.4.0 via library manager

---

**2. White Screen at Boot**

**Possible Causes**:
-   PSRAM misconfigured → Check `PSRAM: OPI PSRAM`
-   LVGL misinitialized → Check serial logs

---

**3. WiFi Not Connecting**

```
[WiFi] ERROR: Connection timeout
```

**Check**:
-   Correct SSID in `config.h`
-   W2K-1 powered on and in AP mode
-   WiFi range (< 10m recommended)
-   Password exactly 8 characters

---

**4. Compiler Too Old**

```
error: 'std::function' has not been declared
```

**Solution**: Install Arduino IDE 2.3.6 minimum

---

### 🧪 Tested Compatibility

| Environment | Version | Status |
|---------------|---------|--------|
| Arduino IDE | 2.3.6 | ✅ OK |
| ESP32 Board | 3.0.7 | ✅ OK |
| LVGL | 8.4.0 | ✅ OK |
| ESP_Panel | 1.2.0 | ✅ OK |
| PlatformIO | Not tested | ❓ |

---

## 🎨 Customization

### File `config.h`

```cpp
// ===== WiFi W2K-1 =====
#define WIFI_SSID "w2k-300353"           // W2K-1 SSID
#define WIFI_PASSWORD "Albaalba.03"      // W2K-1 Password
#define UDP_PORT 60002                   // UDP Port (fixed)

// ===== Debug =====
#define DEBUG_NMEA 1                     // 1=logs, 0=silent
#define WIFI_TIMEOUT 20000               // WiFi timeout (ms)
#define RECONNECT_DELAY 5000             // Reconnection delay (ms)

// ===== Simulation Mode =====
#define SIMULATION_MODE 0                // 1=WiFi bypassed (touch test)
```

---

### File `repeat_wifi.ino`

```cpp
// Lines 60-61: Boat personalization
const char* BOAT_NAME = "ALBA III";      // Displayed name
const char* FIRMWARE_VERSION = "v1.15";  // Version
```

---

### Colors (file `display_data.cpp`)

```cpp
// Line 43: Screen background
lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);  // Black

// Line 52: Title banner
lv_obj_set_style_bg_color(title_banner, lv_color_hex(0x005FBE), 0);  // Victron Blue

// Frames (repeated 7×)
lv_obj_set_style_bg_color(wind_frame, lv_color_hex(0x1a1a1a), 0);  // Dark gray
```

**Victron Colors**:
-   Blue: `0x005FBE`
-   Green: `0x00FF00`
-   Orange: `0xFFA500`
-   Red: `0xFF0000`

---

### Fonts (file `lv_conf.h`)

```cpp
// Lines 536-550: Montserrat fonts enabled
#define LV_FONT_MONTSERRAT_14 1  // Small labels
#define LV_FONT_MONTSERRAT_16 1  // AWA/TWA titles
#define LV_FONT_MONTSERRAT_20 1  // WiFi status
#define LV_FONT_MONTSERRAT_24 1  // Cardinal letters
#define LV_FONT_MONTSERRAT_28 1  // Title banner
#define LV_FONT_MONTSERRAT_36 1  // AWS
#define LV_FONT_MONTSERRAT_38 1  // AWA/TWA values
#define LV_FONT_MONTSERRAT_48 1  // Main values
```

---

### Images (root folder)

**Required `.c` files**:
-   `triangle62x50TCA.c` - AWA wind vane
-   `picto_GWD2_80x80_TC.c` - GWD compass
-   `picto_voilier80x80TCA.c` - HDG sailboat
-   `picto_battery80x54TCA.c` - SOC battery
-   `picto_clock70x70TCA.c` - Clock
-   `picto_deepth66x70TCA.c` - Depth sounder
-   `picto_current70x68TCA.c` - Ammeter
-   `sil_boat180x54TCA.c` - HDG boat silhouette
-   `Splash_screen_vierge341x200TC.c` - Splash background

**Convert your images**:
1.  [LVGL Image Converter](https://lvgl.io/tools/imageconverter)
2.  Format: True Color Alpha (TCA)
3.  Output: C Array
4.  Add to project

---

### 📡 WiFi and Connectivity

**Optimal W2K-1 Range**:
-   **< 5m**: Excellent signal (RSSI > -50 dBm)
-   **5-10m**: Good signal (RSSI > -70 dBm)
-   **> 10m**: Weak signal (possible dropouts)

**ESP32 Placement**:
-   Direct line of sight to W2K-1 if possible
-   Avoid metal bulkheads
-   Similar height (not in bilge vs mast)

**Automatic Reconnection**:
-   Attempt every 5 seconds
-   Last values kept on screen
-   Red message `Connection lost...`

---

### 🔧 Maintenance

**Firmware Updates**:
1.  Backup config.h (SSID/password)
2.  Download new version
3.  Restore your config.h
4.  Compile and upload

**Screen Cleaning**:
-   Damp microfiber cloth
-   No aggressive products
-   Dry immediately

**Data Backup**:
-   UTC offset saved automatically (NVS)
-   Other settings in source code (git recommended)

---

## 🛠️ Troubleshooting

### Problem: White Screen on Boot

**Possible Causes**:
1.  PSRAM misconfigured
2.  Missing images
3.  LVGL misinitialized

**Solutions**:
```
1. Tools → PSRAM → OPI PSRAM
2. Check presence of .c image files
3. Serial Monitor → Look for [Display] ERROR
```

---

### Problem: WiFi Timeout

```
[WiFi] ERROR: Connection timeout
```

**Check**:
1.  W2K-1 powered on (green LED)
2.  Exact SSID in config.h (`w2k-XXXXXX`)
3.  Exact Password (8 characters)
4.  Range < 10m
5.  W2K-1 in AP mode (not client)

**Test**:
```cpp
// Enable simulation mode in config.h
#define SIMULATION_MODE 1
```

---

### Problem: Data `---` Everywhere

**Causes**:
1.  WiFi connected but no UDP data
2.  W2K-1 not on port 60002
3.  NMEA2000 bus off

**Checks**:
```
1. Serial Monitor: [STATS] UDP: 0 pkt/15s
2. W2K-1 config: Port = 60002, ASCII N2K
3. NMEA2000 instruments powered on
```

---

### Problem: Touch Not Working

**If during WiFi reconnection**:
-   **Normal** - Touch frozen for 20s during timeout
-   **Solution**: Wait for reconnection to finish

**If permanent**:
```
1. Serial Monitor: Look for [Touch] ERROR
2. Check touch detected at boot:
   [Touch] ✓ Touch detected and operational
3. Calibration may be necessary
```

---

### Problem: Incorrect Time

**Red `?` indicator**:
-   RTC not yet synchronized with GPS
-   Wait for PGN 129033 (Time & Date) reception

**Time offset**:
-   Check UTC offset (+/- buttons)
-   NVS save: Wait 5s after change

---

### Problem: Aberrant Values

**Examples**:
-   `AWS: 999.9 kts` → Wind sensor faulty
-   `Depth: 0.0 m` → Depth sounder disconnected
-   `SOC: 0%` → BMV-712 not configured

**Diagnosis**:
```
1. Serial Monitor: HDG:xxx / COG:xxx / ...
2. Identify 000 value = data not received
3. Check NMEA2000 sensor
```

---

### Useful Logs

**Enable debug**:
```cpp
// config.h
#define DEBUG_NMEA 1  // Display all frames
```

**Interpret logs**:
```
[STATS] UDP: 42 pkt/15s (2.8 Hz)     ← Reception OK
[STATS] UDP: 0 pkt/15s (0.0 Hz)      ← No data
[PERF] PGN 127506 parsed in 125 µs    ← Performance OK
```

---

## 📜 Version History

### v1.15 (January 2026) - Performance Debug
-   ✅ Runtime logs commented
-   ✅ UDP stats every 15s
-   ✅ Timers for slow PGN parsing
-   ✅ Removal of delay(5) in loop
-   ✅ Display speed optimization

### v1.14 (December 2025) - HDG instead of COG
-   ✅ GWD = HDG + TWA (corrected formula)
-   ✅ [N2K] logs commented
-   ✅ Improved stability

### v1.13 (December 2025) - GPS RTC
-   ✅ Sync ESP32 RTC with pure GPS (PGN 129033)
-   ✅ Time = RTC + manual offset
-   ✅ Network offset ignored

### v1.11 (December 2025) - Centralization
-   ✅ Centralized time display
-   ✅ Simplified loop (1 call instead of 2)
-   ✅ Clean architecture

### v1.10e (December 2025) - Touch+NVS
-   ✅ UTC offset save in NVS
-   ✅ Enlarged touch zones (+10px)
-   ✅ Non-blocking 5s timer

### v1.08 (December 2025) - Architecture
-   ✅ Centralized WiFi Manager
-   ✅ WiFi states with messages
-   ✅ Integrated debug

---

## 📄 License

**Free use** for personal and non-commercial projects.

**Attribution**: Mention "ALBA III - François-Xavier VAN THUAN" if redistributed.

**No Warranty**: Software provided "as is", use at your own risk.

---

## 🎉 Acknowledgments

-   **Actisense**: ASCII N2K documentation
-   **Waveshare**: ESP32-S3 technical support
-   **LVGL Team**: Excellent graphical framework

---

**Fair winds! ⛵**

---
**[File Content End]**