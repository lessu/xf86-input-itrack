#include "./scroll.h"
#include "guesture_manager.h"
#include "guesture.h"
#include <assert.h>
#define SCROLL_INERTIA_MULTIPLIER (3.0f)
#define SIN_30 (0.5f)
#define SIN_60 (0.86602540378444f)
#define SIN_45 (0.707f)
static void s_push_history(struct scroll_guesture_s *guesture,struct gue_i_vector_s vect){
    guesture->dhistory[guesture->dhistory_length] = vect;
    if(guesture->dhistory_length + 1 == SCROLL_GUESTURE_DHISTORY_LENGTH){
        guesture->dhistory_full = TRUE;
    }
    guesture->dhistory_length = ( (guesture->dhistory_length + 1) % SCROLL_GUESTURE_DHISTORY_LENGTH);
}
static struct gue_i_vector_s s_dhistory_avg(struct scroll_guesture_s *guesture){
    int length = 0;
    if(guesture->dhistory_full){
        length = SCROLL_GUESTURE_DHISTORY_LENGTH;
    }else{
        length = guesture->dhistory_length;
    }
    struct gue_i_vector_s sum_vect = {0};
    for(int i = 0;i < length;i++){
        sum_vect.x += guesture->dhistory[i].x;
        sum_vect.y += guesture->dhistory[i].y;
    }
    sum_vect.x /= length;
    sum_vect.y /= length;
    return sum_vect;
}

static void on_start(void *userdata,const struct Touch *touches,int touch_bit){
    LOG_DEBUG("[scroll]on_start\n");
    struct scroll_guesture_s *guesture = userdata;
    guesture->dhistory_length = 0;
    guesture->dhistory_full   = FALSE;
    guesture->scroll_type = SCROLL_TYPE_UNDETERMINTED;
}

static void on_update(void *userdata,const struct Touch *touches,int touch_bit){
    LOG_DEBUG("[scroll]on_update\n");
    struct scroll_guesture_s *guesture = userdata;
	struct itrack_staged_status_s *staged = &guesture->guesture.staged;
    const struct Touch *touch = NULL;
    int i;
    struct gue_i_vector_s avg_dvect = {0};
    foreach_bit(i,touch_bit){
        touch = touches + i;
        avg_dvect.x += touch->dx;
        avg_dvect.y += touch->dy;
        break;
    }
    avg_dvect.x /= 2;
    avg_dvect.y /= 2;
    s_push_history(guesture,avg_dvect);
    assert(touch!=NULL);
    
    if(guesture->guesture.status.match_state == GUESTURE_MATHING){
        guesture_set_match(&guesture->guesture,GUESTURE_SET_MATCH_STATUS_MATCH,touch->update_time);
    }
 	
    // final algorithm 
    // todo://
    // 1 find out whether is vertical or horizontal (axsi +- 10 (tbd) degree)
    // 2 once out of this these area,scroll became freedom

    // use freedom way first

    // todo:// how to ensure its a scroll,not a pitch or rotate?
    
    /** two finger move will calls this function twice */

    staged->scroll.holding = TRUE;
    
    int64_t diff_ms = time_diff_ms(&touch->update_time,&touch->last_update_time);

    // avg the speed
    struct gue_i_vector_s speed_avg = s_dhistory_avg(guesture);
    staged->scroll.velocity_x = 1000.0 * speed_avg.x / diff_ms * guesture->cfg.inertial_scroll_multiplier;
    staged->scroll.velocity_y = 1000.0 * speed_avg.y / diff_ms * guesture->cfg.inertial_scroll_multiplier;

    if( ABSVAL(avg_dvect.x) < 5 && ABSVAL(avg_dvect.y) < 5){
        staged->scroll.x = 0;
        staged->scroll.y = 0;
    }else{
        staged->scroll.x = avg_dvect.x;
        staged->scroll.y = avg_dvect.y;
    }
    if(guesture->scroll_type != SCROLL_TYPE_FREE){
        double sin_angel = 1.0f * avg_dvect.x / sqrt(avg_dvect.x * avg_dvect.x + avg_dvect.y*avg_dvect.y );
        LOG_DEBUG("sin_angel = %lf, type = %d\n",sin_angel,guesture->scroll_type);
        // sin(15°) = 0.258
        Bool is_x_mode = ABSVAL(sin_angel) > SIN_45;
        Bool is_y_mode = ABSVAL(sin_angel) < SIN_45;
        switch(guesture->scroll_type){
        case SCROLL_TYPE_UNDETERMINTED:
            if(is_x_mode){
                guesture->scroll_type = SCROLL_TYPE_X;
                staged->scroll.y = 0;
                staged->scroll.velocity_y = 0;
            }else if(is_y_mode){
                guesture->scroll_type = SCROLL_TYPE_Y;
                staged->scroll.x= 0;
                staged->scroll.velocity_x = 0;
            }
        break;
        case SCROLL_TYPE_X:
            if(is_x_mode){
                staged->scroll.y = 0;
                staged->scroll.velocity_y = 0;
            }else if(is_y_mode){
                guesture->scroll_type = SCROLL_TYPE_FREE;
            }else{
                staged->scroll.y = 0;
                staged->scroll.velocity_y = 0;
            }
        break;
        case SCROLL_TYPE_Y:
            if(is_x_mode){
                guesture->scroll_type = SCROLL_TYPE_FREE;
            }else if(is_y_mode){
                staged->scroll.x= 0;
                staged->scroll.velocity_x = 0;
            }else{
                staged->scroll.x = 0;
                staged->scroll.velocity_x = 0;
            }
        break;
        default:
        break;
        }
    }
    
    LOG_DEBUG("scroll param: offset = (%.2lf,%.2lf) , velocity = (%.2lf,%.2lf) diff_ms=%ld\n",
        staged->scroll.x,staged->scroll.y,
        staged->scroll.velocity_x,staged->scroll.velocity_y,
        diff_ms);
    return ;
}

static Bool on_end(void *userdata,Bool is_cancel,int touch_count){
    LOG_DEBUG("[scroll]on_end\n");
    struct scroll_guesture_s *guesture = userdata;
    struct itrack_staged_status_s *staged = &guesture->guesture.staged;
    /** scroll to move */
    staged->scroll.holding = FALSE;
    return TRUE;
}

static struct guesture_callbacks_s s_callbacks = {
    .on_start   = on_start,
    .on_update  = on_update,
    .on_end     = on_end
};

void scroll_guesture_init(struct scroll_guesture_s *guesture){
    guesture_init(&guesture->guesture,"scroll",&s_callbacks,guesture);

    guesture->guesture.props.required_touches = 2;
    guesture->cfg.inertial_scroll_multiplier  = SCROLL_INERTIA_MULTIPLIER;
}