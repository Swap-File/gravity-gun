#include <Arduino.h>
#include "FastLED.h"
#include "effects.h"

#define NUM_PARTICLES 6
static Pixels particles[NUM_PARTICLES];

static CRGB leds[NUM_LEDS];

#define NUM_LEDS_XY (NUM_LEDS_X_PHY * NUM_LEDS_Y_PHY)
CRGB leds_xy[NUM_LEDS_X_PHY][NUM_LEDS_Y_PHY]; // maps to X + tip
CRGB leds_tip[NUM_LEDS_TIP_PHY];
CRGB leds_screensaver_xy[NUM_LEDS]; // maps to X + tip

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void effects_init(void)
{
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        particles[i].active = false;
        particles[i].y_pos = i % 6;
        particles[i].x_pos = 0;
    }
}

void effects_update(float throttle)
{
    EVERY_N_MILLISECONDS(20)
    {
        fadeToBlackBy(leds_screensaver_xy, NUM_LEDS, 10);
        int pos = random16(NUM_LEDS);
        leds_screensaver_xy[pos] += CHSV(gHue + random8(64), 200, 255);

        gHue++;

        fadeToBlackBy((CRGB *)leds_xy, NUM_LEDS_XY, 50);
        fadeToBlackBy(leds_tip, NUM_LEDS_TIP_PHY, 50);
    }

    // spawn
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        if (particles[i].active == false && random(10) == 0)
        {
            particles[i].active = true;
            particles[i].x_pos = 0;
            particles[i].y_pos += 1;
            if (particles[i].y_pos > 5)
                particles[i].y_pos = 0;
            particles[i].x_step = 1;
            particles[i].speed = 40; // all have the same base speed
            particles[i].last_time = millis();
            particles[i].hue = random(256);
            // Serial.print(i);
            // Serial.println(" Spawning");
        }
    }

    // move

    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        if ((particles[i].last_time + particles[i].speed - throttle < millis()))
        {
            if (particles[i].active == true)
            {
                particles[i].x_pos += particles[i].x_step;
                particles[i].last_time = millis();
                // Serial.println("Move");

                if (particles[i].x_pos >= NUM_LEDS_X_PHY)
                {
                    particles[i].tip_pos = 0;
                    // light up a random tip light in the correct quadrant
                    if (throttle > 0)
                        leds_tip[random(5) + particles[i].y_pos * 4] |= CHSV(particles[i].hue, 125, 125);
                    particles[i].active = false;
                }
            }
        }
    }

    // render new particles
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        if (particles[i].active == true)
            leds_xy[particles[i].x_pos][particles[i].y_pos] |= CHSV(particles[i].hue, 255, 255);
    }
}