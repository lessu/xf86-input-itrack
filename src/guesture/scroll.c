#include "./scroll.h"
#include "guesture_manager.h"
#include <assert.h>

static void on_start(struct scroll_guesture_s *guesture,const struct Touch *touches,int touch_bit){

}

static void on_update(struct scroll_guesture_s *guesture,const struct Touch *touches,int touch_bit){
	struct itrack_staged_status_s *staged = guesture->super.manager->staged;
    struct Touch *touch = NULL;
    int i;
    foreach_bit(i,touch_bit){
        touch = touches + i;
        break;
    }
    assert(touch!=NULL);

    guesture_set_match(&guesture->super,GUESTURE_SET_MATCH_STATUS_MATCH);
 	
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
    touchlist_double_avg(&staged->scroll.velocity_x,1000.0 * touch -> dx / diff_ms * SCROLL_INERTIA_MULTIPLIER,touch_idx,touch_count);
    touchlist_double_avg(&staged->scroll.velocity_y,1000.0 * touch -> dy / diff_ms * SCROLL_INERTIA_MULTIPLIER,touch_idx,touch_count);

    touchlist_double_avg(&staged->scroll.x,(double)touch -> dx,touch_idx,touch_count);
    touchlist_double_avg(&staged->scroll.y,(double)touch -> dy,touch_idx,touch_count);

    LOG_SCROLL("scroll para,offset = (%.2lf,%.2lf) , velocity = (%.2lf,%.2lf) diff_ms=%ld\n",staged->scroll.x,staged->scroll.y,staged->scroll.velocity_x,staged->scroll.velocity_y,diff_ms);

end:
    return ;
}

static void on_end(struct scroll_guesture_s *guesture,const struct Touch *touches,int touch_bit){
    struct itrack_staged_status_s *staged = guesture->super.manager->staged;
    /** scroll to move */
    staged->scroll.holding = FALSE;
}

static struct guesture_callbacks_s s_callbacks = {
    .on_start   = on_start,
    .on_update  = on_update,
    .on_end     = on_end
};

void scroll_guesture_init(struct scroll_guesture_s *guesture){
    guesture_init(&guesture->super,&s_callbacks,guesture);

    guesture->super.props.required_touches = 2;
}