#pragma once
#include "./guesture.h"

// #define SCROLL_POST_FORBIDEN_TIME_MS (300)
#define SCROLL_GUESTURE_DHISTORY_LENGTH (3)
struct gue_i_vector_s{
    int x,y;
};
struct scroll_guesture_s
{
    struct guesture_s guesture;
    struct {
        float inertial_scroll_multiplier;
    } cfg;

    struct gue_i_vector_s dhistory[SCROLL_GUESTURE_DHISTORY_LENGTH];
    size_t dhistory_length;
    size_t dhistory_full;

    enum {
        SCROLL_TYPE_UNDETERMINTED = 0,
        SCROLL_TYPE_X,
        SCROLL_TYPE_Y,
        SCROLL_TYPE_FREE
    } scroll_type;
};

void scroll_guesture_init(struct scroll_guesture_s *guesture);