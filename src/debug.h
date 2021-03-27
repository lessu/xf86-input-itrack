#pragma once
#include "mtstate.h"
#include "itrack.h"
void log_touches(const itrack_t *itrack,const struct Touch *touches,int mask);

void debug_fifo_enable( void );