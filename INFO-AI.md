# 🚤 ALBA III - Répéteur NMEA WiFi

Version actuelle : **v1.15** (Janvier 2025)

>>> AI generated document <<<
---

## 📋 Table des matières

1. [Objectif du projet](#-objectif-du-projet)
2. [Principe de fonctionnement](#-principe-de-fonctionnement)
3. [Matériel requis](#-matériel-requis)
4. [Variables affichées - Usage](#-variables-affichées---usage)
5. [Installation et configuration](#-installation-et-configuration)
6. [Fonctionnalités tactiles](#-fonctionnalités-tactiles)
7. [Contraintes et versions](#-contraintes-et-versions)
8. [Personnalisation](#-personnalisation)
9. [Conseils d'utilisation](#-conseils-dutilisation)
10. [Dépannage](#-dépannage)

---

## 🎯 Objectif du projet

Avoir dans une des cabines une vue sur les paramètres principaux du bateau sans avoir à se déplacer à la table à carte ou dans le cockpit pour lire les instruments
### Vision générale

Créer un **répéteur d'instruments nautiques autonome** pour le voilier ALBA III, permettant d'afficher en temps réel les données de navigation critiques sur un écran tactile 5 pouces placé au cockpit ou dans la cabine.

### Avantages par rapport aux solutions commerciales

- ✅ **Coût réduit** : ~150€ vs 500-800€ pour un répéteur Raymarine/Garmin
- ✅ **Personnalisable** : Interface adaptée à vos besoins spécifiques
- ✅ **Evolutif** : Ajout facile de nouvelles fonctionnalités
- ✅ **Autonome** : Fonctionne indépendamment du traceur
- ✅ **WiFi** : Pas de câblage NMEA2000 supplémentaire

### Cas d'usage typiques

1. **Repos pendant Quarts de nuit** : Affichage HDG, AWS, AWA, SOC, SOG...
2. **Au mouillage** : Surveillance profondeur, GWD, SOC...
3. **Gestion énergie** : SOC batterie, courant charge/décharge
4. **Navigation** : COG, SOG, TWA, GWD
5. **Secours** : Répéteur de secours si le chartplotter principal tombe en panne

---

## ⚙️ Principe de fonctionnement

### Architecture du système

```
┌─────────────────┐
│  Instruments    │
│  (vent, GPS,    │  NMEA2000   ┌──────────────┐
│   sonde, VHF,   ├────────────►│  Actisense   │
│   batterie...)  │   (CAN bus) │   W2K-1      │
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

### Flux de données

1. **Capteurs** → Envoient données sur bus NMEA2000 (250 kbps)
2. **W2K-1** → Convertit NMEA2000 en ASCII N2K et diffuse par WiFi (UDP port 60002) . 
La sortie des données a été filtré dans e W2K-1 pour n'envoyer que les PGN nécessaires et ne pas engorger le flux UDP.
3. **ESP32** → Reçoit paquets UDP, parse les PGN, extrait les données
4. **Calculs** → TWS/TWA calculés à partir de AWS/AWA/SOG/COG
5. **Affichage** → LVGL rafraîchit l'écran (~60 FPS)

### Format de données reçues

**Exemple de trame ASCII N2K** :
```
A123456.789 FF01 1FD02 0100C8005A03
│           │    │     └─ Données (hexa)
│           │    └─────── PGN (130306 = Wind)
│           └──────────── Source/Dest/Prio
└──────────────────────── Timestamp
```

---

## 🖥️ Matériel requis

### Composants essentiels

| Composant | Référence | Prix indicatif | Lien |
|-----------|-----------|----------------|------|
| **Écran ESP32-S3** | Waveshare ESP32-S3 Touch LCD 5B | ~80€ | [Waveshare](https://www.waveshare.com/esp32-s3-touch-lcd-5.htm) |
| **Gateway WiFi** | Actisense W2K-1 | ~200€ | [Actisense](https://actisense.com/products/nmea-2000/w2k-1/) |
| **Boîtier Imp 3D** | IP65 optionnel | ~30€ | - |

### Caractéristiques Waveshare ESP32-S3 Touch LCD 5B

- **Écran** : 5" 1024x600 IPS LCD
- **Tactile** : GT911 capacitif multi-touch
- **CPU** : ESP32-S3 Dual-core 240 MHz
- **RAM** : 512 KB SRAM + 8 MB PSRAM
- **Flash** : 16 MB
- **WiFi** : 802.11 b/g/n 2.4 GHz
- **GPIO** : Nombreux disponibles (futurs capteurs)

---

## 📊 Variables affichées - Usage

### 🧭 **HDG - Heading (Cap compas)**

**Affichage** : Bateau tournant + valeur numérique (ex: `245°`)  
**Source** : PGN 127250 (Vessel Heading)

**Usage** :
- **Repos pendant Quart de nuit** : suivre la route prévue
- **Mouillage** : Surveiller évitage 

---

### 🌊 **AWS - Apparent Wind Speed (Vitesse vent apparent)**

**Affichage** : Grande valeur jaune en bas du compas WIND (ex: `12.4 kts`)  
**Source** : PGN 130306 (Wind Data)

**Usage** :
- **Repos durant Quart de nuit** : Évaluer force du vent à l'extérieur
- **Au Mouillage** : Détecter s'il faut allonge le mouillage ou pas.

**MAXW** : Valeur maximale atteinte en rafale depuis dernier reset (touch cadre WIND)

---

### 🧭 **AWA - Apparent Wind Angle (Angle vent apparent)**

**Affichage** : Triangle jaune tournant + valeur (ex: `45°`)  
**Source** : PGN 130306 (Wind Data)

**Code couleur** :
- **Arc rouge** (300-360°) : Babord
- **Arc vert** (0-60°) : Tribord

---

### 🌬️ **TWS / TWA - True Wind Speed/Angle (Vent réel)**

**Affichage** : TWA en blanc sous réticule (ex: `38°`)  
**Source** : Calculé depuis AWS, AWA, SOG, COG

**Calcul** :
```
TWS² = SOG² + AWS² - 2×SOG×AWS×cos(AWA)
TWA = arccos((SOG² + TWS² - AWS²) / (2×TWS×SOG))
```

**Usage** :
- **Tactique** : Vent réel pour VMG et polaires
- **Météo** : Direction/force vent réel indépendant de la vitesse bateau

---

### 🧭 **GWD - Ground Wind Direction (Direction vent au sol)**

**Affichage** : Valeur verte (ex: `245°`)  
**Source** : Calculé `GWD = HDG + TWA`

**Usage** :
- **Tactique** : Connaître provenance exacte du vent
- **Météo** : Comparer GWD avec prévisions météo
- **Mouillage** : Vérifier si vent tourne (front qui passe)

**Exemple** : HDG=200°, TWA=45° → GWD=245° (vent de SW)

---

### 🚤 **/ HDG - /Heading Cap **

**Affichage** :  Cap dans une rosace  matérialisé par une silhouette de bateau tournant
**Source** : PGN 127250 (Vessel Heading)
---

### 🚤 **/ SOG - /Speed Over Ground**

**Affichage** :  SOG=bas droite vert (ex: `5.8 kts`)  
**Source** : PGN 129026 (COG & SOG, Rapid Update)
---

### ⚡ **SOC - State of Charge (Charge batterie)**

**Affichage** : Pourcentage vert (ex: `87%`)  
**Source** : PGN 127506 (DC Detailed Status)

**Usage** :
- **Au mouillage** : Surveiller autonomie 
- **Navigation** : Vérifier recharge panneaux/alternateur
- **Nuit** : Éviter décharge profonde 

**Conseil** : SOC fiable uniquement avec BMV-712 ou SmartShunt calibré

---

### 🔋 **AMP - Courant batterie**

**Affichage** : Ampères avec couleur (vert=charge, orange=décharge)  
**Source** : PGN 127508 (Battery Status)

**Usage** :
- **Charge** : AMP positif → panneaux/alternateur charge
- **Décharge** : AMP négatif → consommation (frigo, instruments...)
- **Équilibre** : AMP proche 0 = production ≈ consommation

**Exemples** :
- `+15.5 A` : Alternateur charge (moteur ON)
- `-3.2 A` : Consommation modérée (instruments ) Affichage en orange
- `-8.7 A` : Forte décharge ( pilote + instruments + frigo) Affichage en orange

---

### 🌊 **Profondeur**

**Affichage** : Mètres vert (ex: `12.5 m`)  
**Source** : PGN 128267 (Water Depth)

**Usage** :
- **Mouillage** : Vérifier tenue ancre (profondeur stable = bon)


**Conseil** : Profondeur = sous quille (ajouter tirant d'eau pour profondeur totale)

---

### 🕐 **Heure UTC + Offset**

**Affichage** : `14:35:22` + `UTC +1`  
**Source** : PGN 129033 (Time & Date GPS)

**Synchronisation** :
1. GPS envoie heure UTC pure
2. RTC ESP32 synchronisée avec GPS
3. Offset manuel appliqué pour heure locale
4. Offset sauvegardé en NVS (survit aux redémarrages)

**Usage** :
- **Navigation** : Heure exacte 
- **Logbook** : Horodatage 
- **Quarts** : Changement d'équipe à heure précise

**Indicateur** : `?` rouge si RTC pas encore synchro GPS

---

## 🔧 Installation et configuration

### Prérequis logiciels

1. **Arduino IDE 2.3.x** (testé avec 2.3.6)
2. **Bibliothèques ESP32** (via Board Manager)
3. **LVGL 8.4.0** (⚠️ **PAS 9.x !**)
4. **ESP_Panel** (Waveshare) de chez Expressif
5. **ESP_IOExpander_Library** de chez Expressif

### Installation pas à pas

#### 1. Installer Arduino IDE

- Télécharger : [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)
- Installer version **2.3.6** minimum

#### 2. Ajouter ESP32 Board Manager

**Fichier → Préférences → URLs de gestionnaire de cartes supplémentaires** :
```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

**Outils → Type de carte → Gestionnaire de cartes** :
- Chercher "ESP32"
- Installer **"esp32 by Espressif Systems"** version **3.0.x**

#### 3. Installer LVGL 8.4.0

**⚠️ IMPORTANT** : Version **8.4.0** obligatoire (9.x incompatible)

**Croquis → Inclure une bibliothèque → Gérer les bibliothèques** :
- Chercher "lvgl"
- Installer **"lvgl" version 8.4.0** exactement

#### 4. Installer bibliothèques Waveshare

**Gestionnaire de bibliothèques** :
- `ESP_Panel` (dernière version)
- `ESP_IOExpander_Library` (dernière version)

Important: à ce stade il est fortement conseillé de tester l'affichage sur le Waveshare avec un des exemples de LVGL.
#### 5. Télécharger le projet ALBA III

Copier tous les fichiers sources dans un dossier `repeat_wifi/` :

```
repeat_wifi/
├── repeat_wifi.ino          ← Fichier principal
├── config.h                 ← Configuration WiFi/SSID
├── nmea_*.h / .cpp          ← Parsing NMEA
├── wifi_manager.h / .cpp    ← Gestion WiFi
├── display_*.h / .cpp       ← Affichage LVGL
├── [images .c]              ← Pictogrammes
└── [autres .h]              ← Configurations
```

#### 6. Configurer le projet

**Ouvrir `config.h` et modifier** :

```cpp
// WiFi W2K-1 (OBLIGATOIRE) Exemples:
#define WIFI_SSID "w2k-300356"        // Format: w2k-<numéro série>
#define WIFI_PASSWORD "Albaxxxx"   // Mot de passe W2K-1 (8 caractères)
#define UDP_PORT 60002                // Port UDP (En fonction du paramérage fait sur le serveur de donné du W2K-1)
```

**Ouvrir `repeat_wifi.ino` et modifier** :

```cpp
// Personnalisation bateau (ligne 60-61)
const char* BOAT_NAME = "ALBA III";      // Votre nom de bateau
const char* FIRMWARE_VERSION = "v1.15";  // Version actuelle
```

#### 7. Configurer Arduino IDE

**Outils** :
- **Type de carte** : ESP32S3 Dev Module
- **USB CDC On Boot** : Enabled
- **USB DFU On Boot** : Disabled
- **Flash Size** : 16MB (128Mb)
- **Partition Scheme** : 16M Flash (3M APP/9.9M FATFS)
- **PSRAM** : OPI PSRAM
- **Upload Speed** : 921600

![splash](photos/IMG_20260102_165440s.jpg)

#### 8. Compiler et téléverser

1. Connecter ESP32 via USB-C
2. **Croquis → Vérifier/Compiler** (Ctrl+R)
3. **Croquis → Téléverser** (Ctrl+U)
4. Ouvrir **Moniteur Série** (115200 bauds)

---

## 🖐️ Fonctionnalités tactiles

### Vue d'ensemble des zones tactiles

```
┌─────────────────────────────────────────────┐
│         WIND          │   HDG   │   CLOCK   │
│   [Reset Max AWS]     │ [Veille]│  [+] [-]  │
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

### 1️⃣ **Cadre WIND - Reset vent maximum**

**Action** : Toucher n'importe où dans le cadre WIND (gauche)

**Effet** :
- Reset `AWS Max` à la valeur actuelle
- Feedback visuel : Valeur clignote brièvement
- Débounce : 500 ms entre 2 resets

**Usage** :
- Après grain : Reset pour surveiller prochain pic
- Début de navigation : Reset compteur journalier
- Changement de voilure : Nouveau référentiel vent

**Log série** :
```
[Touch] Zone WIND détectée
[Touch] Reset AWS Max effectué
[Touch] Ancienne valeur: 18.5 kts
[Touch] Nouvelle valeur: 12.4 kts
```

---

### 2️⃣ **Cadre HDG - Mode veille**

**Action** : Toucher n'importe où dans le cadre HDG (centre)

**Effet** :
- Passage en mode veille (overlay noir 90%)
- Message `En veille - Toucher pour rétablir`
- Réduit consommation écran

**Sortie de veille** : Toucher n'importe où sur l'écran

**Usage** :
- Nuit : Économiser batterie quand instruments inutilisés
- Éblouissement : Réduire lumière écran la nuit dans la cabine
- Mouillage : Désactiver écran mais laisser système actif

**Débounce** :
- Entrée veille : 300 ms
- Sortie veille : Immédiat

**⚠️ Limitation connue** : Touch peut être gelé pendant reconnexion WiFi (20s)

---

### 3️⃣ **Boutons +/- - Offset UTC**

**Position** : Dans cadre CLOCK, au-dessus et en-dessous du pictogramme horloge: Réglage du décalage horaire, et pas réglage de l'heure.

**Bouton `+`** : Incrémenter offset UTC (+1 heure)  
**Bouton `-`** : Décrémenter offset UTC (-1 heure)

**Limites** :
- Minimum : UTC-12
- Maximum : UTC+14

**Sauvegarde** :
- Timer 5 secondes après dernier changement
- Écriture NVS uniquement si valeur différente
- Survit aux redémarrages

**Feedback visuel** :
- Flash bouton (gris clair 100ms)
- MAJ immédiate affichage heure
- MAJ label `UTC +X`

**Usage** :
- Fuseau horaire local
- Heure d'été/hiver
- Navigation internationale

**Zones tactiles élargies** : +10px dans toutes directions (facilite touch)

**Log série** :
```
[Touch] Zone CLOCK+ détectée
[Touch] Incrément offset → décalage = +2
[Touch] Nouvel offset UTC: +2 heures
[PREFS] Offset sauvegardé: +2
```

---

### 🎯 **Zones tactiles précises**

| Zone | X (pixels) | Y (pixels) | Largeur | Hauteur |
|------|------------|------------|---------|---------|
| WIND | 10-328 | 59-449 | 318 | 390 |
| HDG | 353-671 | 59-449 | 318 | 390 |
| CLOCK+ | 696-766 (+10) | 59-97 (+10) | 70 (+20) | 35 (+20) |
| CLOCK- | 696-766 (+10) | 175-210 (+10) | 70 (+20) | 35 (+20) |

**Note** : (+10) = marge élargie pour faciliter touch

---

## 📚 Contraintes et versions

### ⚠️ Versions critiques

| Bibliothèque | Version OBLIGATOIRE | Raison |
|--------------|---------------------|--------|
| **LVGL** | **8.4.0** exactement | LVGL 9.x incompatible (API changée) |
| **ESP32** | 3.0.x recommandé | Drivers Waveshare optimisés |
| **Arduino IDE** | 2.3.x minimum | Support ESP32-S3 |

### ❌ Erreurs fréquentes

**1. LVGL 9.x installé par erreur**

```cpp
#if LV_VERSION_CHECK(9, 0, 0)
#error "ERREUR: LVGL 9.x détecté ! Ce projet nécessite LVGL 8.4.0"
#endif
```

**Solution** :
- Désinstaller LVGL 9.x
- Installer LVGL 8.4.0 via gestionnaire bibliothèques

---

**2. Écran blanc au démarrage**

**Causes possibles** :
- PSRAM mal configuré → Vérifier `PSRAM: OPI PSRAM`
- LVGL mal initialisé → Vérifier logs série

---

**3. WiFi ne se connecte pas**

```
[WiFi] ERREUR: Timeout de connexion
```

**Vérifier** :
- SSID correct dans `config.h`
- W2K-1 allumé et en mode AP
- Portée WiFi (< 10m recommandé)
- Password 8 caractères exact

---

**4. Compilateur trop ancien**

```
error: 'std::function' has not been declared
```

**Solution** : Installer Arduino IDE 2.3.6 minimum

---

### 🧪 Compatibilité testée

| Environnement | Version | Statut |
|---------------|---------|--------|
| Arduino IDE | 2.3.6 | ✅ OK |
| ESP32 Board | 3.0.7 | ✅ OK |
| LVGL | 8.4.0 | ✅ OK |
| ESP_Panel | 1.2.0 | ✅ OK |
| PlatformIO | Non testé | ❓ |

---

## 🎨 Personnalisation

### Fichier `config.h`

```cpp
// ===== WiFi W2K-1 =====
#define WIFI_SSID "w2k-300353"           // SSID W2K-1
#define WIFI_PASSWORD "Albaalba.03"      // Password W2K-1
#define UDP_PORT 60002                   // Port UDP (fixe)

// ===== Debug =====
#define DEBUG_NMEA 1                     // 1=logs, 0=silencieux
#define WIFI_TIMEOUT 20000               // Timeout WiFi (ms)
#define RECONNECT_DELAY 5000             // Délai reconnexion (ms)

// ===== Mode Simulation =====
#define SIMULATION_MODE 0                // 1=WiFi shunté (test touch)
```

---

### Fichier `repeat_wifi.ino`

```cpp
// Ligne 60-61: Personnalisation bateau
const char* BOAT_NAME = "ALBA III";      // Nom affiché
const char* FIRMWARE_VERSION = "v1.15";  // Version
```

---

### Couleurs (fichier `display_data.cpp`)

```cpp
// Ligne 43: Fond écran
lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);  // Noir

// Ligne 52: Bandeau titre
lv_obj_set_style_bg_color(title_banner, lv_color_hex(0x005FBE), 0);  // Bleu Victron

// Cadres (répété 7×)
lv_obj_set_style_bg_color(wind_frame, lv_color_hex(0x1a1a1a), 0);  // Gris foncé
```

**Couleurs Victron** :
- Bleu : `0x005FBE`
- Vert : `0x00FF00`
- Orange : `0xFFA500`
- Rouge : `0xFF0000`

---

### Polices (fichier `lv_conf.h`)

```cpp
// Ligne 536-550: Polices Montserrat activées
#define LV_FONT_MONTSERRAT_14 1  // Labels petits
#define LV_FONT_MONTSERRAT_16 1  // AWA/TWA titres
#define LV_FONT_MONTSERRAT_20 1  // WiFi status
#define LV_FONT_MONTSERRAT_24 1  // Lettres cardinales
#define LV_FONT_MONTSERRAT_28 1  // Bandeau titre
#define LV_FONT_MONTSERRAT_36 1  // AWS
#define LV_FONT_MONTSERRAT_38 1  // AWA/TWA valeurs
#define LV_FONT_MONTSERRAT_48 1  // Valeurs principales
```

---

### Images (dossier racine)

**Fichiers `.c` requis** :
- `triangle62x50TCA.c` - Girouette AWA
- `picto_GWD2_80x80_TC.c` - Compas GWD
- `picto_voilier80x80TCA.c` - Voilier HDG
- `picto_battery80x54TCA.c` - Batterie SOC
- `picto_clock70x70TCA.c` - Horloge
- `picto_deepth66x70TCA.c` - Sonde profondeur
- `picto_current70x68TCA.c` - Ampèremètre
- `sil_boat180x54TCA.c` - Silhouette bateau HDG
- `Splash_screen_vierge341x200TC.c` - Fond splash

**Convertir vos images** :
1. [LVGL Image Converter](https://lvgl.io/tools/imageconverter)
2. Format : True Color Alpha (TCA)
3. Output : C Array
4. Ajouter au projet

---


### 📡 WiFi et connectivité

**Portée optimale W2K-1** :
- **< 5m** : Excellent signal (RSSI > -50 dBm)
- **5-10m** : Bon signal (RSSI > -70 dBm)
- **> 10m** : Signal faible (coupures possibles)

**Placement ESP32** :
- Ligne de vue directe vers W2K-1 si possible
- Éviter cloisons métalliques
- Hauteur similaire (pas au fond de cale vs mât)

**Reconnexion automatique** :
- Tentative toutes les 5 secondes
- Dernieres valeurs conservées à l'écran
- Message rouge `Connexion perdue...`

---

### 🔧 Maintenance

**Mises à jour firmware** :
1. Sauvegarder config.h (SSID/password)
2. Télécharger nouvelle version
3. Restaurer votre config.h
4. Compiler et téléverser

**Nettoyage écran** :
- Chiffon microfibre humide
- Pas de produits agressifs
- Sécher immédiatement

**Sauvegarde données** :
- Offset UTC sauvegardé automatiquement (NVS)
- Autres réglages dans code source (git recommandé)

---

## 🛠️ Dépannage

### Problème : Écran blanc au boot

**Causes possibles** :
1. PSRAM mal configuré
2. Images manquantes
3. LVGL mal initialisé

**Solutions** :
```
1. Outils → PSRAM → OPI PSRAM
2. Vérifier présence fichiers .c images
3. Moniteur série → Chercher [Display] ERREUR
```

---

### Problème : WiFi timeout

```
[WiFi] ERREUR: Timeout de connexion
```

**Vérifier** :
1. W2K-1 allumé (LED verte)
2. SSID exact dans config.h (`w2k-XXXXXX`)
3. Password exact (8 caractères)
4. Portée < 10m
5. W2K-1 en mode AP (pas client)

**Test** :
```cpp
// Activer mode simulation dans config.h
#define SIMULATION_MODE 1
```

---

### Problème : Données `---` partout

**Causes** :
1. WiFi connecté mais pas de données UDP
2. W2K-1 pas sur port 60002
3. NMEA2000 bus éteint

**Vérifications** :
```
1. Moniteur série: [STATS] UDP: 0 pkt/15s
2. W2K-1 config: Port = 60002, ASCII N2K
3. Instruments NMEA2000 allumés
```

---

### Problème : Touch ne fonctionne pas

**Si pendant reconnexion WiFi** :
- **Normal** - Touch gelé 20s pendant timeout
- **Solution** : Attendre fin reconnexion

**Si permanent** :
```
1. Moniteur série: Chercher [Touch] ERREUR
2. Vérifier touch détecté au boot:
   [Touch] ✓ Touch détecté et opérationnel
3. Calibration peut être nécessaire
```

---

### Problème : Heure incorrecte

**Indicateur `?` rouge** :
- RTC pas encore synchronisée avec GPS
- Attendre réception PGN 129033 (Time & Date)

**Heure décalée** :
- Vérifier offset UTC (boutons +/-)
- Sauvegarde NVS : Attendre 5s après changement

---

### Problème : Valeurs aberrantes

**Exemples** :
- `AWS: 999.9 kts` → Capteur vent HS
- `Depth: 0.0 m` → Sonde profondeur déconnectée
- `SOC: 0%` → BMV-712 non configuré

**Diagnostic** :
```
1. Moniteur série: HDG:xxx / COG:xxx / ...
2. Identifier valeur 000 = donnée non reçue
3. Vérifier capteur NMEA2000
```

---

### Logs utiles

**Activer debug** :
```cpp
// config.h
#define DEBUG_NMEA 1  // Afficher toutes trames
```

**Interpréter logs** :
```
[STATS] UDP: 42 pkt/15s (2.8 Hz)     ← Réception OK
[STATS] UDP: 0 pkt/15s (0.0 Hz)      ← Pas de données
[PERF] PGN 127506 parsé en 125 µs    ← Performance OK
```

---



## 📜 Historique des versions

### v1.15 (Janvier 2026) - Performance Debug
- ✅ Logs runtime commentés
- ✅ Stats UDP toutes les 15s
- ✅ Timers parsing PGN lents
- ✅ Suppression delay(5) loop
- ✅ Optimisation vitesse affichage

### v1.14 (Décembre 2025) - HDG au lieu de COG
- ✅ GWD = HDG + TWA (formule corrigée)
- ✅ Logs [N2K] commentés
- ✅ Stabilité améliorée

### v1.13 (Décembre 2025) - RTC GPS
- ✅ Sync RTC ESP32 avec GPS pur (PGN 129033)
- ✅ Heure = RTC + offset manuel
- ✅ Offset network ignoré

### v1.11 (Décembre 2025) - Centralisation
- ✅ Affichage heure centralisé
- ✅ Loop simplifiée (1 appel au lieu de 2)
- ✅ Architecture propre

### v1.10e (Décembre 2025) - Touch+NVS
- ✅ Sauvegarde offset UTC en NVS
- ✅ Zones tactiles élargies (+10px)
- ✅ Timer 5s non-bloquant

### v1.08 (Décembre 2025) - Architecture
- ✅ WiFi Manager centralisé
- ✅ États WiFi avec messages
- ✅ Debug intégré

---

## 📄 Licence

**Utilisation libre** pour projets personnels et non commerciaux.

**Attribution** : Mentionner "ALBA III - François-Xavier VAN THUAN" si redistribution.

**Aucune garantie** : Logiciel fourni "tel quel", utilisez à vos risques.

---

## 🎉 Remerciements

- **Actisense** : Documentation ASCII N2K
- **Waveshare** : Support technique ESP32-S3
- **LVGL Team** : Framework graphique excellent

---

**Bon vent ! ⛵**

---


