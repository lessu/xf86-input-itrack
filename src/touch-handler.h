#pragma once
#include "common.h"
#include "itrack-type.h"
#include "mtstate.h"
#include "guesture/guesture_manager.h"

// #define TAP_POST_MOVE_INHERBITION_TIME 	(50)
#define MOVE_CLEAR_OTHER_DIST2    		(30*30)

#define SWIPE3_BUTTON 		  0
#define SWIPE3_HOLD_TIME      1000
#define SWIPE3_MOVE_THRESHHOLD_DIST2  (0*0)
#define SWIPE3_MOVE_THRESHHOLD_TIME   (40)

#define PHYSICAL_BUTTON_MICRO_MOVE_THRESHOLD (50)
#define MAX_MOVE_INHERBITION_DIST (16)

#define SCROLL_INERTIA_MULTIPLIER (2.0f)
#define SCROLL_POST_FORBIDEN_TIME_MS (300)
enum guard_settle_e{
	SETTLE_NONE = 0,
	SETTLE_SETTLE,
	SETTLE_CLEARED
};

struct itrack_props_s;

typedef struct 
{
	BOOL start;
	BOOL max_touches_recorded;
	struct {
		enum guard_settle_e physical_button_settle;
		enum guard_settle_e pretap_settle;

		struct timeval move_inherbition_time;
		// struct timeval move_inherbition_dist;
	} move_guard;
	
	struct {
		BOOL 			is_inertia_sliding;
		struct timeval 	last_off_time;
	}scroll_state;

	struct{
		int tap_disabled;
		// int has_physical_button_pressed;
		int sum_x;
		int sum_y;
	} tap;

	struct{
		enum {
			SpecialTyoe_None = 0,
			SpecialTyoe_Swipe3 ,
		} special_type;
		struct timeval start_time;
		struct timeval pseudo_endtime;
		struct timeval pseudo_starttime;
	}state;

	int last_physical_button;
	struct guesture_manager_s guesture_manager;
} touch_handler_t;

void touch_handler_init(touch_handler_t *handler,struct post_stage_s *post_stage,const struct itrack_props_s *props);
void touch_handler_deinit(touch_handler_t *handler);

void on_touch_start(touch_handler_t *handler, const struct Touch *touch,int touch_count,const struct Touch *touchelist,int touchbit,const struct itrack_props_s *props);

void on_touch_move(touch_handler_t *handler,const struct Touch *touch,int touch_idx,int touch_count,const struct Touch *touchelist,int touchbit,const struct itrack_props_s *props);

void on_touch_release(touch_handler_t *handler,const struct Touch *touch,int touch_count,const struct Touch *touchelist,int touchbit,const struct itrack_props_s *props);

void on_touch_invalid(touch_handler_t *handler,const struct Touch *touch,int touch_count,const struct Touch *touchelist,int touchbit,const struct itrack_props_s *props);

void on_physical_button_update(touch_handler_t *handler,const struct timeval *evtime,int physical_button_state);

void touch_handler_set_post_scrolling_state(touch_handler_t *handler,BOOL on,const struct timeval *time,const struct itrack_props_s *props);