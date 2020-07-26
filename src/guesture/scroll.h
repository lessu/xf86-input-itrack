#pragma once
#include "./guesture.h"
struct scroll_guesture_s
{
    struct guesture_s super;
};

void scroll_guesture_init(struct scroll_guesture_s *guesture);