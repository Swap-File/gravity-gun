

#ifndef _GUAGUE_H
#define _GUAGUE_H

#include "io.h"

void gauge_init(void);
void gauge_update(const Gravity_Gun *gun);

bool gauge_render_time_check(void);

void gauge_throttle_text(const char * input);
void gauge_volts_text(const char *input);

#endif