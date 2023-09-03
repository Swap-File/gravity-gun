
#include <GC9A01A_t3n.h>
#include "font_consola.h"

const char *black_mesa[20] = {
    "           .-;+$XHHHHHHX$+;-.           ",
    "        ,;X@@X%/;=----=:/%X@@X/,        ",
    "      =$@@%=.              .=+H@X:      ",
    "    -XMX:                      =XMX=    ",
    "   /@@:                          =H@+   ",
    "  %@X,                            .$@$  ",
    " +@X.                               $@% ",
    "-@@,                                .@@=",
    "%@%                                  +@$",
    "H@:                                  :@H",
    "H@:         :HHHHHHHHHHHHHHHHHHX,    =@H",
    "%@%         ;@M@@@@@@@@@@@@@@@@@H-   +@$",
    "=@@,        :@@@@@@@@@@@@@@@@@@@@@= .@@:",
    " +@X        :@@@@@@@@@@@@@@@M@@@@@@:%@% ",
    "  $@$,      ;@@@@@@@@@@@@@@@@@M@@@@@@$. ",
    "   +@@HHHHHHH@@@@@@@@@@@@@@@@@@@@@@@+   ",
    "    =X@@@@@@@@@@@@@@@@@@@@@@@@@@@@X=    ",
    "      :$@@@@@@@@@@@@@@@@@@@M@@@@$:      ",
    "        ,;$@@@@@@@@@@@@@@@@@@X/-        ",
    "           .-;+$XXHHHHHX$+;-.           "};

static int x = 0;
static int y = 0;

static int blanked = 0;

static uint32_t start_time = 0;

static bool logo_time_check(void)
{
   if (millis() - start_time > 4)
      return true;
   return false;
}

bool logo_blank(GC9A01A_t3n *tft)
{

   start_time = millis();
   while (blanked < 121) // blank the screen if needed
   {
      tft->drawFastHLine(0, blanked, 240, BLACK);
      tft->drawFastHLine(0, 240 - blanked, 240, BLACK);
      blanked++;
      if (logo_time_check())
         return false;
   }
   if (blanked < 121)
      return false;

   return true;
}

void logo_update(GC9A01A_t3n *tft)
{
   start_time = millis();
   tft->setFont(Consolas_7);

   while (1) // this will always spend ~5ms rendering the logo
   {
      tft->setCursor(0, y * 11 + 20, true);
      for (x = 0; x < 40; x++)
      {

         if (random(5) == 0)
            tft->setTextColor(random(0xFFFF));
         else
            tft->setTextColor(WHITE);

         tft->print(black_mesa[y][x]);
      }
      y++;
      if (y >= 20)
         y = 0;

      if (logo_time_check())
         break;
   }
}

void logo_reset(void)
{
   blanked = 0;
}