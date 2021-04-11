#pragma once
#include "./type.h"

#define MAX_CALLBACK_COUNT 8
#define MAX_GUESTURE_COUNT 32
/**
 * From 0 touch to 1 touch,It go in pre wait state
 * Because we are not should if there will be more touch
 * 
 * So wait this `time` 
 * or this touch movement least than `dist`
 * util we can sure no more touch will come
 */ 
#define GUESTURE_PRE_WAITING_TIME  100
#define GUESTURE_PRE_WAITTING_DIST 50
/** 
 * when touch count changes,
 * It will go into changing state.
 * In this state no action could emit
 * 
 * How long to stay in chaning state 
 * when touch count changes 
 */
#define GUESTURE_CHANGING_TIME 150
// #define GUESTURE_POST_WAITING_TIME (200)
#define GESTURE_BUTTON_LEFT 0
#define GESTURE_BUTTON_MIDDLE 1
#define GESTURE_BUTTON_RIGHT 2



typedef int guesture_match_opts_t;
#define GUSTURE_MATH_OPT_NONE   0
#define GUSTURE_MATH_OPT_CLEAR  1

struct post_stage_s;
struct itrack_action_s;
struct guesture_s;
struct guesture_manager_s;
struct guesture_item_s;
struct guesture_manager_action_itr_s{
    struct itrack_action_s *action;
    struct guesture_s      *guesture;
    struct guesture_manager_action_itr_s *next;
} ;

typedef void (*GuestureManagerOnAcceptFn)(struct guesture_manager_s *manager,struct guesture_item_s *item,void *user_data);

struct guesture_item_s {
    enum {
        NO_USE = 0,
        USING
    } used;
    int priority;
    struct guesture_s *guesture;
    guesture_match_opts_t current_accept_opts;
};

struct guesture_manager_s
{
    struct guesture_item_s guesture_list[MAX_GUESTURE_COUNT];

    struct{
        enum{
            /** not started 0 touch */
            MANAGER_STATE_NONE,
            /** touch number changed , waitting for debounce*/
            MANAGER_STATE_WAITING,
            /** touch number changed , and ready to recognize a guesture */
            // MANAGER_STATE_ARMED,
            /** guesture is updating */
            MANAGER_STATE_UPDATING,
            /** from required touch number to un-match touch number */
            MANAGER_STATE_CHANGING,
        } state;

        int                     updating_touchbit;
        struct timeval          last_waitting_update;
        struct guesture_item_s *current_guesture_item_list[MAX_GUESTURE_COUNT];
        int                     current_guesture_count;
        struct guesture_item_s *accepted_item;
        // BOOL                    guesture_state_has_changed;    
        enum{
            GUESTURE_MANAGER_CALLBACK_NONE,
            GUESTURE_MANAGER_CALLBACK_START,
            GUESTURE_MANAGER_CALLBACK_UPDATE,
            GUESTURE_MANAGER_CALLBACK_END,
        } callback_state;
        struct timeval          state_changing_until;
        int                     max_touches_number;
    }private;

    /**
     * guesture manager will only try to match alt guesture when it is un-null
    */
    struct guesture_item_s        *alt_guesture;
    // struct timeval                last_guesture_time;
    struct guesture_item_s        *last_guesture;
    int                           physical_button;

    struct {
        enum{
            ACCEPT_CALLBACK_EMPTY = 0,
            ACCEPT_CALLBACK_USING = 1
        }state;
        GuestureManagerOnAcceptFn callback;
        void* userdata;

    } on_accept_callbacks[MAX_CALLBACK_COUNT];

    // todo:// to rebuild
    struct post_stage_s           *post_stage;
    struct guesture_manager_action_itr_s *action_itr;


    /** last time when manager was feed with a touch event*/
    struct timeval last_update_time;
};


/** manager api */
void guesture_manager_init(struct guesture_manager_s *manager,struct post_stage_s           *post_stage);

int  guesture_manager_add(struct guesture_manager_s *manager,struct guesture_s *guesture,int priority);

struct guesture_item_s *guesture_manager_find_item_by_name(struct guesture_manager_s *manager,const char *name);

/**
 * mouse event
 */ 
void guesture_manager_touch_start(struct guesture_manager_s *manager,const struct Touch *touch,int touch_count,const struct Touch *touchlist,int touchbit);

void guesture_manager_touch_update(struct guesture_manager_s *manager,const struct Touch *touch,int touch_idx,int touch_count,const struct Touch *touchlist,int touchbit);

void guesture_manager_touch_end(struct guesture_manager_s *manager,const struct Touch *touch,int touch_count,const struct Touch *touchlist,int touchbit);

void guesture_manager_physical_button_update(struct guesture_manager_s *manager,int buttonbit);

/**
 * guesture api
 */ 
BOOL guesture_manager_is_accept(struct guesture_manager_s *manager,struct guesture_s *guesture);

void guesture_manager_set_guesture_state_change(struct guesture_manager_s *manager,struct guesture_s *guesture);

void guesture_manager_set_alt(struct guesture_manager_s *manager,struct guesture_item_s *item);

void guesture_manager_clear_alt(struct guesture_manager_s *manager,struct guesture_item_s *item);

int guesture_manager_register_on_accept_callback(struct guesture_manager_s *manager,GuestureManagerOnAcceptFn fn,void *userdata);
    
int guesture_manager_unregister_on_accept_callback(struct guesture_manager_s *manager,GuestureManagerOnAcceptFn fn,void *userdata);
    
// void guesture_manager_set_point_move(struct guesture_s *guesture,int x,int y);