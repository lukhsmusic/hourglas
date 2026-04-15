#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_ADDRESS   0x3C

#define BTN_UP   D5   // GPIO14 — Minuten hoch
#define BTN_DOWN D6   // GPIO12 — Minuten runter

#define BOTH_HOLD_MS  500UL   // beide halten → Modus wechseln
#define REPEAT_DELAY  600UL   // ms bis Schnellwiederholung einsetzt
#define REPEAT_FAST    80UL   // ms zwischen schnellen Schritten
#define BLINK_INTERVAL 500UL  // Blinkintervall im Einstellmodus

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

enum State { RUNNING, SETTING };
State state = RUNNING;

unsigned long targetSeconds = 8UL * 3600UL + 30UL * 60UL;
unsigned long startMillis   = 0;
bool          finished      = false;

// Einstellmodus
unsigned long setSeconds = 0;

// Blinken
bool          blinkOn    = true;
unsigned long lastBlinkMs = 0;

// Display-Refresh im Laufmodus
unsigned long lastDisplayMs = 0;

// Beide-Knöpfe-Erkennung
unsigned long bothSince = 0;
bool          bothFired = false;

// Einzeltasten-Wiederholung
struct BtnState { bool held; unsigned long pressedAt; unsigned long lastRepeat; };
BtnState btnU = {}, btnD = {};

inline bool readUp()   { return digitalRead(BTN_UP)   == LOW; }
inline bool readDown() { return digitalRead(BTN_DOWN) == LOW; }

void drawTime(unsigned long secs, bool forSetting) {
    unsigned long h = secs / 3600;
    unsigned long m = (secs % 3600) / 60;
    unsigned long s = secs % 60;
    char buf[9];
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", h, m, s);

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // Label zentriert
    const char* label = forSetting ? "Einstellen" : "Restzeit";
    display.setTextSize(1);
    int16_t x1, y1; uint16_t lw, lh;
    display.getTextBounds(label, 0, 0, &x1, &y1, &lw, &lh);
    display.setCursor((SCREEN_WIDTH - (int16_t)lw) / 2, 4);
    display.print(label);

    // Zeit gross — im Einstellmodus blinkt sie
    if (!forSetting || blinkOn) {
        display.setTextSize(2);
        uint16_t tw, th;
        display.getTextBounds(buf, 0, 0, &x1, &y1, &tw, &th);
        display.setCursor((SCREEN_WIDTH - (int16_t)tw) / 2, 22);
        display.print(buf);
    }

    // Fortschrittsbalken nur im Laufmodus
    if (!forSetting) {
        int bx = 4, by = 50, bw = SCREEN_WIDTH - 8, bh = 10;
        display.drawRect(bx, by, bw, bh, SSD1306_WHITE);
        long elapsed = (long)targetSeconds - (long)secs;
        long filled  = (long)(((float)elapsed / (float)targetSeconds) * (bw - 2));
        if (filled > 0) display.fillRect(bx + 1, by + 1, (int)filled, bh - 2, SSD1306_WHITE);
    }

    display.display();
}

void drawFinished() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    const char* msg = "FERTIG!";
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - (int16_t)w) / 2, (SCREEN_HEIGHT - (int16_t)h) / 2);
    display.print(msg);
    display.display();
}

void applyStep(long delta) {
    long s = (long)setSeconds + delta;
    if (s < 60L)             s = 60L;
    if (s > 24L * 3600L)     s = 24L * 3600L;
    setSeconds = (unsigned long)s;
}

void setup() {
    Serial.begin(115200);
    pinMode(BTN_UP,   INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println(F("SSD1306 nicht gefunden"));
        for (;;) delay(1000);
    }
    display.clearDisplay();
    display.display();
    startMillis = millis();
}

void loop() {
    unsigned long now  = millis();
    bool up   = readUp();
    bool down = readDown();

    // ── Beide-Knöpfe-Erkennung (funktioniert in jedem Zustand) ─────────────
    if (up && down) {
        if (bothSince == 0) { bothSince = now; bothFired = false; }
        if (!bothFired && (now - bothSince >= BOTH_HOLD_MS)) {
            bothFired = true;
            btnU = {}; btnD = {};   // Einzeltasten-State zurücksetzen

            if (state == RUNNING) {
                // → Einstellmodus betreten, aktuelle Restzeit übernehmen
                if (finished) {
                    setSeconds = targetSeconds;
                } else {
                    unsigned long elapsed = (now - startMillis) / 1000UL;
                    setSeconds = (elapsed < targetSeconds) ? (targetSeconds - elapsed) : 60UL;
                }
                blinkOn     = true;
                lastBlinkMs = now;
                state       = SETTING;
            } else {
                // → Timer mit eingestellter Zeit (neu) starten
                targetSeconds = setSeconds;
                startMillis   = now;
                finished      = false;
                lastDisplayMs = 0;
                state         = RUNNING;
            }
        }
    } else {
        bothSince = 0;
        bothFired = false;
    }

    // ── Einstellmodus ───────────────────────────────────────────────────────
    if (state == SETTING) {
        // Blinken
        if (now - lastBlinkMs >= BLINK_INTERVAL) {
            blinkOn     = !blinkOn;
            lastBlinkMs = now;
        }

        bool onlyUp   = up   && !down;
        bool onlyDown = down && !up;

        // Knopf UP — Minuten erhöhen
        if (onlyUp) {
            if (!btnU.held) {
                btnU = { true, now, now };
                applyStep(+60);
            } else if (now - btnU.pressedAt  >= REPEAT_DELAY &&
                       now - btnU.lastRepeat  >= REPEAT_FAST) {
                btnU.lastRepeat = now;
                applyStep(+60);
            }
        } else {
            btnU.held = false;
        }

        // Knopf DOWN — Minuten verringern
        if (onlyDown) {
            if (!btnD.held) {
                btnD = { true, now, now };
                applyStep(-60);
            } else if (now - btnD.pressedAt  >= REPEAT_DELAY &&
                       now - btnD.lastRepeat  >= REPEAT_FAST) {
                btnD.lastRepeat = now;
                applyStep(-60);
            }
        } else {
            btnD.held = false;
        }

        drawTime(setSeconds, true);
        return;
    }

    // ── Laufmodus ───────────────────────────────────────────────────────────
    if (finished) {
        if (now - lastDisplayMs >= 500) {
            lastDisplayMs = now;
            drawFinished();
        }
        return;
    }

    unsigned long elapsed = (now - startMillis) / 1000UL;
    if (elapsed >= targetSeconds) {
        finished = true;
        drawFinished();
        lastDisplayMs = now;
        return;
    }

    if (now - lastDisplayMs >= 500) {
        lastDisplayMs = now;
        drawTime(targetSeconds - elapsed, false);
    }
}
