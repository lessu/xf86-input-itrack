#include "tap.h"
#include "mtstate.h"
#include "guesture_manager.h"
#include <assert.h>

static void on_start(void *user_data,const struct Touch *touches,int touch_bit){
    TAP_GUESTURE_DEBUG("[tap]on_start\n");
    // struct tap_guesture_s *guesture = user_data;
    // guesture->update_event_count = 0;
}

static void on_update(void *user_data,const struct Touch *touches,int touch_bit){
    TAP_GUESTURE_DEBUG("[tap]on_update\n");
    struct tap_guesture_s *guesture = user_data;

    // guesture->update_event_count ++;

    BOOL is_thumb = FALSE;
    int count = 0;
    int max_move_dist = 0;
    guesture->sum_x = 0;
    guesture->sum_y = 0;
    const struct Touch *ptouch;
    int i; foreach_bit(i,touch_bit){
        ptouch = touches + i;
        guesture->sum_x += ptouch->total_dx;
        guesture->sum_y += ptouch->total_dy;
        count++;
        if(abs(ptouch->dx) > max_move_dist){
            max_move_dist = abs(ptouch->dx);
        }
        if(abs(ptouch->dy) > max_move_dist){
            max_move_dist = abs(ptouch->dy);
        }
        if(ptouch->flags & (1<<MT_THUMB)){
            is_thumb = TRUE;
        }
    }

    guesture->sum_x /= count;
    guesture->sum_y /= count;
            
    /** should have within a certain amount of time */
    int64_t diff_ms = time_diff_ms(&ptouch->update_time,&ptouch->create_time);
    uint64_t dist2 = math_dist2(guesture->sum_x,guesture->sum_y);
    if(is_thumb){
        TAP_GUESTURE_DEBUG("ignore thumb\n");
       // not a tap
        guesture_set_match(&guesture->guesture,GUESTURE_MATCH_NO);
    }else if( diff_ms > TAP_TIME_MAX_HOLD_TIME){
        TAP_GUESTURE_DEBUG("[tap] diff_ms > TAP_TIME_MAX_HOLD_TIME\n");
       // not a tap
        guesture_set_match(&guesture->guesture,GUESTURE_MATCH_NO);
    }else if( max_move_dist > TAP_MOVE_UPDATE_DIST){
        TAP_GUESTURE_DEBUG("[tap] max_move_dist > TAP_MOVE_UPDATE_DIST\n");
        // not a tap
        guesture_set_match(&guesture->guesture,GUESTURE_MATCH_NO);
    }else if( dist2 > TAP_MOVE_TOTAL_DIST2){
        TAP_GUESTURE_DEBUG("[tap] dist2 > TAP_MOVE_TOTAL_DIST2\n");
        // not a tap
        guesture_set_match(&guesture->guesture,GUESTURE_MATCH_NO);
    }
    return ;
}

static BOOL on_end(void *user_data,BOOL is_cancel,int touch_count){
    TAP_GUESTURE_DEBUG("[tap]on_end\n");
    struct tap_guesture_s *guesture = user_data;
    struct timeval time;
 	gettimeofday(&time, NULL);
    // if(guesture->update_event_count > 0){
        TAP_GUESTURE_DEBUG("TAP %d Recognized\n",guesture->guesture.props.required_touches);
        guesture_set_match(&guesture->guesture,GUESTURE_MATCH_OK);
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
            guesture_post_button_down_and_up(&guesture->guesture,tap_button,TAP_UP_TIME);
        }
    // }else{
        // guesture_set_match(&guesture->guesture,GUESTURE_MATCH_NO);
    // }
    return TRUE;
}

static struct guesture_callbacks_s s_callbacks = {
    .on_start   = on_start,
    .on_update  = on_update,
    .on_end     = on_end
};

void tap_guesture_init(struct tap_guesture_s *guesture,int touch_count){
    guesture_init(&guesture->guesture,"tap",&s_callbacks,guesture);
    sprintf(guesture->guesture.name+strlen(guesture->guesture.name),"%d",touch_count);
    guesture->guesture.props.required_touches = touch_count;
}