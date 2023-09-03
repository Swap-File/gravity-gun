#include <Arduino.h>

#define USE_OCTOWS2811
#include <OctoWS2811.h>
#include "FastLED.h"
#include "gauge.h"
// 13 long, 6 strips
// tip is 24

struct Pixels
{
  int x_pos;
  int y_pos;
  int x_step;
  int y_step;
  uint32_t spawn_time;
  uint32_t speed;
  uint32_t next_time;
  uint32_t tip_time;
  int tip_pos;
  bool active;
  uint8_t hue;
} pixeldata;

#define NUM_PARTICLES 3
static Pixels particles[NUM_PARTICLES];

#define NUM_LEDS_Y_PHY 6
#define NUM_LEDS_X_PHY 13
#define NUM_LEDS_TIP_PHY 24
#define NUM_LEDS (NUM_LEDS_X_PHY * NUM_LEDS_Y_PHY + NUM_LEDS_TIP_PHY)

static CRGB Output_Array[(NUM_LEDS)*8]; // actual output for Fastled with OctoWS2811
static CRGB leds[NUM_LEDS];

#define NUM_LEDS_XY (NUM_LEDS_X_PHY * NUM_LEDS_Y_PHY)
static CRGB leds_xy[NUM_LEDS_X_PHY][NUM_LEDS_Y_PHY]; // maps to X + tip
static CRGB leds_tip[NUM_LEDS_TIP_PHY];

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void setup(void)
{
  FastLED.addLeds<OCTOWS2811>(Output_Array, NUM_LEDS);
  FastLED.setBrightness(64);

  gauge_init();

  for (int i = 0; i < NUM_PARTICLES; i++){
    particles[i].active = false;
    particles[i].y_pos = i * 2;
    particles[i].x_pos = 0;
  }
}

void loop()
{

  gauge_update();


  EVERY_N_MILLISECONDS(20) { gHue++; } // slowly cycle the "base color" through the rainbow

  // spawn
  for (int i = 0; i < NUM_PARTICLES; i++)
  {
    if (particles[i].active == false)
    {
      particles[i].active = true;
      particles[i].x_pos = 0;
      particles[i].y_pos += 1;
      if (particles[i].y_pos > 5 )
       particles[i].y_pos = 0;
      particles[i].x_step = 1;
      particles[i].tip_pos = -1;
      particles[i].speed = 10;
      particles[i].next_time = millis() + particles[i].speed;
      particles[i].hue = random(256);
      //Serial.print(i);
      //Serial.println(" Spawning");
    }
  }

  // move

  for (int i = 0; i < NUM_PARTICLES; i++)
  {
    if (particles[i].next_time < millis())
    {
      if (particles[i].tip_pos < 0)
      {
        if (particles[i].active == true)
        {
          particles[i].x_pos += particles[i].x_step;
          particles[i].next_time = millis() + particles[i].speed;
          //Serial.println("Move");
        }

        if (particles[i].x_pos >= NUM_LEDS_X_PHY)
        {
          particles[i].tip_pos = 0;
          //Serial.println("Tip Move");
        }
      }
    }
  }

  // destory
  for (int i = 0; i < NUM_PARTICLES; i++)
  {

    if (particles[i].active == true && particles[i].tip_pos >= 0)
    {
      if (millis() - particles[i].tip_time > 1000)
      {
        particles[i].active = false;
        //Serial.println("Destroy");
      }
    }
  }

  // dim everything
   EVERY_N_MILLISECONDS(20) {
  fadeToBlackBy((CRGB *)leds_xy, NUM_LEDS_XY, 50);
  fadeToBlackBy(leds_tip, NUM_LEDS_TIP_PHY, 50);
   }
  // render new particles
  for (int i = 0; i < NUM_PARTICLES; i++)
  {
    if (particles[i].active == true)
    {
      if (particles[i].tip_pos < 0)
      {
        leds_xy[particles[i].x_pos][particles[i].y_pos] |= CHSV(particles[i].hue, 255, 255);
      }

      else
      {
        leds_tip[particles[i].tip_pos] |= CHSV(particles[i].hue, 125, 125);
      }
    }
  }

  // X 1d render

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

  /*
  for (int i = (NUM_LEDS_X_PHY * NUM_LEDS_Y); i < (NUM_LEDS_X_PHY * NUM_LEDS_Y + NUM_LEDS_TIP_PHY); i++)
  {
    Output_Array[i] = leds_x[NUM_LEDS_X - 1];
  }
  */

  FastLED.show();


}
