#pragma once
#include "guesture.h"

#define MAX_GUESTURE_COUNT (32)
#define GUESTURE_PRE_WAITING_TIME  (150)
#define GUESTURE_PRE_WAITTING_DIST (60)
// #define GUESTURE_POST_WAITING_TIME (200)

struct itrack_staged_status_s;
struct itrack_props_s;
struct guesture_s;
struct guesture_item_s {
    enum {
        NO_USE = 0,
        USING
    } used;
    int priority;
    struct guesture_s *guesture;
    void   *userdata;
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
            MANAGER_STATE_LEAVING
        } state;

        int                     updating_touchbit;
        struct timeval          last_waitting_update;
        struct guesture_item_s *current_guesture_item_list[MAX_GUESTURE_COUNT];
        int                     current_guesture_count;
        struct guesture_item_s *accepted_item;
        // Bool                    guesture_state_has_changed;    
        enum{
            GUESTURE_MANAGER_CALLBACK_NONE,
            GUESTURE_MANAGER_CALLBACK_START,
            GUESTURE_MANAGER_CALLBACK_UPDATE,
            GUESTURE_MANAGER_CALLBACK_END,
        }                    callback_state;
        struct itrack_staged_status_s *current_staged;
        int                     max_touches_number;
    }private;
    struct guesture_item_s        *alt_guesture;
    // struct timeval                 alt_exceed_time;
    int                            physical_button;
};

/** manager api */
void guesture_manager_init(struct guesture_manager_s *manager);

int  guesture_manager_add(struct guesture_manager_s *manager,struct guesture_s *guesture,void *userdata,int priority);

struct guesture_item_s *guesture_manager_find_item_by_name(struct guesture_manager_s *manager,const char *name);

/**
 * mouse event
 */ 
void guesture_manager_touch_start(struct guesture_manager_s *manager,const struct Touch *touch,int touch_count,const struct Touch *touchlist,int touchbit,struct itrack_staged_status_s *staged);

void guesture_manager_touch_update(struct guesture_manager_s *manager,const struct Touch *touch,int touch_idx,int touch_count,const struct Touch *touchlist,int touchbit,struct itrack_staged_status_s *staged);

void guesture_manager_touch_end(struct guesture_manager_s *manager,const struct Touch *touch,int touch_count,const struct Touch *touchlist,int touchbit,struct itrack_staged_status_s *staged);

void guesture_manager_physical_button_update(struct guesture_manager_s *manager,int buttonbit);

/**
 * guesture api
 */ 
Bool guesture_manager_is_accept(struct guesture_manager_s *manager,struct guesture_s *guesture);

void guesture_manager_set_guesture_state_change(struct guesture_manager_s *manager,struct guesture_s *guesture);

void guesture_manager_set_alt(struct guesture_manager_s *manager,struct guesture_item_s *item);

void guesture_manager_clear_alt(struct guesture_manager_s *manager,struct guesture_item_s *item);

// void guesture_manager_set_point_move(struct guesture_s *guesture,int x,int y);