#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 8.5 Stunden in Sekunden
const unsigned long TIMER_SECONDS = 8UL * 3600UL + 30UL * 60UL;

unsigned long startMillis;
bool finished = false;

void showTime(unsigned long remaining) {
    unsigned long h = remaining / 3600;
    unsigned long m = (remaining % 3600) / 60;
    unsigned long s = remaining % 60;

    char buf[9];
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", h, m, s);

    display.clearDisplay();

    // Label
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(38, 4);
    display.print("Restzeit");

    // Zeit gross (textSize 2 = 12x16px pro Zeichen)
    display.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, th;
    display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &th);
    display.setCursor((SCREEN_WIDTH - (int16_t)w) / 2, 20);
    display.print(buf);

    // Fortschrittsbalken
    int barX = 4, barY = 50, barW = SCREEN_WIDTH - 8, barH = 10;
    display.drawRect(barX, barY, barW, barH, SSD1306_WHITE);
    int filled = (int)(((float)(TIMER_SECONDS - remaining) / TIMER_SECONDS) * (barW - 2));
    if (filled > 0) {
        display.fillRect(barX + 1, barY + 1, filled, barH - 2, SSD1306_WHITE);
    }

    display.display();
}

void showFinished() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    const char* msg = "FERTIG!";
    int16_t x1, y1;
    uint16_t w, th;
    display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &th);
    display.setCursor((SCREEN_WIDTH - (int16_t)w) / 2, (SCREEN_HEIGHT - (int16_t)th) / 2);
    display.print(msg);

    display.display();
}

void setup() {
    Serial.begin(115200);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println(F("SSD1306 nicht gefunden"));
        for (;;) delay(1000);
    }

    display.clearDisplay();
    display.display();

    startMillis = millis();
}

void loop() {
    if (finished) {
        showFinished();
        delay(1000);
        return;
    }

    unsigned long elapsed = (millis() - startMillis) / 1000UL;

    if (elapsed >= TIMER_SECONDS) {
        finished = true;
        return;
    }

    showTime(TIMER_SECONDS - elapsed);
    delay(500);
}
