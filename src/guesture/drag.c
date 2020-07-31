#include "./drag.h"
#include "mtstate.h"
#include "guesture_manager.h"
#include <assert.h>
#define SWIPE3_BUTTON 		  0
#define SWIPE3_HOLD_TIME      1000
static void on_accpet_other(struct guesture_manager_s *manager,struct guesture_item_s *item,struct itrack_staged_status_s *staged,void *user_data);
// struct defer_callback_set_value_t {
// 	void *ptr;
// 	uint8_t value[8];
// 	size_t size;
// };

// static void defer_callback_set_value(void *user_data){
// 	LOG_DEBUG("defer_callback_set_value");
// 	struct defer_callback_set_value_t *arg = user_data;
// 	memcpy(arg->ptr,arg->value,arg->size);
// }

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

		// mstime_t timeout_from_end   = time_diff_ms(&touch->update_time,&handler->state.pseudo_endtime);
		// mstime_t timeout_from_start = time_diff_ms(&touch->update_time,&handler->state.pseudo_starttime);
		// mstime_t timeout = MINVAL(timeout_from_start , timeout_from_end);

		// if(touch_count == 1){
		// 	if(timeout < SWIPE3_MOVE_THRESHHOLD_TIME || math_dist2( touch->dx,touch->dy ) < SWIPE3_MOVE_THRESHHOLD_DIST2){
		// 		goto end;
		// 	}else{
		// 		staged->button.up = (1 << SWIPE3_BUTTON );
		// 		staged->button.defer_up.operation = DEFER_CANCEL;
		// 		handler->state.special_type = SpecialTyoe_None;
		// 		// back to normal state
		// 		// goto start;
		// 	}
		// }
		// else if(touch_count == 2){
		// 	if( timeout < SWIPE3_MOVE_THRESHHOLD_TIME || math_dist2( touch->dx,touch->dy ) < SWIPE3_MOVE_THRESHHOLD_DIST2){
		// 		goto end;
		// 	}else{
		// 		staged->button.up = (1 << SWIPE3_BUTTON );
		// 		staged->button.defer_up.operation = DEFER_CANCEL;
		// 		handler->state.special_type = SpecialTyoe_None;
		// 		// back to normal state
		// 		// goto start;
		// 	}
		// }
		// else if(touch_count == 3){
		// 	if(touch_count == 3){
		// 		// if( props->is_absolute_mode ){
		// 		// 	/** not possible */
		// 		// 	goto end;
		// 		// }
		// 		staged->pointer.move_type = RELATIVE;
		// 		touchlist_int_avg(&staged->pointer.x,touch->dx,touch_idx,touch_count);
		// 		touchlist_int_avg(&staged->pointer.y,touch->dy,touch_idx,touch_count);
		// 		staged->button.defer_up.operation = DEFER_CANCEL;
		// 	}
		// }

    struct drag_guesture_s *guesture = user_data;
    struct itrack_staged_status_s *staged = &guesture->guesture.staged;
    const struct Touch *touch;
    int i;
    struct gue_i_vector_s avg_dvect = {0};
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
    			
        staged->pointer.move_type = RELATIVE;
        staged->pointer.x = avg_dvect.x;
        staged->pointer.y = avg_dvect.y;
        staged->button.defer_up.operation = DEFER_CANCEL;
        
    }else{
        if(guesture->guesture.status.match_state == GUESTURE_MATHING){
            guesture_set_match(&guesture->guesture,GUESTURE_SET_MATCH_STATUS_MATCH);
            // todo://
            staged->button.down =  1 << SWIPE3_BUTTON;
            
        }

        // timercp(&handler->state.start_time   , &touch->update_time);
        

        staged->pointer.move_type = RELATIVE;
        staged->pointer.x = avg_dvect.x;
        staged->pointer.y = avg_dvect.y;
    }	
    return ;
}

static Bool on_end(void *user_data,Bool is_cancel,int touch_count){
    GUESTURE_DEBUG("[drag]on_end\n");
    struct drag_guesture_s *guesture = user_data;
    struct itrack_staged_status_s *staged = &guesture->guesture.staged;
    // const struct Touch *touch;
    // timercp(&handler->state.pseudo_endtime, &touch->update_time);

    staged->button.defer_up.operation = DEFER_NEW;
    staged->button.defer_up.button    = 1 << SWIPE3_BUTTON;

    struct timeval now_tv,tv;
    gettimeofday(&now_tv,NULL);
    timeraddms(&now_tv,SWIPE3_HOLD_TIME,&tv);
    staged->button.defer_up.time 	  = tv;
    guesture->hold_time = tv;
    // staged->button.defer_up.callback      = defer_callback_set_value;
    // struct defer_callback_set_value_t *param = (void *) staged->button.defer_up.callback_data;
    // param->ptr   = &handler->state.special_type;
    // *(Bool *)param->value = SpecialTyoe_None;
    // param->size = sizeof(Bool);


    return TRUE;
}
// static void on_deinit(void *user_data){
//     GUESTURE_DEBUG("[drag]on_end\n");
//     struct drag_guesture_s *guesture = user_data;
// }
static void on_accpet_other(struct guesture_manager_s *manager,struct guesture_item_s *item,struct itrack_staged_status_s *staged,void *user_data){
    struct drag_guesture_s *guesture = user_data;

    if(item->guesture != &guesture->guesture){
        if(guesture->hold_time.tv_sec != 0){
            struct timeval tv;
            gettimeofday(&tv,NULL);
            if(time_diff_ms(&tv,&guesture->hold_time) < 0){
                staged->button.up                |= 1 << SWIPE3_BUTTON;
                staged->button.defer_up.operation = DEFER_CANCEL;
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
