# Hourglas

8,5-Stunden-Countdown-Timer für den **AZ-Delivery D1 mini** mit **0,96" I2C OLED Display (SSD1306)**.

## Display

```
┌────────────────────────┐
│       Restzeit         │
│                        │
│      08:29:42          │
│                        │
│ [███████░░░░░░░░░░░░░] │
└────────────────────────┘
```

Nach Ablauf erscheint **FERTIG!** auf dem Display.

## Verdrahtung

| OLED | D1 mini |
|------|---------|
| VCC  | 3.3V    |
| GND  | GND     |
| SDA  | D2      |
| SCL  | D1      |

## Flashen

Projekt mit [PlatformIO](https://platformio.org/) öffnen und flashen:

```bash
pio run --target upload
```

## Abhängigkeiten

- Adafruit SSD1306
- Adafruit GFX Library

Werden von PlatformIO automatisch installiert.
