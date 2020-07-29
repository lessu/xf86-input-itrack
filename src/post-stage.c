#include "post-stage.h"
#include <string.h>
static double scroll_distance_fn(int velocity,int current_time,int total_time);
static void scroll_by(DeviceIntPtr device_ptr,double x,double y);
static CARD32 on_scroll_inertia_sliding_timer_update(OsTimerPtr timer,CARD32 _time,void *_arg){
	struct post_stage_s *handler = _arg;
	Bool end = FALSE;
	LOG_DEBUG("on_scroll_inertia_sliding_timer_update interval=%d\n",handler->scroll.arg.interval);
	if( handler->scroll.passed_time_ms + handler->scroll.arg.interval < handler->scroll.arg.y.duration){
		double diff_pos = 
				scroll_distance_fn(handler->scroll.arg.y.start_velocity ,handler->scroll.passed_time_ms + handler->scroll.arg.interval , handler->scroll.arg.y.duration) 
			  - scroll_distance_fn(handler->scroll.arg.y.start_velocity ,handler->scroll.passed_time_ms 						       , handler->scroll.arg.y.duration);
		LOG_DEBUG("scroll1 diff_pos=%d\n",(int)diff_pos);
		scroll_by(handler->scroll.arg.device_ptr,0,diff_pos);
	}else if(handler->scroll.passed_time_ms <  handler->scroll.arg.y.duration){
		double diff_pos = 
				scroll_distance_fn(handler->scroll.arg.y.start_velocity , handler->scroll.arg.y.duration							   ,handler->scroll.arg.y.duration) 
			  - scroll_distance_fn(handler->scroll.arg.y.start_velocity , handler->scroll.passed_time_ms							   , handler->scroll.arg.y.duration);
		LOG_DEBUG("scroll2 diff_pos=%d\n",(int)diff_pos);
		scroll_by(handler->scroll.arg.device_ptr,0,diff_pos);
		end = TRUE;
	}else{
		end = TRUE;
	}
	handler->scroll.passed_time_ms += handler->scroll.arg.interval;

	if(end){
		handler->scroll.is_inertia_scrolling = FALSE;
		if(handler->scroll.state_change_fn){
			handler->scroll.state_change_fn(handler,handler->scroll.is_inertia_scrolling,handler->scroll.state_change_fn_userdata);
		}
		return 0;
	}else{
		return handler->scroll.arg.interval;
	}
}
static double scroll_distance_fn(int velocity,int current_time,int total_time){
	/** linear implementation */
	return (velocity + (1.0f - 1.0f * current_time / total_time) * velocity ) * current_time / 2 / ONE_SECOND_MS;
}
static int scroll_duration_ms_calc(int velocity){
	/** linear implementation */
	return ABSVAL(ONE_SECOND_MS * velocity / SCROLL_FRICTION);
}

/** if the return value not equal to 0, a new timer will be set and won't stop it */
static CARD32 on_button_defer_callback(OsTimerPtr timer,CARD32 time, void *_arg){
	LOG_DEBUG("on_button_defer_callback\n");
	struct post_stage_s *handler = _arg;
	LOG_DEBUG("defer up button %d\n",handler->defer_arg.button);

	for (int i = 0; i < MAX_BUTTON_NUMBER; i++) {
		if( GETBIT(handler->defer_arg.button , i) ){
			xf86PostButtonEvent(handler->defer_arg.device_ptr, FALSE, i + 1, 0, 0, 0);
			LOG_DEBUG("button %d up\n", i);
		}
	}
	if(handler->defer_arg.callback){
		handler->defer_arg.callback(&handler->defer_arg.callback_data);
	}
	handler->is_defering = FALSE;
	return 0;
}

static void scroll_by(DeviceIntPtr device_ptr,double x,double y){
	ValuatorMask* mask = valuator_mask_new(4);
	valuator_mask_zero(mask);

	// handler->scroll.sum_x += staged_status->scroll.x;
	// handler->scroll.sum_y += staged_status->scroll.y;

	valuator_mask_set_double(mask, 0, (double)0);
	valuator_mask_set_double(mask, 1, (double)0);
	// valuator_mask_set_double(mask, 2, handler->scroll.sum_x / STEP_DISTANCE_NEEDS);
	// valuator_mask_set_double(mask, 3, handler->scroll.sum_y / STEP_DISTANCE_NEEDS);

	valuator_mask_set_double(mask, 2, (NATURE_SCROLL?-1:1) * y / STEP_DISTANCE_NEEDS);
	valuator_mask_set_double(mask, 3, (NATURE_SCROLL?-1:1) * x / STEP_DISTANCE_NEEDS);
	
	xf86PostMotionEventM(device_ptr, 
								Relative /** absolute */, 
								mask
						);

	// handler->scroll.sum_x = 0;//MODVAL(handler->scroll.sum_x , STEP_DISTANCE_NEEDS);
	// handler->scroll.sum_y = 0;//MODVAL(handler->scroll.sum_y , STEP_DISTANCE_NEEDS);
}
void itrack_post_init(struct post_stage_s *handler){
    handler->is_defering = FALSE;
    handler->defer_timer = NULL;
	handler->last_button_status = 0;
	handler->scroll.inertia_sliding_timer = 0;
	handler->scroll.is_inertia_scrolling = FALSE;
}

