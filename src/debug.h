#pragma once
#include "mtstate.h"
#include "itrack.h"
void log_touches(const struct itrack *itrack,const struct Touch *touches,int mask);

void debug_fifo_enable( void );