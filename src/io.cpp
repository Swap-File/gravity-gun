#include <Arduino.h>
#include <ADC.h>

#include "io.h"
#include "gauge.h"
#include "PWMServo.h"

#define TFT_BL 0
#define BUTTON_LED_PIN 23
#define BUTTON_PIN 22
#define THROTTLE_PIN A3
#define BUTTON_PIN 22
#define SERVO_PIN 3
#define BATTERY_VOLTAGE_PIN A5

#define TIME_TO_HOLD_THE_DIP 5000
#define TIME_BEFORE_SCREENSAVER 20000

Gravity_Gun gun;

PWMServo myservo; // create servo object to control a servo

ADC *adc = new ADC();

int max_power = 106;
bool maximized = false;

void io_init(void)
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUTTON_LED_PIN, OUTPUT);
    pinMode(TFT_BL, OUTPUT);
    pinMode(BUTTON_LED_PIN, OUTPUT);

    // ADC0 setup
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    adc->adc0->setAveraging(32);
    adc->adc0->setResolution(16);
    adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_LOW_SPEED);
    adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED);
    adc->adc0->startContinuous(BATTERY_VOLTAGE_PIN);

    pinMode(THROTTLE_PIN, INPUT);
    adc->adc1->setAveraging(32);
    adc->adc1->setResolution(16);
    adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_LOW_SPEED);
    adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED);
    adc->adc1->startContinuous(THROTTLE_PIN);

    analogWrite(TFT_BL, 1023);

    myservo.attach(SERVO_PIN);
    myservo.write(0);
    delay(100);
    myservo.write(90);

    if (!(digitalRead(BUTTON_PIN)))
    {
        max_power = 128;
        maximized = true;
    }
}

static float saved_level = 0.0;
static uint32_t last_time = millis();

void io_update(void)
{
    gun.button_input_last = gun.button_input;
    gun.button_input = !(digitalRead(BUTTON_PIN));

    gun.led_output = (exp(sin(millis() / 2000.0 * PI)) - 0.36787944) * 108.0;

     if (!gun.button_input_last && gun.button_input)
    {
         gun.battery_voltage2 = gun.battery_voltage1;
    }

    static float raw_voltage = 0;
    float read_voltage;
    raw_voltage = raw_voltage * .96 + .04 * (uint16_t)adc->adc0->analogReadContinuous(); // .04
    // absolute calibration at 12v -> 42900    3575
    // then corrected slope at 16v -> 55790    3486
    read_voltage = (raw_voltage / 3575) + (((raw_voltage / 3575) - 12) / 10);

    bool double_needle = false;

    if (!gun.button_input || gun.locked)
        double_needle = false;
    else
        double_needle = true;

    bool extending = false;

    if (!double_needle)
    {
        gun.battery_voltage1 = read_voltage;
        if (millis() - last_time < TIME_TO_HOLD_THE_DIP && !gun.locked)
        {
            gun.battery_voltage2 = saved_level;
            double_needle = true; // stretch this
            extending = true;
        }
        else
            gun.battery_voltage2 = read_voltage;
    }
    else
    {
        gun.battery_voltage1 = min(read_voltage, gun.battery_voltage1);
        saved_level = gun.battery_voltage1;
        last_time = millis();
    }

    int raw = (uint16_t)adc->adc1->analogReadContinuous();

    static int filtered = 0;
    filtered = filtered * .9 + raw * .1;
    // Serial.println(raw);
    float reading = constrain(map(filtered, 16000, 49500, 0, 100), 0, 100);
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

    if (gun.locked == false && gun.button_input)
    {
        int output = constrain(map(gun.throttle_input, 0, 100, 92, max_power), 92, max_power);
        // 106 is 20 amps, about peak usable for levitation.
        //  110 is 30 amps.
        //  Do I need to adjust this to deal with discharge?
        //  Do I want a secret mode to go max power?
        //  What is true max?

        myservo.write(output);
    }
    else
    {
        myservo.write(90);
    }

    // handle top meter text
    if (gun.locked == false)
    {
        if (gun.button_input)
            gauge_throttle_text(" Active ");
        else
        {
            if (maximized)
                gauge_throttle_text(" Turbo ");
            else
                gauge_throttle_text(" Ready ");
        }
    }

    // handle bottom meter text
    char voltage[11];

    if (!double_needle)
    {
        if (((int)gun.battery_voltage1) >= 10)
            sprintf(voltage, "  %.1f V  ", gun.battery_voltage1);
        else
            sprintf(voltage, "   %.1f V  ", gun.battery_voltage1);
    }
    else
    {
        static float result = 0;

        if (!extending)
            result = gun.battery_voltage1 - gun.battery_voltage2;

        if (result < 0)
            sprintf(voltage, "  %.1f ^V ", result);
        else
            sprintf(voltage, "  %.1f ^V ", result);
    }
    gauge_volts_text(voltage);

    // screensaver
    static uint32_t last_motion_time = 0;
    if (gun.button_input || (int)gun.throttle_input > 0)
    {
        last_motion_time = millis();

        gun.screensaver_on = false;
    }

    if (millis() - last_motion_time > TIME_BEFORE_SCREENSAVER && gun.screensaver_on == false)
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

    if (gun.button_input && !gun.locked)
        gun.animation_output = 20 + gun.throttle_input / 6;
    else
        gun.animation_output = 0;

    // low battery alarm

    if (gun.battery_voltage1 < 12.0 && gun.battery_voltage1 > 3)
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
