#include "simu_test.h"
#include "itrack-type.h"
#include "trig.h"
#include <stdio.h>
#include <unistd.h>
#include "guesture/move.h"
#include "guesture/tap.h"
static void print_staged(const struct itrack_staged_status_s *out_staged_status){
	char log[2048] = {0};
	Bool have_change = FALSE;
	sprintf(log,"staged={\n");
	if(out_staged_status->pointer.x != 0 || out_staged_status->pointer.y != 0){
		sprintf(log+strlen(log),"    pointer=(%d,%d)\n",out_staged_status->pointer.x,out_staged_status->pointer.y);
		have_change = TRUE;
	}
	if(out_staged_status->button.down !=0 || out_staged_status->button.defer_up.operation != 0){
		 const char DEFER_TYPE_REPR_MAP[32][32] = {
			"DEFER_NONE",
			"DEFER_NEW",
			"DEFER_DELAY",
			"DEFER_CANCEL",
			"DEFER_TRIGGER_IMMEDIATEY"
		};
		sprintf(log+strlen(log),"    button=(%d,%d) defer(op=%s,up=%d,at=[%ld,%ld])\n",
			out_staged_status->button.down,
			out_staged_status->button.up,
			DEFER_TYPE_REPR_MAP[out_staged_status->button.defer_up.operation],
			out_staged_status->button.defer_up.button,
			out_staged_status->button.defer_up.time.tv_sec,
			out_staged_status->button.defer_up.time.tv_usec
		);
		have_change = TRUE;
	}
	sprintf(log+strlen(log),"}\n");
	if(have_change){
		LOG_DEBUG("%s",log);
	}
}
static void dispatch_event(struct guesture_manager_s *manager,struct multi_touch_s *multi_touch,struct itrack_staged_status_s *out_staged_status);
static Bool is_touch_needs_to_ignore(const struct Touch *touch){
	if( GETBIT(touch->flags, MT_THUMB) || GETBIT(touch->flags, MT_PALM)
		//  ||  GETBIT(touch->flags, MT_EDGE) 
	){
		return TRUE;
	}else{
		return FALSE;
	}
}

void send_touch_sequence(struct guesture_manager_s *manager,struct multi_touch_s *multi_touch,int count){
    uint64_t max_offset_unit = 0;
    for(int i = 0 ; i < count ;i ++){
        if( multi_touch[i].offset_unit > max_offset_unit){
            max_offset_unit =  multi_touch[i].offset_unit;
        }
    }
	
    for(int i = 0 ; i <= max_offset_unit; i ++){
		struct itrack_staged_status_s out_staged_status = {0};
        for(int multi_touch_i = 0 ; multi_touch_i < count ;multi_touch_i ++){
            if( multi_touch[multi_touch_i].offset_unit == i){
				for(int touch_i = 0 ; touch_i < multi_touch[multi_touch_i].touch_count ;touch_i ++){
					multi_touch[multi_touch_i].touches[touch_i].last_update_time = multi_touch[multi_touch_i].touches[touch_i].update_time;
					gettimeofday(&multi_touch[multi_touch_i].touches[touch_i].update_time,NULL);
				}
                dispatch_event(manager,multi_touch + i,&out_staged_status);
				print_staged(&out_staged_status);
				usleep(11000);
				break;
            }
        }
    }
}

static void dispatch_event(struct guesture_manager_s *manager,struct multi_touch_s *multi_touch,struct itrack_staged_status_s *out_staged_status){
    
    // on_physical_button_update(&itrack->private.touch,&itrack->hs.evtime,itrack->hs.button,out_staged_status);

	int i = 0;
	
    bitmask_t touch_used = 0;
    for(i = 0 ; i < multi_touch->touch_count;i++){
        touch_used = (touch_used << 1) | 1;
    }
	int simulated_used = touch_used;
	foreach_bit(i, touch_used){
		if( !is_touch_needs_to_ignore( multi_touch -> touches + i) ){
			if( GETBIT(multi_touch -> touches[i].flags,MT_NEW) ){
				CLEARBIT(simulated_used,i);
			}
		}
	}
	// foreach_bit(i, itrack->status.state.touch_used){
	// 	if( !is_touch_needs_to_ignore( itrack->status.state.touch + i) ){
	// 		if( GETBIT(itrack->status.state.touch[i].flags,MT_NEW) ){
	// 			CLEARBIT(simulated_used,i);
	// 		}
	// 	}
	// }
	foreach_bit(i,touch_used){
		const struct Touch *item = multi_touch -> touches + i;
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
			/** this callback will read out_staged_status physical button setting make sure,out_staged_status physical button have been set at this moment*/
			if( GETBIT(item->flags, MT_NEW) ){
				int j;
				int count = 0;
				SETBIT(simulated_used,i);
				foreach_bit(j, simulated_used){
					count ++;
				}
                guesture_manager_touch_start(manager,item,count,multi_touch->touches,touch_used,out_staged_status);
			}else if( GETBIT(item->flags, MT_RELEASED) ){
				int j;
				int count = 0;
				foreach_bit(j, simulated_used){
					count ++;
				}
                guesture_manager_touch_end(manager,item,count,multi_touch->touches,touch_used,out_staged_status);
				CLEARBIT(simulated_used,i);
				
			}else if( GETBIT(item->flags, MT_INVALID) ){
				int j;
				int count = 0;
				foreach_bit(j, simulated_used){
					count ++;
				}
                // on_touch_invalid
				// on_touch_invalid(&itrack->private.touch,item,count,itrack->status.state.touch,itrack->status.state.touch_used,out_staged_status,&itrack->props);
                guesture_manager_touch_end(manager,item,count,multi_touch->touches,touch_used,out_staged_status);
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
		const struct Touch *item = multi_touch -> touches + i;
		
		if(is_touch_needs_to_ignore(item)){

		}else{
			/** this callback will read out_staged_status physical button setting make sure,out_staged_status physical button have been set at this moment*/
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
                guesture_manager_touch_update(manager,item,touch_idx,count,multi_touch->touches,touch_used,out_staged_status);
				// on_touch_move(&itrack->private.touch,item,touch_idx,count,itrack->status.state.touch,itrack->status.state.touch_used,out_staged_status,&itrack->props);
			}
		}
	}
}

