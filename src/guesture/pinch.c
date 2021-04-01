#include "pinch.h"
#include "mtstate.h"
#include "math.h"
#define MIN_MOVEMENT 40
#define THRESH_HOLD_ANGLE 30
/** to zoomfor 1x PINCH_SCALE_PRESCALE distance is needed */
#define PINCH_SCALE_PRESCALE 400.0

static double vector_dot_cos_theta(double ax,double ay,double bx,double by){
    return (ax*bx + ay*by)/(sqrt(ax*ax+ay*ay)*sqrt(bx*bx+by*by));
}

static void on_start(void *user_data,const struct Touch *touches,int touch_bit){
//    struct pinch_guesture_s *guesture = user_data;
    GUESTURE_DEBUG("[pinch]on_start\n");

}

static void on_update(void *user_data,const struct Touch *touches,int touch_bit){
    GUESTURE_DEBUG("[pinch]on_update\n");
    struct pinch_guesture_s *guesture = user_data;
    const struct Touch *touch1 = NULL,*touch2 = NULL;
    int i;
    foreach_bit(i,touch_bit){
        if(touch1 == NULL ){
            touch1 = touches + i;
        }else{
            touch2 = touches + i;
        }
    }
 
    if( guesture->guesture.status.match_state == GUESTURE_MATHING){
       
        if( math_dist2(touch1 -> dx ,touch1 -> dy) < MIN_MOVEMENT * MIN_MOVEMENT || math_dist2(touch2 -> dx ,touch2 -> dy) < MIN_MOVEMENT * MIN_MOVEMENT ){
            goto end;
        }

        /** all touch move out for a distance */
        /** don't put a else here */
        const double mid_point_x = (touch1->x + touch2->x) / 2;
        const double mid_point_y = (touch1->y + touch2->y) / 2;
        /**
         * 
         * Va<------A<---------M-------->B----->Vb
         * 
         * A=touch1
         * M=mid_point
         * B=touchb
         * Va=velocity(delta) of A
         * Vb=velocity(delta) of B
         * 
         * angleof(vec(MAVa)) should within +-THRESH_HOLD_ANGLE
         * 
         */
        const double cos_theta1 = vector_dot_cos_theta(
            /** MA  */ touch1->x - mid_point_x   , touch1->y - mid_point_y,
            /** AVa */ touch1->dx                , touch1->dy
        );
        const double cos_theta2 = vector_dot_cos_theta(
            /** MA  */ touch1->x - mid_point_x   , touch1->y - mid_point_y,
            /** AVa */ touch1->dx                , touch1->dy
        );
        GUESTURE_DEBUG("pinch try match theta=(A[%.2lf],B[%.2lf]) (A[%d,%d,],M[%d,%d],B[%lf,%lf]) velocity=(A[%d,%d],B[%d,%d])\n",
            acos(cos_theta1),acos(cos_theta2),
            touch1->x,touch1->y,touch2->x,touch2->y,mid_point_x,mid_point_x,
            touch1->dx,touch1->dy,touch2->dx,touch2->dy
        );
        if( cos_theta1 > cos(THRESH_HOLD_ANGLE) && cos_theta2 > cos(THRESH_HOLD_ANGLE)){
            guesture_set_match(&guesture->guesture,TRUE);
            guesture->start_distance2_to_mid = math_dist2((touch1->x - mid_point_x) / 2,(touch1->y - mid_point_y)/2);

            // todo:// pinch and rotate is not supported by xf86 directly,we convert them as touch event.
        }else{
            // guesture_set_match(&guesture->guesture,FALSE);
        }
    }

    if( guesture->guesture.status.match_state == GUESTURE_OK){
        // dist = (start distance to start mid) - (distance to mid)
        // scale = dist / PRESCALE
        const double mid_point_x = (touch1->x + touch2->x) / 2;
        const double mid_point_y = (touch1->y + touch2->y) / 2;
        const int distance_to_mid = math_dist2((touch1->x - mid_point_x) / 2,(touch1->y - mid_point_y)/2);
        const int delta_dist = sqrt(distance_to_mid) - sqrt(guesture->start_distance2_to_mid);
        (void)delta_dist;
        GUESTURE_DEBUG("pinch scale change = %.2lf (%d)\n",delta_dist/PINCH_SCALE_PRESCALE,delta_dist);
    }
end:
    return ;
}

static BOOL on_end(void *user_data,BOOL is_cancel,int touch_count){
    // struct pinch_guesture_s *guesture = user_data;
    GUESTURE_DEBUG("[pinch]on_end\n");
    return TRUE;
}

static struct guesture_callbacks_s s_callbacks = {
    .on_start   = on_start,
    .on_update  = on_update,
    .on_end     = on_end
};

void pinch_guesture_init(struct pinch_guesture_s *guesture){
    guesture_init(&guesture->guesture,"pinch",&s_callbacks,guesture);
    guesture->guesture.props.required_touches = 2;
}