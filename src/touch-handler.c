#include "touch-handler.h"
#include "itrack-main.h"
#include <stdint.h>
#include "guesture/guesture_manager.h"
#include "guesture/tap.h"
#include "guesture/move.h"
#include "guesture/scroll.h"
#include "guesture/drag.h"
#include "guesture/pinch.h"
#define LOG_SCROLL LOG_DEBUG

static int touchlist_release_count(const struct Touch *touches,int touchbit){
	int i = 0;
	int count = 0;
	foreach_bit(i, touchbit){
		if( GETBIT(touches[i].flags,MT_RELEASED) ){
			count ++;
		}
	}
	return count;
}

static void touchlist_int_avg(int *property,int value,int touch_idx,int total_touch){
	if(touch_idx == 0){
		*property = value / total_touch;
	}
	else {
		*property = *property + value / total_touch;
	}
}

static void touchlist_double_avg(double *property,double value,int touch_idx,int total_touch){
	if(touch_idx == 0){
		*property = value / total_touch;
	}
	else {
		*property = *property + value / total_touch;
	}
}

struct defer_callback_set_value_t {
	void *ptr;
	uint8_t value[8];
	size_t size;
};

static void defer_callback_set_value(void *user_data){
	struct defer_callback_set_value_t *arg = user_data;
	memcpy(arg->ptr,arg->value,arg->size);
}

static void touch_state_reset( touch_handler_t *handler ){
	handler -> start = FALSE;
	handler -> max_touches_recorded = 0;
	
	handler->tap.tap_disabled = FALSE;
	handler->tap.sum_x = 0;
	handler->tap.sum_y = 0;

	/** move guard can init itself*/
	// move_guard
	handler->move_guard.pretap_settle = SETTLE_NONE;
	/** dont' reset state */
	// state

}
static struct drag_guesture_s    *s_drag_guesture    = NULL;
void touch_handler_init(touch_handler_t *handler,const struct itrack_props_s *props){
	struct guesture_manager_s  *manager = &handler->guesture_manager;
    touch_state_reset(handler);
	handler->state.special_type = SpecialTyoe_None;


	struct move_guesture_s    *move_guesture    = malloc(sizeof(struct move_guesture_s));
	struct tap_guesture_s     *tap1_guesture    = malloc(sizeof(struct tap_guesture_s));
	struct tap_guesture_s     *tap2_guesture    = malloc(sizeof(struct tap_guesture_s));
	struct tap_guesture_s     *tap3_guesture    = malloc(sizeof(struct tap_guesture_s));
	struct scroll_guesture_s  *scroll_guesture  = malloc(sizeof(struct scroll_guesture_s));
	// struct pinch_guesture_s   *pinch_guesture   = malloc(sizeof(struct pinch_guesture_s));
	s_drag_guesture    = malloc(sizeof(struct drag_guesture_s));
    move_guesture_init(move_guesture);
	tap_guesture_init(tap1_guesture,1);
	tap_guesture_init(tap2_guesture,2);
	tap_guesture_init(tap3_guesture,3);
	scroll_guesture_init(scroll_guesture);
	drag_guesture_init(s_drag_guesture);
	// pinch_guesture_init(pinch_guesture);

	guesture_manager_init(manager);
    guesture_manager_add(manager,&move_guesture->guesture,1);
	guesture_manager_add(manager,&tap1_guesture->guesture,32);
	guesture_manager_add(manager,&tap2_guesture->guesture,33);
	guesture_manager_add(manager,&tap3_guesture->guesture,34);
	guesture_manager_add(manager,&scroll_guesture->guesture,16);
	guesture_manager_add(manager,&s_drag_guesture->guesture,17);
	// guesture_manager_add(manager,&pinch_guesture->guesture,18);

}
void touch_handler_deinit(touch_handler_t *handler){
	struct guesture_manager_s *manager = &handler->guesture_manager;
	drag_guesture_deinit(s_drag_guesture);
	for(int i = 0; i < MAX_GUESTURE_COUNT; i ++ ){
        if( manager->guesture_list[i].used == USING ){
            free(manager->guesture_list[i].guesture);
        }
    }
}

