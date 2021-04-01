#pragma once
#include "guesture.h"
#include <sys/time.h>
struct drag_guesture_s
{
    struct guesture_s   guesture;
    int                 sum_x,sum_y;
    struct timeval      hold_time;
    BOOL                loaded;
};

void drag_guesture_init(struct drag_guesture_s *guesture);
void drag_guesture_deinit(struct drag_guesture_s *guesture);