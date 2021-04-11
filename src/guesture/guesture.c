#include "guesture.h"
#include "guesture_manager.h"

void guesture_init(struct guesture_s *guesture,const char *name,const struct guesture_callbacks_s *callbacks,void* userdata){
    bzero(guesture,sizeof(struct guesture_s));
    strncpy(guesture->name,name,MAX_GUESTURE_NAME_LENGTH - 1);
    guesture->callbacks = *callbacks;
    guesture->userdata = userdata;
}

void guesture_deinit(struct guesture_s *guesture){

}
void guesture_set_post_fn(struct guesture_s *guesture,GuestureActionFn fn,void *data){
    guesture->action_fn = fn;
    guesture->action_userdata = data;
}
void guesture_set_match(
    struct guesture_s *guesture,
    enum guesture_match_e set_match,
    guesture_match_opts_t opts
){
    /** todo://refactory this */
    for(int i = 0 ; i < guesture->manager->private.current_guesture_count ; i ++ ){
        struct guesture_item_s *item = guesture->manager->private.current_guesture_item_list[i];
        if(  item->used==USING && item->guesture == guesture ){
            item->current_accept_opts = opts;
        }
    }

    if(guesture->status.match_state != GUESTURE_MATHING){
        /** guestures should always check matching status */        
    }else{
        gettimeofday(&guesture->status.state_update_time,NULL);
        switch (set_match){
        case GUESTURE_MATCH_ESSENTIAL:
            guesture->status.is_essential = TRUE;
            guesture->status.match_state= GUESTURE_OK;
            GUESTURE_DEBUG("guesture<%s> ask essential\n",guesture->name);
        break;
        case GUESTURE_MATCH_OK:
            guesture->status.is_essential = FALSE;
            guesture->status.match_state= GUESTURE_OK;
            GUESTURE_DEBUG("guesture<%s> ask match\n",guesture->name);
        break;
        case GUESTURE_MATCH_NO:
            guesture->status.is_essential = FALSE;
            guesture->status.match_state= GUESTURE_FAILED;
            GUESTURE_DEBUG("guesture<%s> ask not match\n",guesture->name);
        break;
        }
        guesture_manager_set_guesture_state_change(guesture->manager,guesture);
    }
}

void guesture_post_action(struct guesture_s *guesture,struct itrack_action_s *action){
    action->emit_time = *guesture_get_time(guesture);
    if(guesture->action_fn){
        guesture->action_fn(guesture,action,guesture->action_userdata);
    }else{
        free(action);
    }
}

void guesture_post_movment(struct guesture_s *guesture,int x,int y,BOOL absolute){
    struct itrack_action_s *action = malloc(sizeof(struct itrack_action_s));
    bzero(action,sizeof(struct itrack_action_s) );
    action->pointer.x = x;
    action->pointer.y = y;
    action->pointer.move_type = absolute? ABSOLUTE:RELATIVE;
    guesture_post_action(guesture,action);
}

void guesture_post_button_up(struct guesture_s *guesture,int button_id,int defer_timeout){
    struct itrack_action_s *action = malloc(sizeof(struct itrack_action_s));
    bzero(action,sizeof(struct itrack_action_s) );
    
    if(defer_timeout > 0){
        action->button.defer_up.button    = 1 << button_id;
        struct timeval now_tv,tv;
        gettimeofday(&now_tv,NULL);
        timeraddms(&now_tv,defer_timeout,&tv);

        action->button.defer_up.operation = DEFER_NEW;
        action->button.defer_up.time 	  = tv;
    }else{
        action->button.up                 = 1 << button_id;
    }
    guesture_post_action(guesture,action);
}
void guesture_post_button_up_cancel(struct guesture_s *guesture,int button_id){
    struct itrack_action_s *action = malloc(sizeof(struct itrack_action_s));
    bzero(action,sizeof(struct itrack_action_s) );
    // if(button_id){}
    action->button.defer_up.operation = DEFER_CANCEL;
    (void)button_id;
    guesture_post_action(guesture,action);
}
void guesture_post_button_down(struct guesture_s *guesture,int button_id){
    struct itrack_action_s *action = malloc(sizeof(struct itrack_action_s));
    bzero(action,sizeof(struct itrack_action_s) );
    action->button.down =  1 << button_id;
    guesture_post_action(guesture,action);
}

void guesture_post_button_down_and_up(struct guesture_s *guesture,int button_id,int defer_timeout){
    struct itrack_action_s *action = malloc(sizeof(struct itrack_action_s));
    bzero(action,sizeof(struct itrack_action_s) );
    action->button.down               = (1 << button_id);
    action->button.defer_up.operation = DEFER_NEW;
    action->button.defer_up.button    = (1 << button_id);
    struct timeval now,tv;
    gettimeofday(&now,NULL);
    timeraddms(&now,defer_timeout,&tv);
    action->button.defer_up.time      = tv;
    guesture_post_action(guesture,action);
}


void guesture_post_scroll(struct guesture_s *guesture,int x,int y,double vx,double vy){
    struct itrack_action_s *action = malloc(sizeof(struct itrack_action_s));
    bzero(action,sizeof(struct itrack_action_s) );
    action->scroll.holding = TRUE;
    action->scroll.x = x;
    action->scroll.y = y;
    action->scroll.velocity_x = vx;
    action->scroll.velocity_y = vy;

    guesture_post_action(guesture,action);
}

    
    
void guesture_post_scroll_end(struct guesture_s *guesture){
    struct itrack_action_s *action = malloc(sizeof(struct itrack_action_s));
    bzero(action,sizeof(struct itrack_action_s) );

    action->scroll.holding = FALSE;

    guesture_post_action(guesture,action);
}

const struct timeval *guesture_get_time(struct guesture_s *guesture){
    return &guesture->manager->last_update_time;
}