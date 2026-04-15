# Hourglas

Countdown-Timer für den **AZ-Delivery D1 mini** mit **0,96" I2C OLED Display (SSD1306)**.
Die Startzeit ist frei einstellbar (Standard: 8,5 Stunden).

## Display

**Laufmodus:**
```
┌────────────────────────┐
│       Restzeit         │
│                        │
│      08:29:42          │
│                        │
│ [███████░░░░░░░░░░░░░] │
└────────────────────────┘
```

**Einstellmodus** (Zeit blinkt):
```
┌────────────────────────┐
│      Einstellen        │
│                        │
│      08:30:00          │  ← blinkt
│                        │
│                        │
└────────────────────────┘
```

Nach Ablauf erscheint **FERTIG!** auf dem Display.

## Bedienung

Zwei Momentary-Taster (4-Pin) an **D5** (UP) und **D6** (DOWN):

| Aktion | Effekt |
|---|---|
| Beide Knöpfe 500 ms halten (Laufmodus) | Einstellmodus öffnen |
| Knopf UP / DOWN (Einstellmodus) | +1 / −1 Minute |
| Knopf gehalten (>600 ms) | schnelle Wiederholung alle 80 ms |
| Beide Knöpfe 500 ms halten (Einstellmodus) | Timer mit neuer Zeit starten |

## Verdrahtung

**OLED:**

| OLED | D1 mini |
|------|---------|
| VCC  | 3.3V    |
| GND  | GND     |
| SDA  | D2      |
| SCL  | D1      |

**Taster** (je eine Seite an GND, andere Seite an den jeweiligen Pin):

| Taster | D1 mini |
|--------|---------|
| UP     | D5      |
| DOWN   | D6      |

Die Pins nutzen den internen Pull-up — kein externer Widerstand nötig.

## Flashen

Projekt mit [PlatformIO](https://platformio.org/) öffnen und flashen:

```bash
pio run --target upload
```

## Abhängigkeiten

- Adafruit SSD1306
- Adafruit GFX Library

Werden von PlatformIO automatisch installiert.
