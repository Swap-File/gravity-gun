
#ifndef _EFFECTS_H
#define _EFFECTS_H

#include <Arduino.h>

// 13 long, 6 strips
// tip is 24

#define NUM_LEDS_Y_PHY 6
#define NUM_LEDS_X_PHY 13
#define NUM_LEDS_TIP_PHY 24
#define NUM_LEDS (NUM_LEDS_X_PHY * NUM_LEDS_Y_PHY + NUM_LEDS_TIP_PHY)

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
} ;

void effects_update(void);
void effects_init(void);

#endif