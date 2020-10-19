/**
 * 
 * 
 */
#pragma once
#include "itrack-type.h"
#include "mtstate.h"
#include <stdbool.h>
#include "common.h"

#define GUESTURE_DEBUG LOG_NULL

#define MAX_TOUCH_COUNT (10)
#define MAX_GUESTURE_NAME_LENGTH (32)

struct guesture_s;
struct itrack_action_s;
struct Touch;
/**
 * @param action :The action is on heap,and guesture don't free it.
 */ 
typedef void (*GuestureActionFn)(struct guesture_s *gursture,struct itrack_action_s *action,void *user_data);

struct gi_vector_s{
    int x,y;
};

enum guesture_match_e{
    GUESTURE_MATCH_ESSENTIAL,
    GUESTURE_MATCH_OK,
    GUESTURE_MATCH_NO
};
struct guesture_status_s{
    enum {
        GUESTURE_NONE = 0,
        GUESTURE_MATHING,
        GUESTURE_FAILED,
        GUESTURE_OK,
    } match_state;
    bool            is_essential;
    struct timeval  state_update_time;
};

struct guesture_props_s{
    int required_touches;
};
struct guesture_callbacks_s
{
    void (*on_start)(void *user_data,const struct Touch *touches,int touch_bit);
    void (*on_update)(void *user_data,const struct Touch *touches,int touch_bit);
    /**
     * @retval the return values is ignored
     */ 
    bool (*on_end)(void *user_data,bool is_cancel,int touch_count);
};

struct guesture_manager_s;
struct guesture_s {
    char name[MAX_GUESTURE_NAME_LENGTH];
    struct guesture_props_s         props;
    struct guesture_status_s        status;
    struct guesture_callbacks_s     callbacks;
    struct guesture_manager_s       *manager;
    void                            *userdata;
    
    GuestureActionFn                action_fn;
    void                            *action_userdata;
};

/**
 * Lifecyle
 */ 
void guesture_init(struct guesture_s *guesture,const char *name,const struct guesture_callbacks_s *callbacks,void* userdata);
void guesture_deinit(struct guesture_s *guesture);

/**
 * called inner guesture manager
 */ 
void guesture_set_post_fn(struct guesture_s *guesture,GuestureActionFn fn,void *data);

/**
 * called inner guesture
 */ 
void guesture_set_match(struct guesture_s *guesture,enum guesture_match_e set_match);

/**
 * actions
 */ 
void guesture_post_action(struct guesture_s *guesture,struct itrack_action_s *action);
void guesture_post_movment(struct guesture_s *guesture,int x,int y,bool absolute);
void guesture_post_button_up(struct guesture_s *guesture,int button_id,int defer_timeout);
void guesture_post_button_up_cancel(struct guesture_s *guesture,int button_id);
void guesture_post_button_down(struct guesture_s *guesture,int button_id);
void guesture_post_button_down_and_up(struct guesture_s *guesture,int button_id,int defer_timeout);
void guesture_post_scroll(struct guesture_s *guesture,int x,int y,double vx,double vy);
void guesture_post_scroll_end(struct guesture_s *guesture);