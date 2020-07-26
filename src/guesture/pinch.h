#pragma once
#include "guesture.h"
struct pinch_guesture_s
{
    struct guesture_s guesture;
    int start_distance2_to_mid;

};

void pinch_guesture_init(struct pinch_guesture_s *guesture);