void itrack_post_deinit(struct post_stage_s *handler){
    if(handler->defer_timer){
		TimerFree(handler->defer_timer);
		handler->defer_timer = NULL;
        handler->is_defering = FALSE;
	}
}

void itrack_post_read(struct post_stage_s *handler,DeviceIntPtr device_ptr, const struct timeval *tv,const struct itrack_staged_status_s __nonull *staged_status){
	
	Bool pointer_move = staged_status->pointer.x !=0 || staged_status->pointer.y != 0;

	// scroll
	if(staged_status->scroll.holding == TRUE){
		/** cancel sliding */
		if( handler->scroll.is_inertia_scrolling ){
			TimerCancel(handler->scroll.inertia_sliding_timer);
			handler->scroll.is_inertia_scrolling = FALSE;
			if(handler->scroll.state_change_fn){
				handler->scroll.state_change_fn(handler,handler->scroll.is_inertia_scrolling,handler->scroll.state_change_fn_userdata);
			}
		}
		scroll_by(device_ptr,staged_status->scroll.x,staged_status->scroll.y);
		handler->scroll.velocity_x = staged_status->scroll.velocity_x;
		handler->scroll.velocity_y = staged_status->scroll.velocity_y;
	}else{
		
        // handler->scroll.sum_x = 0;
        // handler->scroll.sum_y = 0;
		if(ABSVAL(handler->scroll.velocity_x) > SCROLL_INERTIA_SPEED_TRIGER || ABSVAL(handler->scroll.velocity_y) > SCROLL_INERTIA_SPEED_TRIGER ){
			LOG_DEBUG("inertia scroll velocity (%.2lf,%.2lf)\n",staged_status->scroll.velocity_x,staged_status->scroll.velocity_y);	
			/** cancel sliding */
			// TimerSet(handler->scroll.inertia_sliding_timer);
			handler->scroll.is_inertia_scrolling   = TRUE;
			if(handler->scroll.state_change_fn){
				handler->scroll.state_change_fn(handler,handler->scroll.is_inertia_scrolling,handler->scroll.state_change_fn_userdata);
			}
			handler->scroll.passed_time_ms       = 0;
			handler->scroll.arg.device_ptr	     = device_ptr;
			handler->scroll.arg.interval 		 = ONE_SECOND_MS / SCROLL_INERTIA_POST_RATE;
			handler->scroll.arg.x.start_velocity = handler->scroll.velocity_x;
			handler->scroll.arg.x.duration       = scroll_duration_ms_calc(handler->scroll.velocity_x);

			handler->scroll.arg.y.start_velocity = handler->scroll.velocity_y;
			handler->scroll.arg.y.duration       = scroll_duration_ms_calc(handler->scroll.velocity_y);
			LOG_DEBUG("start inertia timer duration (%d,%d) \n",handler->scroll.arg.x.duration,handler->scroll.arg.y.duration);
			handler->scroll.inertia_sliding_timer = TimerSet(handler->scroll.inertia_sliding_timer , 0 , handler->scroll.arg.interval , on_scroll_inertia_sliding_timer_update,handler);

		}else{
			if( pointer_move ){
				if( handler->scroll.is_inertia_scrolling ){
					TimerCancel(handler->scroll.inertia_sliding_timer);
					handler->scroll.is_inertia_scrolling = FALSE;
					if(handler->scroll.state_change_fn){
						handler->scroll.state_change_fn(handler,handler->scroll.is_inertia_scrolling,handler->scroll.state_change_fn_userdata);
					}
				}
			}
		}
		handler->scroll.velocity_x = handler->scroll.velocity_y = 0;
    }

	// todo :// Aux with gesture button behavior
	for (int i = 0; i < MAX_BUTTON_NUMBER; i++) {
		if( GETBIT(staged_status->physical_button , i) != GETBIT(handler->last_button_status , i) ){
			if(GETBIT(staged_status->physical_button , i)){
				xf86PostButtonEvent(device_ptr, FALSE, i + 1, 1, 0, 0);
				// LOG_DEBUG("button %d down\n", i);
			}else{
				xf86PostButtonEvent(device_ptr, FALSE, i + 1, 0, 0, 0);
				// LOG_DEBUG("button %d up\n", i);
			}
		}
	}
	handler->last_button_status = staged_status->physical_button;
	
	for (int i = 0; i < MAX_BUTTON_NUMBER; i++) {
		if( GETBIT(staged_status->button.down , i) ){
			xf86PostButtonEvent(device_ptr, FALSE, i + 1, 1, 0, 0);
			// LOG_DEBUG("button %d down\n", i);
		}
		if( GETBIT(staged_status->button.up , i) ){
			xf86PostButtonEvent(device_ptr, FALSE, i + 1, 0, 0, 0);
			// LOG_DEBUG("button %d up\n", i);
		}
	}

	if(staged_status->button.defer_up.operation != DEFER_NONE){		
		if(staged_status->button.defer_up.operation == DEFER_NEW){
			LOG_DEBUG("defer button %d up at [%ld,%06ld]\n", staged_status->button.defer_up.button,staged_status->button.defer_up.time.tv_sec,staged_status->button.defer_up.time.tv_usec);
			if( handler->is_defering ){
				LOG_WARNING("defer button up conflict,stop previous\n");
			}
			struct timeval timeout_tv;
			timersub(&staged_status->button.defer_up.time,tv,&timeout_tv);
			mstime_t timeout = timertoms(&timeout_tv);
			
			handler->defer_arg.device_ptr = device_ptr;
			handler->defer_arg.callback = staged_status->button.defer_up.callback;
			memcpy(handler->defer_arg.callback_data,staged_status->button.defer_up.callback_data,sizeof(handler->defer_arg.callback_data));
			handler->defer_arg.button   = staged_status->button.defer_up.button;

			/** flag TimerForceOld = call TimerForce() on old one if old one is pending */
			handler->defer_timer = TimerSet(handler->defer_timer,TimerForceOld , timeout , on_button_defer_callback,handler);

			handler->is_defering = TRUE;

		}else if(staged_status->button.defer_up.operation == DEFER_CANCEL){
			
			if( handler-> is_defering){
				LOG_DEBUG("defer button Cancel\n");
				TimerCancel(handler->defer_timer);
				handler->is_defering = FALSE;
			}
		}else if(staged_status->button.defer_up.operation == DEFER_TRIGGER_IMMEDIATEY){
			
			if( handler->is_defering ){
				LOG_DEBUG("defer button trigger\n");
				TimerForce(handler->defer_timer);
				TimerCancel(handler->defer_timer);
				/** handler->is_defering will be set to False in callback */
			}
		}else if(staged_status->button.defer_up.operation == DEFER_DELAY){
			if( handler->is_defering ){
				LOG_DEBUG("defer button delay to [%ld,%06ld]\n",staged_status->button.defer_up.time.tv_sec,staged_status->button.defer_up.time.tv_usec);
				struct timeval timeout_tv;
				timersub(&staged_status->button.defer_up.time,tv,&timeout_tv);
				mstime_t timeout = timertoms(&timeout_tv);
				handler->defer_timer = TimerSet(handler->defer_timer,0,timeout,on_button_defer_callback,handler);
			}
		}
	}

	// if(staged_status->pointer.move_type == RELATIVE){
		/* 
		 * Give the HW coordinates to Xserver as absolute coordinates, these coordinates
		 * are not scaled, this is oke if the touchscreen has the same resolution as the display.
		 */
		xf86PostMotionEvent(device_ptr, 
									Relative /** absolute */, 
									0 /** start valuator */, 
									2 /** valuator count */,
									/** valuators */
									staged_status->pointer.x, staged_status->pointer.y
							);
	// }else if(staged_status->pointer.move_type == ABSOLUTE){
	// 	/* 
	// 	 * Give the HW coordinates to Xserver as absolute coordinates, these coordinates
	// 	 * are not scaled, this is oke if the touchscreen has the same resolution as the display.
	// 	 */
	// 	xf86PostMotionEvent(device_ptr, 
	// 								Absolute /** absolute */, 
	// 								0 /** start valuator */, 
	// 								2 /** valuator count */,
	// 								/** valuators */
	// 								staged_status->pointer.x, staged_status->pointer.y
	// 						);
	// }
}
void itrack_post_set_on_inertia_scroll_state_change_callback(struct post_stage_s *handler,ITrackInertiaScrollStateChangeFn fn,void *userdata){
	handler->scroll.state_change_fn = fn;
	handler->scroll.state_change_fn_userdata = userdata;
}