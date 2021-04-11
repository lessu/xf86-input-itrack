#include "drag.h"
#include "mtstate.h"
#include "guesture_manager.h"
#include <assert.h>
#define SWIPE3_BUTTON 		  0
#define SWIPE3_HOLD_TIME      1000
static void on_accpet_other(struct guesture_manager_s *manager,struct guesture_item_s *item,void *user_data);

static void on_start(void *user_data,const struct Touch *touches,int touch_bit){
    GUESTURE_DEBUG("[drag]on_start\n");
    struct drag_guesture_s *guesture = user_data;
    if(guesture->loaded == FALSE){
        guesture->loaded = TRUE;
        guesture_manager_register_on_accept_callback(guesture->guesture.manager,on_accpet_other,guesture);
    }
		// timercp(&handler->state.pseudo_starttime,&touch->update_time);
	
}

static void on_update(void *user_data,const struct Touch *touches,int touch_bit){
    GUESTURE_DEBUG("[drag]on_update\n");

    struct drag_guesture_s *guesture = user_data;
    const struct Touch *touch;
    int i;
    struct gi_vector_s avg_dvect = {0};
    foreach_bit(i,touch_bit){
        touch = touches + i;
        avg_dvect.x += touch->dx;
        avg_dvect.y += touch->dy;
    }
    avg_dvect.x /= 3;
    avg_dvect.y /= 3;

    if(ABSVAL(avg_dvect.x) < 2 && ABSVAL(avg_dvect.y)  < 2){
        return ;
    }

    if( guesture_manager_is_accept(guesture->guesture.manager,&guesture->guesture) ){

        guesture_post_movment(&guesture->guesture,avg_dvect.x,avg_dvect.y,FALSE);
        guesture_post_button_up_cancel(&guesture->guesture,SWIPE3_BUTTON);

    }else{

        if(guesture->guesture.status.match_state == GUESTURE_MATHING){
            guesture_set_match(&guesture->guesture,GUESTURE_MATCH_OK,GUSTURE_MATH_OPT_CLEAR_MOVE);
            guesture_post_button_down(&guesture->guesture,SWIPE3_BUTTON);
        }

        // timercp(&handler->state.start_time   , &touch->update_time);
        guesture_post_movment(&guesture->guesture,avg_dvect.x,avg_dvect.y,FALSE);

    }	
    return ;
}

static BOOL on_end(void *user_data,BOOL is_cancel,int touch_count){
    GUESTURE_DEBUG("[drag]on_end\n");
    struct drag_guesture_s *guesture = user_data;
    guesture_post_button_up(&guesture->guesture,SWIPE3_BUTTON,SWIPE3_HOLD_TIME);
    
    struct timeval now_tv;
    gettimeofday(&now_tv,NULL);
    timeraddms(&now_tv,SWIPE3_HOLD_TIME,&guesture->hold_time);

    return TRUE;
}
// static void on_deinit(void *user_data){
//     GUESTURE_DEBUG("[drag]on_end\n");
//     struct drag_guesture_s *guesture = user_data;
// }
static void on_accpet_other(struct guesture_manager_s *manager,struct guesture_item_s *item,void *user_data){
    struct drag_guesture_s *guesture = user_data;

    if(item->guesture != &guesture->guesture){
        if(guesture->hold_time.tv_sec != 0){
            struct timeval tv;
            gettimeofday(&tv,NULL);
            if(time_diff_ms(&tv,&guesture->hold_time) < 0){
                guesture_post_button_up(item->guesture,SWIPE3_BUTTON,0);
                guesture_post_button_up_cancel(item->guesture,SWIPE3_BUTTON);
            }
            guesture->hold_time.tv_sec = 0;
        }
    }
}

static struct guesture_callbacks_s s_callbacks = {
    .on_start   = on_start,
    .on_update  = on_update,
    .on_end     = on_end //,
    // .on_deinit  = on_deinit
};

void drag_guesture_init(struct drag_guesture_s *guesture){
    guesture_init(&guesture->guesture,"drag",&s_callbacks,guesture);
    guesture->guesture.props.required_touches = 3;
    guesture->loaded = FALSE;
}

void drag_guesture_deinit(struct drag_guesture_s *guesture){
    guesture_manager_unregister_on_accept_callback(guesture->guesture.manager,on_accpet_other,guesture);
    guesture_deinit(&guesture->guesture);
}