void on_touch_start(touch_handler_t *handler,const struct Touch *touch,int touch_count,const struct Touch *touchlist,int touchbit,struct itrack_staged_status_s *staged,const struct itrack_props_s *props){
	if(props->cfg.api == 2){
		//,props
		guesture_manager_touch_start(&handler->guesture_manager,touch,touch_count,touchlist,touchbit,staged);
		return ;
	}
	// LOG_DEBUG("on_touch_start count = %d\n",touch_count);
	if(touch_count == 0){
		/** noway */
		return ;
	} 
	if( handler -> state.special_type != SpecialTyoe_None){
	
		timercp(&handler->state.pseudo_starttime,&touch->update_time);
	
	}else{

		if(touch_count == 1) {
			if(handler->start != FALSE){
				LOG_WARNING("handler.start != FALSE , strange it shouldn't happen");
			}
			touch_state_reset(handler);
			handler->start = TRUE;
			handler->move_guard.pretap_settle = SETTLE_SETTLE;
		}else if(touch_count == 2) {
			/** init scroll */
			staged->scroll.holding = TRUE;
			staged->scroll.velocity_x = 0;
			staged->scroll.velocity_y = 0;
			staged->scroll.x = 0;
			staged->scroll.y = 0;
			/** if two finger down when scrolling, that is called holding scrolling, should disable tap*/
			if( handler->scroll_state.is_inertia_sliding ){
				LOG_DEBUG("scroll hold disable tap\n");
				handler->tap.tap_disabled  = TRUE;
			}else if( time_diff_ms(&touch->update_time, &handler->scroll_state.last_off_time) < SCROLL_POST_FORBIDEN_TIME_MS ){
				LOG_DEBUG("scroll hold disable tap forbiden time\n");
				handler->tap.tap_disabled  = TRUE;
			}
		}

		if(touch_count > handler->max_touches_recorded){
			handler->max_touches_recorded = touch_count;
			LOG_DEBUG("max_touches_recorded = %d\n",handler->max_touches_recorded);
		}
	}
	

}

