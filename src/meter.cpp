#include "Arduino.h"
#include "meter.h"
#include "GC9A01A_t3n_font_ArialBold.h"
#include "GC9A01A_t3n_font_Arial.h"

// #########################################################################
// Meter constructor
// #########################################################################
MeterWidget::MeterWidget(GC9A01A_t3n *tft)
{
  ltx = 0;              // Saved x coord of bottom of needle
  osx = 120, osy = 120; // Saved x & y coords
  old_analog = 0;       // Value last displayed
  old_digital = -999;   // Value last displayed
  old_analog2 = 0;      // Value last displayed
  old_digital2 = -999;  // Value last displayed
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


// #########################################################################
// Draw meter meter at x, y and define full scale range & the scale labels
// #########################################################################
void MeterWidget::analogMeter(uint16_t x, uint16_t y, float fullScale, const char *label, const char *s0, const char *s1, const char *s2, const char *s3, const char *s4)
{
  // Save offsets for needle plotting
  mx = x;
  my = y;
  factor = 100.0 / fullScale;

  strncpy(mlabel, label, 8);

  strncpy(ms0, s0, 4);
  strncpy(ms1, s1, 4);
  strncpy(ms2, s2, 4);
  strncpy(ms3, s3, 4);
  strncpy(ms4, s4, 4);

  // Meter outline
  // ntft->fillRect(x, y, 239, 126, TFT_BLACK);
  // ntft->fillRect(x + 5, y + 3, 230, 119, TFT_BLACK );

  ntft->setTextColor(WHITE); // Text colour
  ntft->setFont(Arial_9_Bold);
  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -50; i < 51; i += 5)
  {
    // Long scale tick length
    int tl = 15;

    // Coordinates of tick to draw
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = x + sx * (100 + tl) + 120;
    uint16_t y0 = y + sy * (100 + tl) + 140;
    uint16_t x1 = x + sx * 100 + 120;
    uint16_t y1 = y + sy * 100 + 140;

    // Coordinates of next tick for zone fill
    float sx2 = cos((i + 5 - 90) * 0.0174532925);
    float sy2 = sin((i + 5 - 90) * 0.0174532925);
    int x2 = x + sx2 * (100 + tl) + 120;
    int y2 = y + sy2 * (100 + tl) + 140;
    int x3 = x + sx2 * 100 + 120;
    int y3 = y + sy2 * 100 + 140;

    // Red zone limits
    if (redEnd > redStart)
    {
      if (i >= redStart && i < redEnd)
      {
        ntft->fillTriangle(x0, y0, x1, y1, x2, y2, RED);
        ntft->fillTriangle(x1, y1, x2, y2, x3, y3, RED);
      }
    }

    // Orange zone limits
    if (orangeEnd > orangeStart)
    {
      if (i >= orangeStart && i < orangeEnd)
      {
        ntft->fillTriangle(x0, y0, x1, y1, x2, y2, ORANGE);
        ntft->fillTriangle(x1, y1, x2, y2, x3, y3, ORANGE);
      }
    }

    // Yellow zone limits
    if (yellowEnd > yellowStart)
    {
      if (i >= yellowStart && i < yellowEnd)
      {
        ntft->fillTriangle(x0, y0, x1, y1, x2, y2, YELLOW);
        ntft->fillTriangle(x1, y1, x2, y2, x3, y3, YELLOW);
      }
    }

    // Green zone limits
    if (greenEnd > greenStart)
    {
      if (i >= greenStart && i < greenEnd)
      {
        ntft->fillTriangle(x0, y0, x1, y1, x2, y2, GREEN);
        ntft->fillTriangle(x1, y1, x2, y2, x3, y3, GREEN);
      }
    }

    // Short scale tick length
    if (i % 25 != 0)
      tl = 8;

    // Recalculate coords in case tick length changed
    x0 = x + sx * (100 + tl) + 120;
    y0 = y + sy * (100 + tl) + 140;
    x1 = x + sx * 100 + 120;
    y1 = y + sy * 100 + 140;

    // Draw tick
    ntft->drawLine(x0, y0, x1, y1, WHITE);

    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0)
    {
      // Calculate label positions
      x0 = x + sx * (100 + tl + 10) + 120;
      y0 = y + sy * (100 + tl + 10) + 140;
      switch (i / 25)
      {
      case -2:
        ntft->drawString(ms0, x0, y0 - 12);
        break;
      case -1:
        ntft->drawString(ms1, x0, y0 - 9);
        break;
      case 0:
        ntft->drawString(ms2, x0, y0 - 6);
        break;
      case 1:
        ntft->drawString(ms3, x0, y0 - 9);
        break;
      case 2:
        ntft->drawString(ms4, x0, y0 - 12);
        break;
      }
    }

    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * 0.0174532925);
    sy = sin((i + 5 - 90) * 0.0174532925);
    x0 = x + sx * 100 + 120;
    y0 = y + sy * 100 + 140;
    // Draw scale arc, don't draw the last part
    if (i < 50)
      ntft->drawLine(x0, y0, x1, y1, WHITE);
  }
  ntft->setFont(Arial_18_Bold);
  // ntft->drawString(mlabel, x + 5 + 230 - 40, y + 119 - 20, 2); // Units at bottom right
  ntft->drawString(mlabel, x + 120, y + 70); // Comment out to avoid font 4
  // ntft->drawRect(x + 5, y + 3, 230, 119, TFT_WHITE);           // Draw bezel line

  updateNeedle(0, 0);
}

// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void MeterWidget::updateNeedle(float val, float val2)
{
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
  float sx2 = cos(sdeg2 * 0.0174532925);
  float sy2 = sin(sdeg2 * 0.0174532925);

  // Calculate x delta of needle start (does not start at pivot point)
  float tx = tan((sdeg + 90) * 0.0174532925);

  // Calculate x delta of needle start (does not start at pivot point)
  float tx2 = tan((sdeg2 + 90) * 0.0174532925);

  // Erase old needle image
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

  // Re-plot text under needle
  if (old_analog != value || old_analog2 != value2)
  {
    ntft->setTextColor(WHITE);
    ntft->setFont(Arial_18_Bold);
    ntft->drawString(mlabel, mx + 120, my + 70); // // Comment out to avoid font 4
  }

  // Store new needle end coords for next erase
  ltx = tx;
  osx = sx * 98 + 120;
  osy = sy * 98 + 140;

  // Store new needle end coords for next erase
  ltx2 = tx2;
  osx2 = sx2 * 98 + 120;
  osy2 = sy2 * 98 + 140;

  // Draw the needle in the new position, magenta makes needle a bit bolder
  // draws 3 lines to thicken needle
  if (old_analog != value)
  {
    ntft->drawLine(mx + 120 + 30 * ltx - 1, my + 140 - 30, mx + osx - 1, my + osy, RED);
    ntft->drawLine(mx + 120 + 30 * ltx, my + 140 - 30, mx + osx, my + osy, MAGENTA);
    ntft->drawLine(mx + 120 + 30 * ltx + 1, my + 140 - 30, mx + osx + 1, my + osy, RED);
  }
  // Draw the needle in the new position, magenta makes needle a bit bolder
  // draws 3 lines to thicken needle
  if (old_analog2 != value2 && value2 != value)
  {
    ntft->drawLine(mx + 120 + 30 * ltx2 - 1, my + 140 - 30, mx + osx2 - 1, my + osy2, RED);
    ntft->drawLine(mx + 120 + 30 * ltx2, my + 140 - 30, mx + osx2, my + osy2, MAGENTA);
    ntft->drawLine(mx + 120 + 30 * ltx2 + 1, my + 140 - 30, mx + osx2 + 1, my + osy2, RED);
  }

  old_analog = value;
  old_analog2 = value2;
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