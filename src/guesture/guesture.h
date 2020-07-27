/**
 * 
 * 
 */
#pragma once
#include "itrack-type.h"
#include "mtstate.h"
#include "common.h" 
#define MAX_TOUCH_COUNT (10)
#define MAX_GUESTURE_NAME_LENGTH (32)
struct itrack_staged_status_s;
struct Touch;

enum guesture_set_match_status_e{
    GUESTURE_SET_MATCH_STATUS_ESSENTIAL,
    GUESTURE_SET_MATCH_STATUS_MATCH,
    GUESTURE_SET_MATCH_STATUS_NOT_MATCH
};
struct guesture_status_s{
    enum {
        GUESTURE_NONE = 0,
        GUESTURE_MATHING,
        GUESTURE_FAILED,
        GUESTURE_OK,
    } match_state;
    Bool            is_essential;
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
    Bool (*on_end)(void *user_data,Bool is_cancel,int touch_count);
    // void (*on_cancel)(struct pinch_guesture_s *guesture,Bool is_cancel);
};

struct guesture_manager_s;
struct guesture_s {
    char name[MAX_GUESTURE_NAME_LENGTH];
    struct guesture_props_s         props;
    struct guesture_status_s        status;
    struct guesture_callbacks_s     callbacks;
    struct guesture_manager_s       *manager;
    struct itrack_staged_status_s   staged;
};

void guesture_init(struct guesture_s *guesture,const char *name,const struct guesture_callbacks_s *callbacks,void* userdata);

void guesture_set_match(struct guesture_s *guesture,enum guesture_set_match_status_e set_match,struct timeval time);

// Bool guesture_set_alt(struct guesture_s *guesture);

// void guesture_clear_alt(struct guesture_s *guesture);