void on_touch_move(touch_handler_t *handler,const struct Touch *touch,int touch_idx,int touch_count,const struct Touch *touchlist,int touchbit,struct itrack_staged_status_s *staged,const struct itrack_props_s *props){
	if(props->cfg.api == 2){
		// ,props
		guesture_manager_touch_update(&handler->guesture_manager,touch,touch_idx,touch_count,touchlist,touchbit,staged);
		return ;
	}
// start:
	if( handler->state.special_type != SpecialTyoe_None){
		int release_count = touchlist_release_count(touchlist,touchbit);
		if(release_count > 0){
			goto end;
		}
		mstime_t timeout_from_end   = time_diff_ms(&touch->update_time,&handler->state.pseudo_endtime);
		mstime_t timeout_from_start = time_diff_ms(&touch->update_time,&handler->state.pseudo_starttime);
		mstime_t timeout = MINVAL(timeout_from_start , timeout_from_end);

		if(touch_count == 1){
			if(timeout < SWIPE3_MOVE_THRESHHOLD_TIME || math_dist2( touch->dx,touch->dy ) < SWIPE3_MOVE_THRESHHOLD_DIST2){
				goto end;
			}else{
				staged->button.up = (1 << SWIPE3_BUTTON );
				staged->button.defer_up.operation = DEFER_CANCEL;
				handler->state.special_type = SpecialTyoe_None;
				// back to normal state
				// goto start;
			}
		}
		else if(touch_count == 2){
			if( timeout < SWIPE3_MOVE_THRESHHOLD_TIME || math_dist2( touch->dx,touch->dy ) < SWIPE3_MOVE_THRESHHOLD_DIST2){
				goto end;
			}else{
				staged->button.up = (1 << SWIPE3_BUTTON );
				staged->button.defer_up.operation = DEFER_CANCEL;
				handler->state.special_type = SpecialTyoe_None;
				// back to normal state
				// goto start;
			}
		}
		else if(touch_count == 3){
			if(touch_count == 3){
				// if( props->is_absolute_mode ){
				// 	/** not possible */
				// 	goto end;
				// }
				staged->pointer.move_type = RELATIVE;
				touchlist_int_avg(&staged->pointer.x,touch->dx,touch_idx,touch_count);
				touchlist_int_avg(&staged->pointer.y,touch->dy,touch_idx,touch_count);
				staged->button.defer_up.operation = DEFER_CANCEL;
			}
		}
	}else{

		/** pre tap movement inherbition ,debounce the small movement when touchdown before recognized as a tap*/
		if( handler->move_guard.pretap_settle == SETTLE_SETTLE && time_diff_ms(&touch->update_time,&touch->create_time) < TAP_TIME_MAX_HOLD_TIME){
			if(math_dist2(touch->total_dx,touch->total_dy) < MAX_MOVE_INHERBITION_DIST * MAX_MOVE_INHERBITION_DIST){
				goto end;
			}else{
				handler->move_guard.pretap_settle = SETTLE_CLEARED;
			}
		}

		/** common inherbition */
		/** 
		 * 1. (X removed)Post tap move 
		*/
		if( time_diff_ms(&touch->update_time,&handler->move_guard.move_inherbition_time) <= 0 ){
			LOG_DEBUG("common inherbition\n");
			goto end;
		}

		if(touch_count == 1){
 			
			/** micro move when physical button pressing should be ignored */
			if(handler->last_physical_button > 0){
				/** 
				 * todo://
				 * if pressing the trackpad and finger gets up.
				 * there will be some move events triggered before touch relase.
				 * so after button down,it should cache some moves to debounce that move events.
				*/
				if(handler->move_guard.physical_button_settle == SETTLE_NONE){
					/** from no physical button pressed to pressed state */
					handler->move_guard.physical_button_settle = SETTLE_SETTLE;
					handler->tap.tap_disabled = TRUE;
				}

				if( handler->move_guard.physical_button_settle == SETTLE_SETTLE ){
					if( math_dist2( touch->dx,touch->dy ) < PHYSICAL_BUTTON_MICRO_MOVE_THRESHOLD * PHYSICAL_BUTTON_MICRO_MOVE_THRESHOLD){
						LOG_DEBUG("button down micro move guard\n");
						goto end;
					}else{
						/** once big movement checked out,disable micro move guard */
						handler->move_guard.physical_button_settle = SETTLE_CLEARED;
					}
				}

			}else{
				/** from physical button pressed to idle state */
				handler->move_guard.physical_button_settle = SETTLE_NONE;
			}

			
			// if( props->is_absolute_mode ){
			
			// 	staged->pointer.move_type = ABSOLUTE;
			// 	staged->pointer.x = touch -> x + get_cap_xmid(&props->caps);
			// 	staged->pointer.y = touch -> y + get_cap_ymid(&props->caps);
			
			// }else{
			
				staged->pointer.move_type = RELATIVE;
				staged->pointer.x = touch -> dx;
				staged->pointer.y = touch -> dy;

			// }

		}else if(touch_count == 2){
			
			// final algorithm 
			// todo://
			// 1 find out whether is vertical or horizontal (axsi +- 10 (tbd) degree)
			// 2 once out of this these area,scroll became freedom

			// use freedom way first

			// todo:// how to ensure its a scroll,not a pitch or rotate?
			
			/** two finger move will calls this function twice */
			staged->scroll.holding = TRUE;
			
			int64_t diff_ms = time_diff_ms(&touch->update_time,&touch->last_update_time);

			touchlist_double_avg(&staged->scroll.velocity_x,1000.0 * touch -> dx / diff_ms * SCROLL_INERTIA_MULTIPLIER,touch_idx,touch_count);
			touchlist_double_avg(&staged->scroll.velocity_y,1000.0 * touch -> dy / diff_ms * SCROLL_INERTIA_MULTIPLIER,touch_idx,touch_count);
		
			touchlist_double_avg(&staged->scroll.x,(double)touch -> dx,touch_idx,touch_count);
			touchlist_double_avg(&staged->scroll.y,(double)touch -> dy,touch_idx,touch_count);

			LOG_SCROLL("scroll para,offset = (%.2lf,%.2lf) , velocity = (%.2lf,%.2lf) diff_ms=%ld\n",staged->scroll.x,staged->scroll.y,staged->scroll.velocity_x,staged->scroll.velocity_y,diff_ms);

		}else if( touch_count == 3 ){

			handler->state.special_type = SpecialTyoe_Swipe3;
			timercp(&handler->state.start_time   , &touch->update_time);
			staged->button.down =  1 << SWIPE3_BUTTON;

			staged->pointer.move_type = RELATIVE;
			touchlist_int_avg(&staged->pointer.x,touch->dx,touch_idx,touch_count);
			touchlist_int_avg(&staged->pointer.y,touch->dy,touch_idx,touch_count);
			
		}
	}
 end:
 	return ;
}

