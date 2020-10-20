#include "debug.h"
#include "common.h"
#define DEBUG_LOGGING_ON 0
static void log_touch(const struct Touch *touch){
    const char *status_string;
    const char *type_string;
    const char *extra_string;
    if ( touch->flags & (1<<MT_NEW) ){
        status_string = "NEW";
    }else if ( touch->flags & (1<<MT_RELEASED) ){
        status_string = "RELEASE";
    }else if ( touch->flags & (1<<MT_INVALID) ){
        status_string = "INVALID";
    }else{
        status_string = "UPDATE";
    }

    if ( touch->flags & (1<<MT_THUMB) ){
        type_string = "THUMB";
    }else if ( touch->flags & (1<<MT_PALM) ){
        type_string = "PALM";
    }else{
        type_string = "FINGER";
    }
    if ( touch->flags & (1<<MT_EDGE) ){
        extra_string = " EDGE";
    }else{
        extra_string = "";
    }

    LOG_DEBUG_CONT("([%ld.%03ld] id=%d , %s %s%s , size=(%d,%d) , loc=(%d,%d) , delta=(%d,%d) ),sun_delta=(%d,%d) create_time=[%ld,%03ld]\n",
        touch->update_time.tv_sec,touch->update_time.tv_usec,
        touch->tracking_id,
        status_string,
        type_string,
        extra_string,
        touch->size_touch,touch->size_ap,
        touch->x,touch->x,
        touch->dx,touch->dy,
        touch->total_dx,touch->total_dy,
        touch->create_time.tv_sec,touch->create_time.tv_usec
    );
}
void log_touches(const struct Touch *touches,int mask){
#if DEBUG_LOGGING_ON
    LOG_DEBUG("touches = [");
    for(int i = 0; i < 32 ;i ++ ){
        if( GETBIT(mask,i) ){
            log_touch(touches + i);
        }
    }
    LOG_DEBUG_CONT("];\n");
#endif
}