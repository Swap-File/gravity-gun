#include <Arduino.h>
#define USE_OCTOWS2811
#include <OctoWS2811.h>
#include "FastLED.h"
#include "gauge.h"
#include "effects.h"
#include "io.h"

extern CRGB leds_xy[NUM_LEDS_X_PHY][NUM_LEDS_Y_PHY];
extern CRGB leds_tip[NUM_LEDS_TIP_PHY];
extern CRGB leds_screensaver_xy[NUM_LEDS_X_PHY * NUM_LEDS_Y_PHY + NUM_LEDS_TIP_PHY];
static CRGB Output_Array[(NUM_LEDS)*8]; // actual output for FastLED with OctoWS2811
extern Gravity_Gun gun;

void setup(void)
{
  FastLED.addLeds<OCTOWS2811>(Output_Array, NUM_LEDS);
  FastLED.setBrightness(128);
  io_init();
  gauge_init();
  effects_init();
}

void loop()
{
  io_update();
  gauge_update(&gun);
  effects_update(gun.animation_output);

  // Render
  for (int i = 0; i < NUM_LEDS_X_PHY; i++)
  {
    Output_Array[NUM_LEDS_X_PHY * 0 + i] = leds_xy[i][0];
    Output_Array[NUM_LEDS_X_PHY * 1 + (NUM_LEDS_X_PHY - i - 1)] = leds_xy[i][1];
    Output_Array[NUM_LEDS_X_PHY * 2 + i] = leds_xy[i][2];
    Output_Array[NUM_LEDS_X_PHY * 3 + (NUM_LEDS_X_PHY - i - 1)] = leds_xy[i][3];
    Output_Array[NUM_LEDS_X_PHY * 4 + i] = leds_xy[i][4];
    Output_Array[NUM_LEDS_X_PHY * 5 + (NUM_LEDS_X_PHY - i - 1)] = leds_xy[i][5];
  }
  for (int i = 0; i < NUM_LEDS_TIP_PHY; i++)
  {
    Output_Array[NUM_LEDS_X_PHY * NUM_LEDS_Y_PHY + i] = leds_tip[NUM_LEDS_TIP_PHY - i - 1];
  }

  for (int i = 0; i < NUM_LEDS; i++){
    Output_Array[i] |= leds_screensaver_xy[i];
  }

  FastLED.show();
}
