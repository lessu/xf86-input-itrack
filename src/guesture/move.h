#pragma once
#include "guesture.h"

#define PHYSICAL_BUTTON_MIN_MOVE (50)

struct move_guesture_s
{
    struct guesture_s guesture;
    enum{
        PHYSICAL_BUTTON_SETTLE_NONE,
        PHYSICAL_BUTTON_SETTLE_SETTLE,
        PHYSICAL_BUTTON_SETTLE_CLEAR
    }physical_button_settle;
};

void move_guesture_init(struct move_guesture_s *guesture);