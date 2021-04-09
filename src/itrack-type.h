#pragma once
#include <sys/time.h>
#include <stdint.h>
struct itrack_action_s{
    struct{
        /** button down */
        int down;
        /** button up */
        int up;
        /** button defer */
        struct{
            int            button;
            struct timeval time;
            enum {
                DEFER_NONE = 0,
                DEFER_NEW,
                DEFER_DELAY,
                DEFER_CANCEL,
                DEFER_TRIGGER_IMMEDIATEY
            }operation;
            void (*callback)(void *user_data);
            uint8_t callback_data[128];
        }defer_up;
    } button;

    int physical_button;

    struct {
        int x,y;
        enum {
            NONE=0,
            ABSOLUTE,
            RELATIVE
        } move_type;
    } pointer;

    struct {
        double x,y;
        double velocity_x,velocity_y;/** point per second */
        int    holding;
    } scroll;

    struct timeval emit_time;
};
