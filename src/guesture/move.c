#include "common.h"
#include "./move.h"
#include "guesture_manager.h"
#include <assert.h>
#include "itrack-type.h"
#include "mtstate.h"

static void on_start(void *handler,const struct Touch *touches,int touch_bit){
    LOG_DEBUG("[move]on_start\n");
}

static void on_update(void *handler,const struct Touch *touches,int touch_bit){
    LOG_DEBUG("[move]on_update\n");
    struct move_guesture_s *guesture = handler;
    const struct Touch *touch = NULL;
    
    int i; foreach_bit(i,touch_bit){
        touch = touches + i;
        break;
    }
    assert(touch!=NULL);

    /** micro move when physical button pressing should be ignored */
    if(guesture->guesture.manager->physical_button > 0){
        /** 
         * todo://
         * if pressing the trackpad and finger gets up.
         * there will be some move events triggered before touch relase.
         * so after button down,it should cache some moves to debounce that move events.
        */
        if(guesture->physical_button_settle == PHYSICAL_BUTTON_SETTLE_NONE){
            /** from no physical button pressed to pressed state */
            guesture->physical_button_settle = PHYSICAL_BUTTON_SETTLE_SETTLE;
            // handler->tap.tap_disabled = TRUE;
            guesture_set_match(&guesture->guesture,GUESTURE_SET_MATCH_STATUS_ESSENTIAL,touch->update_time);
        }

        if( guesture->physical_button_settle == PHYSICAL_BUTTON_SETTLE_SETTLE ){
            if( math_dist2( touch->dx,touch->dy ) < PHYSICAL_BUTTON_MIN_MOVE * PHYSICAL_BUTTON_MIN_MOVE){
                LOG_DEBUG("button down micro move guard\n");
                goto end;
            }else{
                /** once big movement checked out,disable micro move guard */
                guesture->physical_button_settle = PHYSICAL_BUTTON_SETTLE_CLEAR;
            }
        }

    }else if(guesture->physical_button_settle == PHYSICAL_BUTTON_SETTLE_CLEAR){
        /** from physical button pressed to idle state */
        /** pointer never moves */
        guesture->physical_button_settle = PHYSICAL_BUTTON_SETTLE_NONE;
    }else if(guesture->physical_button_settle == PHYSICAL_BUTTON_SETTLE_SETTLE){
        /** from physical button pressed to idle state */
        /** pointer moved */
        guesture->physical_button_settle = PHYSICAL_BUTTON_SETTLE_NONE;
    }else{
        if(guesture->guesture.status.match_state == GUESTURE_MATHING){
            guesture_set_match(&guesture->guesture,GUESTURE_SET_MATCH_STATUS_MATCH,touch->update_time);
        }else if(guesture->guesture.status.match_state == GUESTURE_OK){

        }
        guesture->guesture.staged.pointer.x = touch -> dx;
        guesture->guesture.staged.pointer.y = touch -> dy;
    }
end:
    return ;
}
static Bool on_end(void *handler,Bool is_cancel,int touch_count){
    LOG_DEBUG("[move]on_end\n");
    return TRUE;
}

static struct guesture_callbacks_s s_callbacks = {
    .on_start   = on_start,
    .on_update  = on_update,
    .on_end     = on_end
};

void move_guesture_init(struct move_guesture_s *guesture){
    guesture_init(&guesture->guesture,"move",&s_callbacks,guesture);
    guesture->physical_button_settle            = PHYSICAL_BUTTON_SETTLE_NONE;
    guesture->guesture.props.required_touches   = 1;
}