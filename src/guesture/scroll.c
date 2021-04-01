#include "scroll.h"
#include "guesture_manager.h"
#include "guesture.h"
#include <assert.h>
#include <math.h>
#define SCROLL_INERTIA_MULTIPLIER 1.6f
#define SIN_30 0.5f
#define SIN_60 0.86602540378444f
#define SIN_45 0.707f
static void s_push_history(struct scroll_guesture_s *guesture,struct gi_vector_s vect){
    guesture->dhistory[guesture->dhistory_length] = vect;
    if(guesture->dhistory_length + 1 == SCROLL_GUESTURE_DHISTORY_LENGTH){
        guesture->dhistory_full = TRUE;
    }
    guesture->dhistory_length = ( (guesture->dhistory_length + 1) % SCROLL_GUESTURE_DHISTORY_LENGTH);
}
static struct gi_vector_s s_dhistory_avg(struct scroll_guesture_s *guesture){
    int length = 0;
    if(guesture->dhistory_full){
        length = SCROLL_GUESTURE_DHISTORY_LENGTH;
    }else{
        length = guesture->dhistory_length;
    }
    struct gi_vector_s sum_vect = {0};
    for(int i = 0;i < length;i++){
        sum_vect.x += guesture->dhistory[i].x;
        sum_vect.y += guesture->dhistory[i].y;
    }
    sum_vect.x /= length;
    sum_vect.y /= length;
    return sum_vect;
}

static void on_start(void *userdata,const struct Touch *touches,int touch_bit){
    GUESTURE_DEBUG("[scroll]on_start\n");
    struct scroll_guesture_s *guesture = userdata;
    guesture->dhistory_length = 0;
    guesture->dhistory_full   = FALSE;
    guesture->scroll_type = SCROLL_TYPE_UNDETERMINTED;
}

static void on_update(void *userdata,const struct Touch *touches,int touch_bit){
    GUESTURE_DEBUG("[scroll]on_update\n");
    struct scroll_guesture_s *guesture = userdata;
    const struct Touch *touch = NULL;
    int i;
    struct gi_vector_s avg_dvect = {0};
    foreach_bit(i,touch_bit){
        touch = touches + i;
        avg_dvect.x += touch->dx;
        avg_dvect.y += touch->dy;
    }
    avg_dvect.x /= 2;
    avg_dvect.y /= 2;
    s_push_history(guesture,avg_dvect);
    assert(touch!=NULL);
    
    if(guesture->guesture.status.match_state == GUESTURE_MATHING){
        guesture_set_match(&guesture->guesture,GUESTURE_MATCH_OK);
    }
 	
    // final algorithm 
    // todo://
    // 1 find out whether is vertical or horizontal (axsi +- 10 (tbd) degree)
    // 2 once out of this these area,scroll became freedom

    // use freedom way first

    // todo:// how to ensure its a scroll,not a pitch or rotate?
    
    /** two finger move will calls this function twice */
    struct gi_vector_s scroll,scroll_velocity;
    int64_t diff_ms = time_diff_ms(&touch->update_time,&touch->last_update_time);
    // avg the speed
    struct gi_vector_s speed_avg = s_dhistory_avg(guesture);
    scroll_velocity.x = 1000.0 * speed_avg.x / diff_ms * guesture->cfg.inertial_scroll_multiplier;
    scroll_velocity.y = 1000.0 * speed_avg.y / diff_ms * guesture->cfg.inertial_scroll_multiplier;

    if( ABSVAL(avg_dvect.x) < 2 && ABSVAL(avg_dvect.y) < 2){
        scroll.x = 0;
        scroll.y = 0;
    }else{
        scroll.x = avg_dvect.x;
        scroll.y = avg_dvect.y;
    }
    if(guesture->scroll_type != SCROLL_TYPE_FREE){
        double sin_angel = 1.0f * avg_dvect.x / sqrt(avg_dvect.x * avg_dvect.x + avg_dvect.y*avg_dvect.y );
        GUESTURE_DEBUG("sin_angel = %lf, type = %d\n",sin_angel,guesture->scroll_type);
        // sin(15°) = 0.258
        BOOL is_x_mode = ABSVAL(sin_angel) > SIN_45;
        BOOL is_y_mode = ABSVAL(sin_angel) < SIN_45;
        switch(guesture->scroll_type){
        case SCROLL_TYPE_UNDETERMINTED:
            if(is_x_mode){
                guesture->scroll_type = SCROLL_TYPE_X;
                scroll.y = 0;
                scroll_velocity.y = 0;
            }else if(is_y_mode){
                guesture->scroll_type = SCROLL_TYPE_Y;
                scroll.x= 0;
                scroll_velocity.x = 0;
            }
        break;
        case SCROLL_TYPE_X:
            if(is_x_mode){
                scroll.y = 0;
                scroll_velocity.y = 0;
            }else if(is_y_mode){
                guesture->scroll_type = SCROLL_TYPE_FREE;
            }else{
                scroll.y = 0;
                scroll_velocity.y = 0;
            }
        break;
        case SCROLL_TYPE_Y:
            if(is_x_mode){
                guesture->scroll_type = SCROLL_TYPE_FREE;
            }else if(is_y_mode){
                scroll.x= 0;
                scroll_velocity.x = 0;
            }else{
                scroll.x = 0;
                scroll_velocity.x = 0;
            }
        break;
        default:
        break;
        }
    }
    guesture_post_scroll(&guesture->guesture,
        scroll.x,
        scroll.y,
        scroll_velocity.x,
        scroll_velocity.y
    );
    // GUESTURE_DEBUG("scroll param: offset = (%.2lf,%.2lf) , velocity = (%.2lf,%.2lf) diff_ms=%ld\n",
    //     staged->scroll.x,staged->scroll.y,
    //     staged->scroll.velocity_x,staged->scroll.velocity_y,
    //     diff_ms);
    return ;
}

static BOOL on_end(void *userdata,BOOL is_cancel,int touch_count){
    GUESTURE_DEBUG("[scroll]on_end\n");
    struct scroll_guesture_s *guesture = userdata;
    /** scroll to move */
    guesture_post_scroll_end(&guesture->guesture);
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