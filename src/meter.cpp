#include "Arduino.h"
#include "meter.h"
#include "GC9A01A_t3n_font_ArialBold.h"
#include "GC9A01A_t3n_font_Arial.h"
#include "font_LiberationMono.h"
#include "gauge.h"

// #########################################################################
// Meter constructor
// #########################################################################
MeterWidget::MeterWidget(GC9A01A_t3n *tft)
{
  ltx = 0;              // Saved x coord of bottom of needle
  osx = 120, osy = 120; // Saved x & y coords
  old_analog = 0;       // Value last displayed
  old_analog2 = 0;      // Value last displayed
  mx = 0;
  my = 0;

  factor = 1.0;

  mlabel[8] = '\0';

  // Defaults
  strncpy(ms0, "0", 4);
  strncpy(ms1, "25", 4);
  strncpy(ms2, "50", 4);
  strncpy(ms3, "75", 4);
  strncpy(ms4, "100", 4);

  redStart = 0;
  redEnd = 0;
  orangeStart = 0;
  orangeEnd = 0;
  yellowStart = 0;
  yellowEnd = 0;
  greenStart = 0;
  greenEnd = 0;

  ntft = tft;
}

void MeterWidget::analogMeterDrawReset(void)
{
  complete = false;
  first = true;
  i = -50;
  old_analog2 = -1;
  old_analog = -1;
}

bool MeterWidget::analogMeterDraw(void)
{
  if (complete == 1)
    return true;

  ntft->setTextColor(WHITE); // Text colour
  ntft->setFont(Arial_9_Bold);
  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  while (i < 51)
  {
    if (gauge_render_time_check())
      return false;

    // Long scale tick length
    int tl = 15;

    // Coordinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    x0_local = x_local + sx * (100 + tl) + 120;
    y0_local = y_local + sy * (100 + tl) + 140;
    x1_local = x_local + sx * 100 + 120;
    y1_local = y_local + sy * 100 + 140;

    // Coordinates of next tick for zone fill
    float sx2_local = cos((i + 5 - 90) * 0.0174532925);
    float sy2_local = sin((i + 5 - 90) * 0.0174532925);
    x2_local = x_local + sx2_local * (100 + tl) + 120;
    y2_local = y_local + sy2_local * (100 + tl) + 140;
    x3_local = x_local + sx2_local * 100 + 120;
    y3_local = y_local + sy2_local * 100 + 140;

    // Red zone limits
    if (redEnd > redStart)
    {
      if (i >= redStart && i < redEnd)
      {
        ntft->fillTriangle(x0_local, y0_local, x1_local, y1_local, x2_local, y2_local, RED);
        ntft->fillTriangle(x1_local, y1_local, x2_local, y2_local, x3_local, y3_local, RED);
      }
    }

    // Orange zone limits
    if (orangeEnd > orangeStart)
    {
      if (i >= orangeStart && i < orangeEnd)
      {
        ntft->fillTriangle(x0_local, y0_local, x1_local, y1_local, x2_local, y2_local, ORANGE);
        ntft->fillTriangle(x1_local, y1_local, x2_local, y2_local, x3_local, y3_local, ORANGE);
      }
    }

    // Yellow zone limits
    if (yellowEnd > yellowStart)
    {
      if (i >= yellowStart && i < yellowEnd)
      {
        ntft->fillTriangle(x0_local, y0_local, x1_local, y1_local, x2_local, y2_local, YELLOW);
        ntft->fillTriangle(x1_local, y1_local, x2_local, y2_local, x3_local, y3_local, YELLOW);
      }
    }

    // Green zone limits
    if (greenEnd > greenStart)
    {
      if (i >= greenStart && i < greenEnd)
      {
        ntft->fillTriangle(x0_local, y0_local, x1_local, y1_local, x2_local, y2_local, GREEN);
        ntft->fillTriangle(x1_local, y1_local, x2_local, y2_local, x3_local, y3_local, GREEN);
      }
    }

    // Short scale tick length
    if (i % 25 != 0)
      tl = 8;

    // Recalculate coords in case tick length changed
    x0_local = x_local + sx * (100 + tl) + 120;
    y0_local = y_local + sy * (100 + tl) + 140;
    x1_local = x_local + sx * 100 + 120;
    y1_local = y_local + sy * 100 + 140;

    // Draw tick
    ntft->drawLine(x0_local, y0_local, x1_local, y1_local, WHITE);

    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0)
    {
      // Calculate label positions
      x0_local = x_local + sx * (100 + tl + 10) + 120;
      y0_local = y_local + sy * (100 + tl + 10) + 140;
      switch (i / 25)
      {
      case -2:
        ntft->drawString(ms0, x0_local, y0_local - 12);
        break;
      case -1:
        ntft->drawString(ms1, x0_local, y0_local - 9);
        break;
      case 0:
        ntft->drawString(ms2, x0_local, y0_local - 6);
        break;
      case 1:
        ntft->drawString(ms3, x0_local, y0_local - 9);
        break;
      case 2:
        ntft->drawString(ms4, x0_local, y0_local - 12);
        break;
      }
    }

    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);
    x0_local = x_local + sx * 100 + 120;
    y0_local = y_local + sy * 100 + 140;
    // Draw scale arc, don't draw the last part
    if (i < 50)
      ntft->drawLine(x0_local, y0_local, x1_local, y1_local, WHITE);

    i += 5;
  }

  ntft->setFont(Arial_18_Bold);
  ntft->drawString(mlabel, x_local + 120, y_local + 70); // Comment out to avoid font 4

  complete = true;
  return true;
}
// #########################################################################
// Draw meter meter at x, y and define full scale range & the scale labels
// #########################################################################
void MeterWidget::analogMeter(uint16_t x, uint16_t y, float fullScale, const char *label, const char *s0, const char *s1, const char *s2, const char *s3, const char *s4)
{

  // Save offsets for needle plotting
  mx = x;
  my = y;
  factor = 100.0 / fullScale;
  x_local = x;
  y_local = y;
  strncpy(mlabel, label, 8);

  strncpy(ms0, s0, 4);
  strncpy(ms1, s1, 4);
  strncpy(ms2, s2, 4);
  strncpy(ms3, s3, 4);
  strncpy(ms4, s4, 4);
}

