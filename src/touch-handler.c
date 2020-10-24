#include "touch-handler.h"
#include "itrack-main.h"
#include <stdint.h>
#include "guesture/guesture_manager.h"
#include "guesture/tap.h"
#include "guesture/move.h"
#include "guesture/scroll.h"
#include "guesture/drag.h"
#include "guesture/pinch.h"
struct defer_callback_set_value_t {
	void *ptr;
	uint8_t value[8];
	size_t size;
};

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
void touch_handler_init(touch_handler_t *handler,struct post_stage_s *post_stage,const struct itrack_props_s *props){
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

	guesture_manager_init(manager,post_stage);
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

void on_touch_start(touch_handler_t *handler,const struct Touch *touch,int touch_count,const struct Touch *touchlist,int touchbit,const struct itrack_props_s *props){
	guesture_manager_touch_start(&handler->guesture_manager,touch,touch_count,touchlist,touchbit);
}

void on_touch_move(touch_handler_t *handler,const struct Touch *touch,int touch_idx,int touch_count,const struct Touch *touchlist,int touchbit,const struct itrack_props_s *props){
	guesture_manager_touch_update(&handler->guesture_manager,touch,touch_idx,touch_count,touchlist,touchbit);
}

void on_touch_release(touch_handler_t *handler,const struct Touch *touch,int touch_count,const struct Touch *touchlist,int touchbit,const struct itrack_props_s *props){
	guesture_manager_touch_end(&handler->guesture_manager,touch,touch_count,touchlist,touchbit);
}

void on_touch_invalid(touch_handler_t *handler,const struct Touch *touch,int touch_count,const struct Touch *touchlist,int touchbit,const struct itrack_props_s *props){
	return on_touch_release(handler,touch,touch_count,touchlist,touchbit,props);
}

void on_physical_button_update(touch_handler_t *handler,const struct timeval *evtime,int physical_button_state){
	struct itrack_action_s *action = malloc( sizeof(struct itrack_action_s) );
	bzero(action,sizeof(struct itrack_action_s) );
	action->physical_button = physical_button_state;
	itrack_post_own(handler->guesture_manager.post_stage,action);
	if(&handler->guesture_manager){
		handler->guesture_manager.physical_button = physical_button_state;
	}
	handler->last_physical_button = physical_button_state;
}

void touch_handler_set_post_scrolling_state(touch_handler_t *handler,bool on,const struct timeval *time,const struct itrack_props_s *props){
		struct guesture_item_s *item = guesture_manager_find_item_by_name(&handler->guesture_manager,"scroll");
		assert(item);
		if( on ){
			guesture_manager_set_alt(&handler->guesture_manager,item);
		}else{
			guesture_manager_clear_alt(&handler->guesture_manager,item);
		}
}