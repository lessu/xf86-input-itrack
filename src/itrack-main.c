#include <xorg/input.h>
#include "itrack-main.h"
#include "debug.h"
#include "touch-handler.h"

static bool is_touch_needs_to_ignore(const struct Touch *touch){
	if( GETBIT(touch->flags, MT_THUMB) || GETBIT(touch->flags, MT_PALM)
		//  ||  GETBIT(touch->flags, MT_EDGE) 
	){
		return TRUE;
	}else{
		return FALSE;
	}
}

static void on_post_stage_inertia_scroll_state_change(const struct post_stage_s *handler,bool state,void* userdata){
	itrack_t *itrack = userdata;
	struct timeval time;
 	gettimeofday(&time, NULL);
	touch_handler_set_post_scrolling_state(&itrack->private.touch,state,&time,&itrack->props);
}

int itrack_open(itrack_t *itrack, int fd){
    int ret;
	itrack->fd = fd;
	ret = mtdev_open(&itrack->dev, itrack->fd);
	if (ret){
		LOG_ERROR("mtdev_open error");
		goto error;
	}
	hwstate_init(&itrack->hs , &itrack->props.caps);
	mtstate_init(&itrack->status.state);
	touch_handler_init(&itrack->private.touch,&itrack->private.post_stage_handler,&itrack->props);
	itrack_post_init(&itrack->private.post_stage_handler);
	itrack_post_set_on_inertia_scroll_state_change_callback(&itrack->private.post_stage_handler,on_post_stage_inertia_scroll_state_change,itrack);
	return 0;
 error:
	return ret;
}

int itrack_close(itrack_t *itrack){
	mtdev_close(&itrack->dev);
	itrack_post_deinit(&itrack->private.post_stage_handler);
	touch_handler_deinit(&itrack->private.touch);
	return 0;
}

bool itrack_read(itrack_t *itrack){
	/** todo:// */
	// struct itrack_action_s *action;
	
    int ret = hwstate_modify(&itrack->hs, &itrack->dev, itrack->fd, &itrack->props.caps);
	if (ret <= 0)
		return 0;
	mtstate_extract(&itrack->status.state, &itrack->props.cfg, &itrack->hs, &itrack->props.caps);
	
	log_touches(itrack->status.state.touch ,itrack->status.state.touch_used);

	// physical button
	// static int last_button = 0;
	// for(int i = 0; i < 32 ; i ++){
	// 	if( GETBIT(itrack->hs.button,i) != GETBIT(last_button,i) ){
	// 		if( GETBIT(itrack->hs.button,i) ){
	// 			//down
	// 			action->button.down
	// 		}else{
	// 			//up
	// 		}
	// 	}
	// }
	// last_button = itrack->hs.button;
	on_physical_button_update(&itrack->private.touch,&itrack->hs.evtime,itrack->hs.button);

	int i = 0;
	int simulated_used = itrack->status.state.touch_used;
	foreach_bit(i, itrack->status.state.touch_used){
		if( !is_touch_needs_to_ignore( itrack->status.state.touch + i) ){
			if( GETBIT(itrack->status.state.touch[i].flags,MT_NEW) ){
				CLEARBIT(simulated_used,i);
			}
		}
	}
	foreach_bit(i,itrack->status.state.touch_used){
		const struct Touch *item = itrack->status.state.touch + i;
		if(is_touch_needs_to_ignore(item)){
			#if 0 /** deal with in event handlers */
			// if( GETBIT(item->flags, MT_THUMB) ){
			// }
			// if( GETBIT(item->flags, MT_PALM) ){
			// }
			// if( GETBIT(item->flags, MT_EDGE) ){
			// }
			#endif
		}else{
			/** this callback will read action physical button setting make sure,action physical button have been set at this moment*/
			if( GETBIT(item->flags, MT_NEW) ){
				int j;
				int count = 0;
				SETBIT(simulated_used,i);
				foreach_bit(j, simulated_used){
					count ++;
				}
				on_touch_start(&itrack->private.touch,item,count,itrack->status.state.touch,itrack->status.state.touch_used,&itrack->props);
				
			}else if( GETBIT(item->flags, MT_RELEASED) ){
				int j;
				int count = 0;
				foreach_bit(j, simulated_used){
					count ++;
				}
				on_touch_release(&itrack->private.touch,item,count,itrack->status.state.touch,itrack->status.state.touch_used,&itrack->props);
				CLEARBIT(simulated_used,i);
				
			}else if( GETBIT(item->flags, MT_INVALID) ){
				int j;
				int count = 0;
				foreach_bit(j, simulated_used){
					count ++;
				}
				on_touch_invalid(&itrack->private.touch,item,count,itrack->status.state.touch,itrack->status.state.touch_used,&itrack->props);
				CLEARBIT(simulated_used,i);
			}else{
				/** move update events delayed calling ,make sure move callback was called after all touch new/cancel event done*/
			}
			
			#if 0 /** not done from mtouch */
			if( GETBIT(item->flags, MT_TAP) ){}
			if( GETBIT(item->flags, MT_BUTTON) ){}
			#endif
		}
	}

	foreach_bit(i,simulated_used){
		const struct Touch *item = itrack->status.state.touch + i;
		
		if(is_touch_needs_to_ignore(item)){

		}else{
			/** this callback will read action physical button setting make sure,action physical button have been set at this moment*/
			if( GETBIT(item->flags, MT_NEW) ){
			}else if( GETBIT(item->flags, MT_RELEASED) ){
			}else if( GETBIT(item->flags, MT_INVALID) ){
			}else{
				int j;
				int count = 0;
				int touch_idx = 0;
				foreach_bit(j, simulated_used){
					if(j == i){
						touch_idx = count;
					}
					count ++;
				}
				on_touch_move(&itrack->private.touch,item,touch_idx,count,itrack->status.state.touch,itrack->status.state.touch_used,&itrack->props);
			}
		}
	}
	return 1;
}
