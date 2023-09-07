#include <SPI.h>
#include <GC9A01A_t3n.h>
#include "meter.h"
#include "logo.h"
#include "gauge.h"

#define LOOP_PERIOD 10 // Display updates every 35 ms

#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 1
#define TFT_SCK 13
#define TFT_MOSI 11

GC9A01A_t3n tft = GC9A01A_t3n(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK);

MeterWidget throttle = MeterWidget(&tft);
MeterWidget volts = MeterWidget(&tft);

static uint32_t start_time = 0;

void gauge_throttle_text(const char *input)
{
    throttle.updateText(input);
}


void gauge_volts_text(const char *input)
{
    volts.updateText(input);
}


bool gauge_render_time_check(void)
{
    if (millis() - start_time > 4)
        return true;
    return false;
}

static inline void gauge_render_time_reset(void)
{
    start_time = millis();
}

void gauge_init(void)
{

    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(BLACK);

    tft.setTextColor(WHITE);

    tft.setTextDatum(MC_DATUM);

    //  do ascii art
    tft.fillScreen(BLACK);

    //            --Red--  -Org-   -Yell-  -Grn-
    throttle.setZones(75, 100, 50, 75, 25, 50, 0, 25);
    throttle.analogMeter(0, 16, 100, "Locked", "0", "25", "50", "75", "100");

    //            --Red--  -Org-   -Yell-  -Grn-
    volts.setZones(0, 25, 25, 50, 50, 75, 75, 100);
    volts.analogMeter(0, 126, 100, "Volts", "10", "12", "14", "16", "18");
}

float mapValue(float ip, float ipmin, float ipmax, float tomin, float tomax)
{
    return tomin + (((tomax - tomin) * (ip - ipmin)) / (ipmax - ipmin));
}

void gauge_update(const Gravity_Gun *gun)
{

    static bool screensaver = false;
    static uint32_t updateTime = 0;
    static bool alternate_update = false;

    if (millis() - updateTime >= LOOP_PERIOD)
    {

        updateTime = millis();

        gauge_render_time_reset();
        if (gun->screensaver_on)
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
                volts.analogMeterDrawReset();
                throttle.analogMeterDrawReset();
            }

            if (logo_blank(&tft))
            {
                if (throttle.analogMeterDraw() && volts.analogMeterDraw())
                {
                    if (alternate_update)
                    {
                        throttle.updateNeedle(gun->throttle_output, gun->throttle_output);

                    }
                    else
                    {
                        float voltage = mapValue(gun->battery_voltage1, (float)10.0, (float)18.0, (float)0.0, (float)100.0);
                        float voltage2 = mapValue(gun->battery_voltage2, (float)10.0, (float)18.0, (float)0.0, (float)100.0);
                        volts.updateNeedle(voltage, voltage2);
                    }
                    alternate_update = !alternate_update;
                }
            }
        }
        uint32_t time_spent = millis() - start_time;
        if (time_spent > 5)
        {
            Serial.print(time_spent);
            Serial.println(" Too Long!");
        }
    }
}