void MeterWidget::updateText(const char *label) // only change this at zero scale so we don't collide with the needle.
{
  strncpy(mlabel, label, 8);
  text = true;
}

// #########################################################################
// Update needle position
// #########################################################################

void MeterWidget::updateNeedle(float val, float val2)
{
  if (gauge_render_time_check())
    return;

  int value = val * factor;

  if (value < -10)
    value = -10; // Limit value to emulate needle end stops
  if (value > 110)
    value = 110;

  int value2 = val2 * factor;

  if (value2 < -10)
    value2 = -10; // Limit value to emulate needle end stops
  if (value2 > 110)
    value2 = 110;

  float sdeg = map(value, -10, 110, -150, -30); // Map value to angle
  // Calculate tip of needle coords
  float sx = cos(sdeg * 0.0174532925);
  float sy = sin(sdeg * 0.0174532925);

  float sdeg2 = map(value2, -10, 110, -150, -30); // Map value to angle
  // Calculate tip of needle coords
  float sx2_local = cos(sdeg2 * 0.0174532925);
  float sy2_local = sin(sdeg2 * 0.0174532925);

  // Calculate x delta of needle start (does not start at pivot point)
  float tx = tan((sdeg + 90) * 0.0174532925);

  // Calculate x delta of needle start (does not start at pivot point)
  float tx2_local = tan((sdeg2 + 90) * 0.0174532925);

  // Erase old needle image
  if (!first)
  {
    if (old_analog != value)
    {
      ntft->drawLine(mx + 120 + 30 * ltx - 1, my + 140 - 30, mx + osx - 1, my + osy, BLACK);
      ntft->drawLine(mx + 120 + 30 * ltx, my + 140 - 30, mx + osx, my + osy, BLACK);
      ntft->drawLine(mx + 120 + 30 * ltx + 1, my + 140 - 30, mx + osx + 1, my + osy, BLACK);
    }

    // Erase old needle image
    if (old_analog2 != value2)
    {
      ntft->drawLine(mx + 120 + 30 * ltx2 - 1, my + 140 - 30, mx + osx2 - 1, my + osy2, BLACK);
      ntft->drawLine(mx + 120 + 30 * ltx2, my + 140 - 30, mx + osx2, my + osy2, BLACK);
      ntft->drawLine(mx + 120 + 30 * ltx2 + 1, my + 140 - 30, mx + osx2 + 1, my + osy2, BLACK);
    }
  }
  // Re-plot text under needle
  if (old_analog != value || old_analog2 != value2 || text)
  {
    if (text)
    
     ntft->setTextColor(WHITE,BLACK);
    else
    ntft->setTextColor(WHITE);
    ntft->setFont(Arial_18_Bold);
    ntft->drawString(mlabel, mx + 120, my + 70);
  }

  // Draw the needle in the new position, magenta makes needle a bit bolder
  // draws 3 lines to thicken needle
  if (old_analog != value || first || text)
  {
    // Store new needle end coords for next erase
    ltx = tx;
    osx = sx * 98 + 120;
    osy = sy * 98 + 140;

    ntft->drawLine(mx + 120 + 30 * ltx - 1, my + 140 - 30, mx + osx - 1, my + osy, RED);
    ntft->drawLine(mx + 120 + 30 * ltx, my + 140 - 30, mx + osx, my + osy, MAGENTA);
    ntft->drawLine(mx + 120 + 30 * ltx + 1, my + 140 - 30, mx + osx + 1, my + osy, RED);
    old_analog = value;
    if (value != value2)
      old_analog2 = -1;
  }

  // Draw the needle in the new position, magenta makes needle a bit bolder
  // draws 3 lines to thicken needle
  if (old_analog2 != value2 || first|| text)
  {
    // Store new needle end coords for next erase
    ltx2 = tx2_local;
    osx2 = sx2_local * 98 + 120;
    osy2 = sy2_local * 98 + 140;

    ntft->drawLine(mx + 120 + 30 * ltx2 - 1, my + 140 - 30, mx + osx2 - 1, my + osy2, RED);
    ntft->drawLine(mx + 120 + 30 * ltx2, my + 140 - 30, mx + osx2, my + osy2, MAGENTA);
    ntft->drawLine(mx + 120 + 30 * ltx2 + 1, my + 140 - 30, mx + osx2 + 1, my + osy2, RED);

    old_analog2 = value2;
  }
  text = false;
  first = false;
}

// #########################################################################
// Set red, orange, yellow and green start+end zones as a % of full scale
// #########################################################################
void MeterWidget::setZones(uint16_t rs, uint16_t re, uint16_t os, uint16_t oe, uint16_t ys, uint16_t ye, uint16_t gs, uint16_t ge)
{
  // Meter scale is -50 to +50
  redStart = rs - 50;
  redEnd = re - 50;
  orangeStart = os - 50;
  orangeEnd = oe - 50;
  yellowStart = ys - 50;
  yellowEnd = ye - 50;
  greenStart = gs - 50;
  greenEnd = ge - 50;
}