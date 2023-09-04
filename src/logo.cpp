
#include <GC9A01A_t3n.h>
#include "font_consola.h"
#include "gauge.h"

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

static int blanked = 0;

bool logo_blank(GC9A01A_t3n *tft)
{

   while (blanked <= (tft->height() / 2)) // blank the screen if needed
   {
      if (gauge_render_time_check())
         return false;

      tft->drawFastHLine(0, blanked, tft->width(), BLACK);
      tft->drawFastHLine(0, tft->height() - blanked, tft->width(), BLACK);
      blanked++;
   }

   return true;
}

void logo_update(GC9A01A_t3n *tft)
{
   tft->setFont(Consolas_7);

   while (1)
   {
      int x = random(40);
      int y = random(20);

      tft->setCursor(4 + x * 6, y * 11 + 20, true);

      if (random(5) == 0)
         tft->setTextColor(random(0xFFFF));
      else
         tft->setTextColor(WHITE);

      tft->print(black_mesa[y][x]);

      if (gauge_render_time_check())
         return;
   }
}

void logo_reset(void)
{
   blanked = 0;
}