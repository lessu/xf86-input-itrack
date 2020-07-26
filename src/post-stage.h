#pragma once
#include "common.h"
#include "itrack-type.h"
#define NATURE_SCROLL 1
#define MAX_BUTTON_NUMBER 32
#define SCROLL_INERTIA_SPEED_TRIGER 1000
#define SCROLL_FRICTION             100000 /** decent 1000 speed persecond*/
#define SCROLL_INERTIA_POST_RATE    (60) /** post 60 event per second*/
#define STEP_DISTANCE_NEEDS 200

struct post_stage_s;
typedef void (*ITrackInertiaScrollStateChangeFn)( const struct post_stage_s *handler , Bool is_on,void *userdata);
/*
* Timers documentation:
* http://www.x.org/releases/X11R7.7/doc/xorg-server/Xserver-spec.html#id2536042
*/
struct defer_arg_s{
	DeviceIntPtr device_ptr;
    void (*callback)(void *user_data);
    uint8_t callback_data[128];
	int button;
};

struct scroll_arg_item_s{
    int duration;
    int start_velocity;
};

struct post_stage_s{
    Bool        is_defering;
    OsTimerPtr  defer_timer;
    struct defer_arg_s defer_arg;

    int         last_button_status;
    struct {
        // double sum_x,sum_y;
        int         velocity_x,velocity_y;
        Bool        is_inertia_scrolling;
        OsTimerPtr  inertia_sliding_timer;
        CARD32      passed_time_ms;
        Bool        last_scroll_detected;
        struct {
            // CARD32      start_time;
            DeviceIntPtr device_ptr;
            struct scroll_arg_item_s x,y;
            CARD32      interval;
        } arg;
        ITrackInertiaScrollStateChangeFn state_change_fn;
        void* state_change_fn_userdata;
    } scroll;

    
};

void itrack_post_init(struct post_stage_s *handler);
void itrack_post_deinit(struct post_stage_s *handler);
void itrack_post_read(struct post_stage_s *handler, DeviceIntPtr device_ptr, const struct timeval *tv,const struct itrack_staged_status_s __nonull *staged_status);
void itrack_post_set_on_inertia_scroll_state_change_callback(struct post_stage_s *handler,ITrackInertiaScrollStateChangeFn fn,void *userdata);