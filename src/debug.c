#include "debug.h"
#include "common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "guesture.h"
#define DEBUG_LOGGING_ON 0

#define DEBUG_FIFO_PATH "/tmp/itrack_debug_fifo"

#define TOUCHES_BUFF_SIZE \
    (sizeof(struct Touch) * MAX_TOUCH_COUNT)    

struct {
    bool      enabled;
    int       fd;
    pthread_t thread;
    pthread_cond_t  cond;
    pthread_mutex_t mutex;

    uint8_t         buff[TOUCHES_BUFF_SIZE + sizeof(int) + sizeof(int)];

    bool            is_waiting;
    bool            is_writing;
    bool            discontinue;
} s_debug_pipe;

#if DEBUG_LOGGING_ON
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

    // LOG_DEBUG_CONT("([%ld.%03ld] id=%d , %s %s%s , size=(%d,%d) , loc=(%d,%d) , delta=(%d,%d) ),sun_delta=(%d,%d) create_time=[%ld,%03ld]\n",
    //     touch->update_time.tv_sec,touch->update_time.tv_usec,
    //     touch->tracking_id,
    //     status_string,
    //     type_string,
    //     extra_string,
    //     touch->size_touch,touch->size_ap,
    //     touch->x,touch->x,
    //     touch->dx,touch->dy,
    //     touch->total_dx,touch->total_dy,
    //     touch->create_time.tv_sec,touch->create_time.tv_usec
    // );

    LOG_DEBUG_CONT("([%ld.%03ld] id=%d , %s %s%s , size=([%d,%d],ap=[%d,%d]), distance=(%d), loc=(%d,%d) , delta=(%d,%d) )\n",
        touch->update_time.tv_sec,touch->update_time.tv_usec,
        touch->tracking_id,
        status_string,
        type_string,
        extra_string,
        touch->touch_major,touch->touch_minor,touch->ap_major,touch->ap_minor,
        touch->distance,
        touch->x,touch->x,
        touch->dx,touch->dy
    );
}

void log_touches(const struct Touch *touches,int mask){
    LOG_DEBUG("touches = [");
    for(int i = 0; i < MAX_TOUCH_COUNT ;i ++ ){
        if( GETBIT(mask,i) ){
            log_touch(touches + i);
        }
    }
    LOG_DEBUG_CONT("];\n");
}
#else



void log_touches(const struct itrack *itrack,const struct Touch *touches,int mask)
{
    if( !s_debug_pipe.enabled ) {
        return ;
    }
    int state = 0;
    if(s_debug_pipe.is_waiting){
        if(s_debug_pipe.discontinue == true){
            state = 0x1;
        }

        uint8_t *buff = s_debug_pipe.buff;
        memcpy(buff,&state,sizeof(int));
        buff += sizeof(int);
        memcpy(buff,&mask,sizeof(int));
        buff += sizeof(int);
        memcpy(buff,touches,TOUCHES_BUFF_SIZE);

        pthread_cond_signal(&s_debug_pipe.cond);

    }else{
        /**
         *  
         * when is writing, 
         * this log is to lost
         * 
         */
        s_debug_pipe.discontinue = true;
    }
}

#endif

static void* debug_fifo_thread_main(void *user_data){
    while(1){
        s_debug_pipe.is_waiting  = true;
        pthread_cond_wait(&s_debug_pipe.cond,&s_debug_pipe.mutex);
        s_debug_pipe.is_waiting  = false;

        if(s_debug_pipe.fd < 0){
            /** try open */
            s_debug_pipe.fd = open(DEBUG_FIFO_PATH,O_WRONLY,0666);
        }

        if(s_debug_pipe.fd < 0){
            /** 
             * When other side is not ready for reading
             * Just retry next time
             */
            continue;
        }
        
        s_debug_pipe.is_writing  = true;
        int wrote_size = write(
            s_debug_pipe.fd,
            s_debug_pipe.buff,
            sizeof(s_debug_pipe.buff)
        );
        s_debug_pipe.is_writing  = false;

        if ( wrote_size < 0 ) {
            switch (errno)
            {
            case EPIPE:
                /** reader closed */
                LOG_INFO("debug fifo EPIPE received\n");
                close(s_debug_pipe.fd);
                break;
            case EINTR:
                /** intr signal received */
                /** 
                 * this should not happen
                 */ 
                break;
            default:
                /** unknown reason */
                close(s_debug_pipe.fd);
                break;
            }
            s_debug_pipe.fd = -1;
        }else if( wrote_size == sizeof(s_debug_pipe.buff) ){
            /** normal */
        }else{
            /** buff full */
            /** this should not happen */
            /** stop thread */
            LOG_DEBUG("debug buff write uncomplete.\n");
            return (void*)0;
        }
    }
    return (void*)0;
}

static int debug_fifo_create( void )
{
    int ret = 0;
    struct stat st;
    if ( stat( DEBUG_FIFO_PATH, &st ) != 0 ) {
        // not exist;
        int mk_ret = mkfifo( DEBUG_FIFO_PATH, 0666 );
        if ( mk_ret != 0 ) {
            LOG_DEBUG("Create debug fifo %s failed,errno=%s(%d)\n",DEBUG_FIFO_PATH,strerror(errno),errno);
            ret = -1;
        }else{
            LOG_DEBUG("Create debug fifo %s success\n",DEBUG_FIFO_PATH);
            ret = 0;
        }
    } else {
        if ( S_ISFIFO( st.st_mode ) ){
            // already exist
            LOG_DEBUG("Debug fifo %s already exist\n",DEBUG_FIFO_PATH);
            ret = 0;
        } else {
            // already exist a no fifo file
            LOG_DEBUG("Create debug fifo %s failed,file exist but not a fifo\n",DEBUG_FIFO_PATH);
            ret = -2;
        }
    }
    return ret;
}

void debug_fifo_enable( void ){
    if( s_debug_pipe.enabled ){
        return ;
    }

    if( debug_fifo_create() == 0){
        s_debug_pipe.enabled = true;
        s_debug_pipe.is_waiting = false;
        s_debug_pipe.is_writing = false;
        s_debug_pipe.fd = -1;
        pthread_cond_init(&s_debug_pipe.cond,NULL);
        pthread_mutex_init(&s_debug_pipe.mutex,NULL);
        pthread_create(&s_debug_pipe.thread,0,debug_fifo_thread_main,NULL);
    }

}