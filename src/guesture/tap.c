#include "./tap.h"
#include "mtstate.h"
#include "guesture_manager.h"
#include <assert.h>
#define LOG_TAP LOG_DEBUG

static void on_start(void *user_data,const struct Touch *touches,int touch_bit){
    LOG_TAP("[tap]on_start\n");
}

static void on_update(void *user_data,const struct Touch *touches,int touch_bit){
    LOG_TAP("[tap]on_update\n");
    struct tap_guesture_s *guesture = user_data;

    int count = 0;
    
    guesture->sum_x = 0;
    guesture->sum_y = 0;
    const struct Touch *ptouch;
    int i; foreach_bit(i,touch_bit){
        ptouch = touches + i;
        guesture->sum_x += ptouch->total_dx;
        guesture->sum_y += ptouch->total_dy;
        count++;
    }

    guesture->sum_x /= count;
    guesture->sum_y /= count;
            
    // /** should have within a certain amount of time */
    int64_t diff_ms = time_diff_ms(&ptouch->update_time,&ptouch->create_time);
    uint64_t dist2 = math_dist2(guesture->sum_x,guesture->sum_y);
    if( diff_ms > TAP_TIME_MAX_HOLD_TIME){
        LOG_TAP("[tap] diff_ms > TAP_TIME_MAX_HOLD_TIME\n");
       // not a tap
        guesture_set_match(&guesture->guesture,GUESTURE_SET_MATCH_STATUS_NOT_MATCH,ptouch->update_time);
    }else if( dist2 > TAP_MOVE_DIST2){
        LOG_TAP("[tap] dist2 > TAP_MOVE_DIST2\n");
        // not a tap
        guesture_set_match(&guesture->guesture,GUESTURE_SET_MATCH_STATUS_NOT_MATCH,ptouch->update_time);
    }
    return ;
}

static Bool on_end(void *user_data,Bool is_cancel,int touch_count){
    LOG_TAP("[tap]on_end\n");
    struct tap_guesture_s *guesture = user_data;
    // struct itrack_staged_status_s *staged = guesture->guesture.manager->staged;
    struct timeval time;
 	gettimeofday(&time, NULL);
    // /** last finger leaves */
    // if(guesture->guesture.status.end_touch_count == 1){
        
        LOG_TAP("TAP %d Recognized\n",guesture->guesture.props.required_touches);
        guesture_set_match(&guesture->guesture,GUESTURE_SET_MATCH_STATUS_MATCH,time);
        int tap_button = -1;
        
        if( guesture->guesture.props.required_touches == 1 ){
            tap_button = TAP_1_BUTTON;
        }else if( guesture->guesture.props.required_touches == 2 ){
            tap_button = TAP_2_BUTTON;
        }else if( guesture->guesture.props.required_touches == 3 ){
            tap_button = TAP_3_BUTTON;
        }else if( guesture->guesture.props.required_touches == 4 ){
            tap_button = TAP_4_BUTTON;
        }else{
            tap_button = -1;
        }

        if( tap_button >= 0 ){
            /** it's a tap */
            guesture->guesture.staged.button.down               = (1 << tap_button);
            guesture->guesture.staged.button.defer_up.operation = DEFER_NEW;
            guesture->guesture.staged.button.defer_up.button    = (1 << tap_button);
            struct timeval tv;
            timeraddms(&time,TAP_UP_TIME,&tv);
            guesture->guesture.staged.button.defer_up.time      = tv;
        }
    // }else{
    //     // not fully leaves
    //     // not a tap
    //     guesture_set_match(&guesture->guesture,GUESTURE_SET_MATCH_STATUS_NOT_MATCH);
    // }
    return TRUE;
}

static struct guesture_callbacks_s s_callbacks = {
    .on_start   = on_start,
    .on_update  = on_update,
    .on_end     = on_end
};

void tap_guesture_init(struct tap_guesture_s *guesture,int touch_count){
    guesture_init(&guesture->guesture,&s_callbacks,guesture);
    guesture->guesture.props.required_touches = touch_count;
}