#pragma once
#include "./guesture.h"
#define TAP_TIME_MAX_HOLD_TIME      200
#define TAP_UP_TIME  				20
#define TAP_MOVE_DIST2    		    (35*35)

#define TAP_1_BUTTON 	      0
#define TAP_2_BUTTON 	      2
#define TAP_3_BUTTON 	      1
#define TAP_4_BUTTON 	      3

struct tap_guesture_s
{
    struct guesture_s   guesture;
    int                 sum_x,sum_y;
};

void tap_guesture_init(struct tap_guesture_s *guesture,int touch_count);