void on_touch_release(touch_handler_t *handler,const struct Touch *touch,int touch_count,const struct Touch *touchlist,int touchbit,struct itrack_staged_status_s *staged,const struct itrack_props_s *props){
	LOG_DEBUG("on_touch_release count = %d\n",touch_count);
	if(props->cfg.api == 2){
		// ,props
		guesture_manager_touch_end(&handler->guesture_manager,touch,touch_count,touchlist,touchbit,staged);
		return ;
	}
	// guesture_manager_touch_end(&handler->)
	if( handler->state.special_type != SpecialTyoe_None ){
		if(touch_count == 1){
			if(handler->state.special_type == SpecialTyoe_Swipe3){
				
				timercp(&handler->state.pseudo_endtime, &touch->update_time);

				staged->button.defer_up.operation = DEFER_NEW;
				staged->button.defer_up.button    = 1 << SWIPE3_BUTTON;

				struct timeval tv;
				timeraddms(&touch->update_time,SWIPE3_HOLD_TIME,&tv);
				staged->button.defer_up.time 	  = tv;
				staged->button.defer_up.callback      = defer_callback_set_value;
				struct defer_callback_set_value_t *param = (void *) staged->button.defer_up.callback_data;
				param->ptr   = &handler->state.special_type;
				*(Bool *)param->value = SpecialTyoe_None;
				param->size = sizeof(Bool);
			}
		}
	}else{
		
		// sum each touch movement
		handler->tap.sum_x += abs(touch->total_dx);
		handler->tap.sum_y += abs(touch->total_dy);

		/** last finger leaves */
		if(touch_count == 1){
			/** try match a tap*/
			if( handler->tap.tap_disabled == FALSE ){
				LOG_DEBUG("max_touches_recorded = %d\n",handler->max_touches_recorded);
				
				/** should have within a certain amount of time */
				int64_t diff_ms = time_diff_ms(&touch->update_time,&touch->create_time);
				if( diff_ms < TAP_TIME_MAX_HOLD_TIME){
					LOG_DEBUG("TAP %d time is OK diff(%ld),sum_offset = (%d,%d)\n",handler->max_touches_recorded,diff_ms,handler->tap.sum_x,handler->tap.sum_y);
					/** should move a little */
					uint64_t dist2 = math_dist2(handler->tap.sum_x / handler->max_touches_recorded,handler->tap.sum_y / handler->max_touches_recorded);
					if( dist2 < TAP_MOVE_DIST2){
						LOG_DEBUG("TAP %d Recognized\n",handler->max_touches_recorded);
						int tap_button = -1;
						
						if( handler->max_touches_recorded == 1 ){
							tap_button = TAP_1_BUTTON;
						}else if( handler->max_touches_recorded == 2 ){
							tap_button = TAP_2_BUTTON;
						}else if( handler->max_touches_recorded == 3 ){
							tap_button = TAP_3_BUTTON;
						}else if( handler->max_touches_recorded == 4 ){
							tap_button = TAP_4_BUTTON;
						}else{
							tap_button = -1;
						}

						if( tap_button >= 0 ){
							/** it's a tap */
							staged->button.down = (1 << tap_button);
							staged->button.defer_up.operation = DEFER_NEW;
							staged->button.defer_up.button    = (1 << tap_button);
							struct timeval tv;
							timeraddms(&touch->update_time,TAP_UP_TIME,&tv);
							staged->button.defer_up.time      = tv;

							// struct
							// todo://to compare, handler->move_guard.move_inherbition_time and touch -> update_time,TAP_POST_MOVE_INHERBITION_TIME, which is newer
							// timeraddms(&touch -> update_time, TAP_POST_MOVE_INHERBITION_TIME, &handler->move_guard.move_inherbition_time);
						}
					}
				}
			}
			handler->start = FALSE;

		}else if(touch_count == 2){
			
			/** scroll to move */
			staged->scroll.holding = FALSE;

		}else if(touch_count == 3){
			
		}
	}
	return ;
}

void on_touch_invalid(touch_handler_t *handler,const struct Touch *touch,int touch_count,const struct Touch *touchlist,int touchbit,struct itrack_staged_status_s *staged,const struct itrack_props_s *props){
	LOG_DEBUG("on_touch_invalid count = %d\n",touch_count);
	return on_touch_release(handler,touch,touch_count,touchlist,touchbit,staged,props);
}

void on_physical_button_update(touch_handler_t *handler,const struct timeval *evtime,int physical_button_state,struct itrack_staged_status_s *staged){
	staged->physical_button = physical_button_state;
	if(&handler->guesture_manager){
		handler->guesture_manager.physical_button = physical_button_state;
	}
	handler->last_physical_button = physical_button_state;
}

void touch_handler_set_post_scrolling_state(touch_handler_t *handler,Bool on,const struct timeval *time,const struct itrack_props_s *props){
	LOG_DEBUG("touch_handler_set_post_scrolling_state on=%s time=(%ld,%ld)\n",on?"ON":"OFF",time->tv_sec,time->tv_usec);
	if(props->cfg.api == 2){
		struct guesture_item_s *item = guesture_manager_find_item_by_name(&handler->guesture_manager,"scroll");
		assert(item);
		if( on ){
			guesture_manager_set_alt(&handler->guesture_manager,item);
		}else{
			guesture_manager_clear_alt(&handler->guesture_manager,item);
		}
	}else{
		handler->scroll_state.is_inertia_sliding = on;
		if( on == FALSE){
			timercp(&handler->scroll_state.last_off_time , time);
		}
	}
}