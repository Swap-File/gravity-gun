#include <Arduino.h>
#include "io.h"
#include "gauge.h"

#define TFT_BL 0
#define BUTTON_LED_PIN 23
#define BUTTON_PIN 22
#define THROTTLE_PIN A3
Gravity_Gun gun;

void io_init(void)
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUTTON_LED_PIN, OUTPUT);
    pinMode(THROTTLE_PIN, INPUT);
    pinMode(TFT_BL, OUTPUT);
    analogWrite(TFT_BL, 1023);
}

static float saved_level = 0.0;
static uint32_t last_time = millis();

void io_update(void)
{
    gun.button_input_last = gun.button_input;
    gun.button_input = !(digitalRead(BUTTON_PIN));

    gun.led_output = (exp(sin(millis() / 2000.0 * PI)) - 0.36787944) * 108.0;

    // if (!gun.button_input_last && gun.button_input)
    //{
    //     gun.battery_voltage2 = gun.battery_voltage1;
    //}

    volatile float read_voltage;
    read_voltage = 15 + (exp(sin(millis() / 2000.0 * PI)) - 0.36787944) - gun.throttle_input / 100;

    bool double_needle;

    if (!gun.button_input || gun.locked)
        double_needle = false;
    else
        double_needle = true;

    if (!double_needle)
    {
        gun.battery_voltage1 = read_voltage;
        if (millis() - last_time < 2000 && !gun.locked)
        {
            gun.battery_voltage2 = saved_level;
            double_needle = true; // stretch this
        }

        else
            gun.battery_voltage2 = read_voltage;
    }
    else
    {
        gun.battery_voltage1 = min(read_voltage, gun.battery_voltage1);
        saved_level = read_voltage;
        last_time = millis();
    }

    int raw = analogRead(THROTTLE_PIN);
    float reading = constrain(map(raw, 150, 950, 0, 100), 0, 100);
    gun.throttle_input = gun.throttle_input * .7 + reading * .3;

    // unlock

    static bool released = true;
    if ((gun.button_input || (int)gun.throttle_input > 0) && released)
        released = false;

    if ((!gun.button_input && (int)gun.throttle_input == 0) && !released)
    {
        gun.locked = false;

        released = true;
    }

    // handle top meter text
    if (gun.locked == false && gun.button_input != gun.button_input_last)
    {
        if (gun.button_input)
            gauge_throttle_text(" Active ");
        else
            gauge_throttle_text(" Ready ");
    }

    // handle bottom meter text
    char voltage[10];

    if (!double_needle)
    {
        if (((int)gun.battery_voltage1) >= 10)
            sprintf(voltage, " %.1f V", gun.battery_voltage1);
        else
            sprintf(voltage, "  %.1f V", gun.battery_voltage1);
    }
    else
    {
        float result = gun.battery_voltage1 - gun.battery_voltage2;
        if (result < 0)
            sprintf(voltage, "  %.1f V ", result);
        else
            sprintf(voltage, "   %.1f V ", result);
    }
    gauge_volts_text(voltage);

    // screensaver
    static uint32_t last_motion_time = 0;
    if (gun.button_input || (int)gun.throttle_input > 0)
    {
        last_motion_time = millis();

        gun.screensaver_on = false;
    }

    if (millis() - last_motion_time > 10000 && gun.screensaver_on == false)
    {
        gun.screensaver_on = true;
        gun.locked = true;
        gauge_throttle_text("Locked");
    }

    // throttle lock
    if (gun.locked)
        gun.throttle_output = 0;
    else
        gun.throttle_output = gun.throttle_input;

    // low battery alarm

    if (gun.battery_voltage1 < 12.0)
    {
        if (0x01 & (millis() >> 7))
        {
            analogWrite(TFT_BL, 1023);
            analogWrite(BUTTON_LED_PIN, 0);
        }
        else
        {
            analogWrite(TFT_BL, 0);
            analogWrite(BUTTON_LED_PIN, 1023);
        }
    }
    else
    {
        analogWrite(TFT_BL, 1023);

        if (gun.button_input)
            analogWrite(BUTTON_LED_PIN, 0);
        else
            analogWrite(BUTTON_LED_PIN, gun.led_output);
    }
}
