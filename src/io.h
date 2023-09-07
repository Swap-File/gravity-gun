

#ifndef _IO_H
#define _IO_H

void io_init(void);
void io_update(void);

struct Gravity_Gun
{
  bool button_input_last;
  bool button_input;

  int led_output;

  float throttle_input = 0.0;
  float throttle_output = 0.0;
    float animation_output = 0.0;
  

  bool screensaver_on = true;
  bool locked = true;

  float battery_voltage1 = 0.0;
  float battery_voltage2 = 0.0;
};

#endif