void simu_test_guesture_manager_init(struct guesture_manager_s *manager){
    struct move_guesture_s *move_guesture = malloc(sizeof(struct move_guesture_s));
	struct tap_guesture_s  *tap1_guesture = malloc(sizeof(struct tap_guesture_s));
    guesture_manager_init(manager);

    move_guesture_init(move_guesture);
    tap_guesture_init(tap1_guesture,1);
	
	guesture_manager_add(manager,&move_guesture->guesture,1);
	guesture_manager_add(manager,&tap1_guesture->guesture,16);
}

void simu_test_guesture_manager_deinit(struct guesture_manager_s *manager){
	for(int i = 0; i < MAX_GUESTURE_COUNT; i ++ ){
        if( manager->guesture_list[i].used == USING ){
            free(manager->guesture_list[i].guesture);
        }
    }
}

int touch_create_new(struct Touch *touch,int x,int y,int seq_num){
	static int touch_id = 0;
	touch->flags = 1 << MT_NEW;
	touch->direction = TR_NONE;
	touch->tracking_id = touch_id;
	touch->x = x;
	touch->y = y;
	touch->dx = 0;
	touch->dy = 0;
	touch->total_dx = 0;
	touch->total_dy = 0;
	gettimeofday(&touch->create_time,NULL);
	touch->update_time 			= touch->create_time;
	touch->last_update_time 	= touch->create_time;
	return touch_id;
}

void touch_update_no_change(struct Touch *touch){
	touch->flags = 0;
	touch->direction = TR_NONE;
	touch->dx = 0;
	touch->dy = 0;
	// touch->last_update_time 	= touch->update_time;
	// touch->update_time.tv_sec  += 11;
}

void touch_update_offset(struct Touch *touch,int dx,int dy){
	touch->flags = 0;
	touch->x += dx;
	touch->y += dy;
	touch->dx = dx;
	touch->dy = dx;
	touch->total_dx += dx;
	touch->total_dy += dy;
	// touch->last_update_time 	= touch->update_time;
	// touch->update_time.tv_sec  += 11;
}

void touch_update_release(struct Touch *touch){
	touch->flags = 1 << MT_RELEASED;
	touch->direction = TR_NONE;
	touch->dx = 0;
	touch->dy = 0;
	// touch->last_update_time 	= touch->update_time;
	// touch->update_time.tv_sec  += 11;
}

#define MAX_TOUCH_SEQUENCE 50
void touch_sequence_builder_init(struct touch_sequence_builder_s *builder){
	builder->sequence 		= malloc(sizeof(struct multi_touch_s) * MAX_TOUCH_SEQUENCE);
	bzero(builder->sequence,sizeof(struct multi_touch_s) * MAX_TOUCH_SEQUENCE);
	builder->seqence_length = 0;
	builder->current_seq_num = 0;

	builder->sequence[builder->seqence_length].offset_unit = 0;
	builder->sequence[builder->seqence_length].touch_count = 0;
}
void touch_sequence_builder_add(struct touch_sequence_builder_s *builder,const struct Touch *touch){
	builder->sequence[builder->seqence_length].touches[builder->sequence[builder->seqence_length].touch_count] = *touch;
	builder->sequence[builder->seqence_length].touch_count ++;
}
void touch_sequence_builder_next(struct touch_sequence_builder_s *builder){
	builder->current_seq_num++;
	if(builder->sequence[builder->seqence_length].touch_count != 0){
		builder->seqence_length ++;
		builder->sequence[builder->seqence_length].offset_unit = builder->current_seq_num;
		builder->sequence[builder->seqence_length].touch_count = 0;
	}
}
void touch_sequence_builder_add_and_next(struct touch_sequence_builder_s *builder,const struct Touch *touch){
	touch_sequence_builder_add(builder,touch);
	touch_sequence_builder_next(builder);
}

void touch_sequence_builder_release(struct touch_sequence_builder_s *builder){
	free(builder->sequence);
	builder->sequence = NULL;
}