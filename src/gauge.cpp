#include <SPI.h>
#include <GC9A01A_t3n.h>

#include "meter.h"
#include "logo.h"

#define BUTTON_LED_PIN 23
#define BUTTON_PIN 22

#define LOOP_PERIOD 10 // Display updates every 35 ms

#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 1
#define TFT_SCK 13
#define TFT_MOSI 11

GC9A01A_t3n tft = GC9A01A_t3n(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK);

MeterWidget throttle = MeterWidget(&tft);
MeterWidget volts = MeterWidget(&tft);

#define TFT_BL 0

void gauge_init(void)
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pinMode(TFT_BL, OUTPUT);
    analogWrite(TFT_BL, 1023);

    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(BLACK);

    tft.setTextColor(WHITE);

    tft.setTextDatum(MC_DATUM);

    //  do ascii art
    tft.fillScreen(BLACK);

    //            --Red--  -Org-   -Yell-  -Grn-
    throttle.setZones(75, 100, 50, 75, 25, 50, 0, 25);
    throttle.analogMeter(0, 16, 100, "Output", "0", "25", "50", "75", "100");

    //            --Red--  -Org-   -Yell-  -Grn-
    volts.setZones(0, 25, 25, 50, 50, 75, 75, 100);
    volts.analogMeter(0, 126, 10.0, "Volts", "10", "12", "14", "16", "18");
}

float mapValue(float ip, float ipmin, float ipmax, float tomin, float tomax)
{
    return tomin + (((tomax - tomin) * (ip - ipmin)) / (ipmax - ipmin));
}

bool screensaver = false;
void gauge_update(void)
{
    static int d = 0;
    static uint32_t updateTime = 0;
    static bool alternate_update = false;

    if (millis() - updateTime >= LOOP_PERIOD)
    {
        d += 4;
        if (d > 360)
            d = 0;

        // Create a Sine wave for testing, value is in range 0 - 100
        float value = 50.0 + 50.0 * sin((d + 0) * 0.0174532925);
        updateTime = millis();
        bool pin = digitalRead(BUTTON_PIN);
        if (pin)
            analogWrite(BUTTON_LED_PIN, mapValue(value, (float)0.0, (float)100.0, (float)0.0, (float)255.0));
        else
            analogWrite(BUTTON_LED_PIN, 255);

        if (pin)
        {
            if (screensaver == false)
            {
                screensaver = true;
                logo_reset();
            }
            if (logo_blank(&tft))
                logo_update(&tft);
        }
        else
        {
            if (screensaver == true)
            {
                screensaver = false;
                logo_reset();
            }
            if (logo_blank(&tft))
            {
                if (alternate_update)
                {
                    // Serial.print("I = "); Serial.print(current);
                    throttle.updateNeedle(value, value);
                }
                else
                {
                    float voltage;
                    voltage = mapValue(value, (float)0.0, (float)100.0, (float)0.0, (float)10.0);
                    float voltage2 = mapValue(value, (float)100.0, (float)0.0, (float)0.0, (float)10.0);
                    // Serial.print(", V = "); Serial.println(voltage);
                    volts.updateNeedle(voltage, voltage2);
                }
                alternate_update = !alternate_update;
            }
        }
    }
}