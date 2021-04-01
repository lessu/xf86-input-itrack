#include "common.h"
#include "itrack.h"
#include "guesture_manager.h"
#include "post-stage.h"
#include "guesture.h"

#define LOG_GM LOG_NULL
static void apply_on_accept_callback(struct guesture_manager_s *manager,struct guesture_item_s *item);

static void s_guesture_action_matching_fn(struct guesture_s *guesture,struct itrack_action_s *action,void *userdata){
    struct guesture_manager_s *manager = userdata;

    struct guesture_manager_action_itr_s **action_itr = &manager->action_itr;
    while( *action_itr != NULL ){
        action_itr = &(*action_itr)->next;
    }
    *action_itr = malloc(sizeof(struct guesture_manager_action_itr_s));
    (*action_itr)->action = action;
    (*action_itr)->guesture = guesture;
    (*action_itr)->next = NULL;
}

static void s_guesture_action_accept_fn(struct guesture_s *guesture,struct itrack_action_s *action,void *userdata){
    struct guesture_manager_s *manager = userdata;
    itrack_post_own(manager->post_stage,action);
}

static void s_guesture_start(struct guesture_manager_s *manager,struct guesture_item_s *item,const struct Touch *touches,int touch_bit){
    const struct Touch *touch;
    int i;foreach_bit(i,touch_bit){
        touch = touches + i;
    }
    assert(touch != NULL);
    assert(item->guesture->status.match_state == GUESTURE_NONE);
    item->guesture->status.match_state= GUESTURE_MATHING;
    item->guesture->callbacks.on_start(item->guesture->userdata,touches,touch_bit);
    guesture_set_post_fn(item->guesture, s_guesture_action_matching_fn,manager);

}
static void s_guesture_update(struct guesture_manager_s *manager,struct guesture_item_s *item,const struct Touch *touches,int touch_bit){
    if( item->guesture->status.match_state == GUESTURE_MATHING || item->guesture->status.match_state == GUESTURE_OK ){
        item->guesture->callbacks.on_update(item->guesture->userdata,touches,touch_bit);
    }
}
static bool s_guesture_end(struct guesture_manager_s *manager,struct guesture_item_s *item,bool cancel,int count){
    bool ret = 0;
    if( item->guesture->status.match_state == GUESTURE_MATHING || 
        item->guesture->status.match_state == GUESTURE_OK 
    ){
        ret = item->guesture->callbacks.on_end(item->guesture->userdata,cancel,count);
        item->guesture->status.match_state = GUESTURE_NONE;
        guesture_set_post_fn(item->guesture,NULL,NULL);
    }
    return ret;
}

static void s_guesture_reset(struct guesture_manager_s *manager,struct guesture_item_s *item){
    switch (item->guesture->status.match_state){
    case GUESTURE_MATHING:
    case GUESTURE_FAILED:
    case GUESTURE_OK:
        s_guesture_end(manager,item,TRUE,0);
    break;
    case GUESTURE_NONE:
    default:
        // doing nothing;
        break;
    }
    item->guesture->status.match_state= GUESTURE_NONE;
}
static void s_get_matched_gesture(struct guesture_manager_s *manager,int touches,struct guesture_item_s *guestures[MAX_GUESTURE_COUNT],int *count){
    *count = 0;
    for(int i = 0; i < MAX_GUESTURE_COUNT; i ++ ){
        if( manager->guesture_list[i].used == USING ){
            if(manager->guesture_list[i].guesture->props.required_touches == touches){
                guestures[*count] = &manager->guesture_list[i];
                *count = *count + 1;
            }
        }
    }
}
static void s_accept_guesture(struct guesture_manager_s *manager,struct guesture_item_s *accept_item){
    for(int i = 0 ; i < manager->private.current_guesture_count ; i ++ ){
        struct guesture_item_s *item = manager->private.current_guesture_item_list[i];
        if( item == accept_item){
            // guesture_set_post_fn(item->guesture,NULL,NULL);
        }else if( item->guesture->status.match_state == GUESTURE_MATHING || item->guesture->status.match_state == GUESTURE_OK ){
            s_guesture_end(manager,item,TRUE,0);
        }
    }

    apply_on_accept_callback(manager,accept_item);

    /** post staged actions */
    struct guesture_manager_action_itr_s *action_itr = manager->action_itr;
    while( action_itr != NULL ){
        struct guesture_manager_action_itr_s *tmp = action_itr;
        if(action_itr->guesture == accept_item->guesture){
            itrack_post_own(manager->post_stage,action_itr->action);
        }else{
            free(action_itr->action);
        }
        action_itr = action_itr->next;
        free(tmp);
    }
    manager->action_itr = NULL;

    LOG_GM("guesture manager accept guesture<%s>\n",accept_item->guesture->name);
    manager->last_guesture = manager->private.accepted_item;
    gettimeofday(&manager->last_guesture_time,NULL);
    manager->private.accepted_item = accept_item;
    guesture_set_post_fn(accept_item->guesture, s_guesture_action_accept_fn,manager);
    manager->private.current_guesture_count = 0;
}

enum get_matched_res_e {
    GET_MATCH_NONE,
    GET_MATCH_MATCH,
    GET_MATCH_CONFLICT
};

static enum get_matched_res_e s_get_matched_guesture(struct guesture_manager_s *manager,struct guesture_item_s **matched){
    /** no essensial guesture */
    /** do common process */
    /** find out the highest priority one ,and current matched one */
    struct guesture_item_s *max_priority_item = NULL;
    struct guesture_item_s *current_ok_item   = NULL;
    int                     current_ok_priority=0;
    for(int i = 0 ; i < manager->private.current_guesture_count ; i ++ ){
        struct guesture_item_s *item = manager->private.current_guesture_item_list[i];
        if( 
            item->used==USING 
            &&( item->guesture->status.match_state == GUESTURE_MATHING  || item->guesture->status.match_state == GUESTURE_OK) 
            && (max_priority_item == NULL || item->priority > max_priority_item->priority)
        ){
            max_priority_item = item;
        }
        if( item->priority > current_ok_priority && item->guesture->status.match_state == GUESTURE_OK){
            current_ok_item     = item;
            current_ok_priority = item->priority;
        }
    }
    /** 
     * if ok guesture have highest priority then accept it 
     * or, needs to wait higher priority one become failed or  ok
    */
    if(current_ok_item == NULL ){
        *matched = NULL;
        return GET_MATCH_NONE;
    }
    if( max_priority_item == current_ok_item ){
        // matches
        *matched = current_ok_item;
        return GET_MATCH_MATCH;
    }else{
        *matched = current_ok_item;
        return GET_MATCH_CONFLICT;
    }
}

void guesture_manager_init(struct guesture_manager_s *manager,struct post_stage_s *post_stage){
    bzero(manager,sizeof(struct guesture_manager_s));
    manager->post_stage = post_stage;
}

int  guesture_manager_add(struct guesture_manager_s *manager,struct guesture_s *guesture,int priority){
    for(int i = 0; i < MAX_GUESTURE_COUNT; i ++ ){
        if( manager->guesture_list[i].used == TRUE && manager->guesture_list[i].priority == priority){
            LOG_ERROR("guesture_manager_add duplicated priority");
            return -1;
        }
    }
    for(int i = 0; i < MAX_GUESTURE_COUNT; i ++ ){
        if( manager->guesture_list[i].used       == FALSE ){
            manager->guesture_list[i].used     = TRUE;
            manager->guesture_list[i].guesture = guesture;
            manager->guesture_list[i].guesture->manager = manager;
            manager->guesture_list[i].priority = priority;
            break;
        }
    }
    return 0;
}

struct guesture_item_s *guesture_manager_find_item_by_name(struct guesture_manager_s *manager,const char *name){
    for(int i = 0; i < MAX_GUESTURE_COUNT; i ++ ){
        if( manager->guesture_list[i].used == TRUE){
            if( strcmp(manager->guesture_list[i].guesture->name, name) == 0){
                return manager->guesture_list + i;
            }
        }
    }
    return NULL;
}

void guesture_manager_touch_start(struct guesture_manager_s *manager,const struct Touch *touch,int touch_count,const struct Touch *touchlist,int touchbit)
{
    manager->private.callback_state = GUESTURE_MANAGER_CALLBACK_START;
    // must no active guesture
    switch(manager->private.state){
    /** not started 0 touch */
    case MANAGER_STATE_NONE:
        manager->private.last_waitting_update = touch->update_time;
        manager->private.state       = MANAGER_STATE_WAITING;
    break;
    /** touch number changed , waitting for debounce*/
    case MANAGER_STATE_WAITING:
        manager->private.last_waitting_update = touch->update_time;
    break;
    /** guesture is updating */
    case MANAGER_STATE_UPDATING:
        // this will cause 
        // won't accept

        if(manager->private.accepted_item){
            // stop 
            s_guesture_end(manager,manager->private.accepted_item,FALSE,touch_count);
        
        }else{
            /** no none is acceptted now */
            // stop all active guesture
            for(int i = 0 ; i < manager->private.current_guesture_count ; i ++ ){
                s_guesture_end(manager,manager->private.current_guesture_item_list[i],FALSE,touch_count);
            }
        
            if(manager->private.accepted_item != NULL){
                /** doing accept one update event */
            }
        }

        manager->private.accepted_item          = NULL;
        manager->private.current_guesture_count = 0;
        manager->private.state                  = MANAGER_STATE_WAITING;
        manager->private.last_waitting_update   = touch->update_time;
        // ready to restart
    break;
    /** from required touch number to un-match touch number */
    case MANAGER_STATE_CHANGING:
        // doing nothing
    break;
    }
    manager->private.max_touches_number = touch_count;
    manager->private.callback_state = GUESTURE_MANAGER_CALLBACK_NONE;
}
void guesture_manager_touch_update(struct guesture_manager_s *manager,const struct Touch *touch,int touch_idx,int touch_count,const struct Touch *touchlist,int touchbit)
{
    manager->private.callback_state = GUESTURE_MANAGER_CALLBACK_UPDATE;
    
    // check if max timeout or max move dist match
    if(manager->private.state == MANAGER_STATE_CHANGING){

        if( time_diff_ms(&touch->update_time,&manager->private.state_changing_until) > 0 ){
            manager->private.state                  = MANAGER_STATE_WAITING;
            manager->private.last_waitting_update   = touch->update_time;
        }else{
            goto end;
        }

    }
    
    if( manager->private.state == MANAGER_STATE_WAITING ){
        if( manager->alt_guesture ){
            if( manager->alt_guesture->guesture->props.required_touches == touch_count ){
                manager->private.current_guesture_item_list[0] = manager->alt_guesture;
                manager->private.current_guesture_count = 1;
                goto guesture_post_init;
            }
        }

        if(touch_idx == 0){
            // time check
            int64_t time_diff = time_diff_ms(&touch->update_time,&manager->private.last_waitting_update);
            if(time_diff > GUESTURE_PRE_WAITING_TIME){
                goto guesture_init;
            }
        }
        if( math_dist2(touch->total_dx,touch->total_dy) < GUESTURE_PRE_WAITTING_DIST * GUESTURE_PRE_WAITTING_DIST){
            goto end;
        }
guesture_init:
        s_get_matched_gesture(
            manager,
            touch_count,
            manager->private.current_guesture_item_list,
            &manager->private.current_guesture_count
        );
guesture_post_init:

        for(int i = 0 ; i < manager->private.current_guesture_count ; i ++ ){
            s_guesture_reset(manager,manager->private.current_guesture_item_list[i]);
            s_guesture_start(manager,manager->private.current_guesture_item_list[i],touchlist,touchbit);
        }
        manager->private.updating_touchbit = touchbit;
        manager->private.state             = MANAGER_STATE_UPDATING;
        manager->private.accepted_item     = NULL;
        /** clear all action */
        struct guesture_manager_action_itr_s *action_itr = manager->action_itr;
        while( action_itr != NULL ){
            struct guesture_manager_action_itr_s *tmp = action_itr;
            free(action_itr -> action);
            action_itr = action_itr->next;
            free(tmp);
        }
        manager->action_itr = NULL;
    }
// handler:
    if(manager->private.state == MANAGER_STATE_UPDATING){
        if(touch_idx == 0){   
            // do normal update
            if( manager->private.accepted_item != NULL ){

                s_guesture_update(manager,manager->private.accepted_item,touchlist,manager->private.updating_touchbit);

            }else{
                for(int i = 0 ; i < manager->private.current_guesture_count ; i ++ ){
                    struct guesture_item_s *item = manager->private.current_guesture_item_list[i];
                    s_guesture_update(manager,item,touchlist,manager->private.updating_touchbit);
                }
            }
        
            if(manager->private.accepted_item != NULL){
                /** doing accept one update event */
            }
        }
    }
    
end:
    manager->private.callback_state = GUESTURE_MANAGER_CALLBACK_NONE;
    return ;
}

void guesture_manager_touch_end(struct guesture_manager_s *manager,const struct Touch *touch,int touch_count,const struct Touch *touchlist,int touchbit){
    manager->private.callback_state = GUESTURE_MANAGER_CALLBACK_END;
    // todo:// wait about some guesture have post forbidden time

    switch(manager->private.state){
    /** not started 0 touch */
    case MANAGER_STATE_NONE:
        // doing nothing
    break;
    /** touch number changed , waitting for debounce*/
    case MANAGER_STATE_WAITING:
        if(touch_count == 1){
            /** instant leave */
            s_get_matched_gesture(
                manager,
                manager->private.max_touches_number,
                manager->private.current_guesture_item_list,
                &manager->private.current_guesture_count
            );

            for(int i = 0 ; i < manager->private.current_guesture_count ; i ++ ){
                s_guesture_reset(manager,manager->private.current_guesture_item_list[i]);
                s_guesture_start(manager,manager->private.current_guesture_item_list[i],touchlist,touchbit);
            }
            manager->private.updating_touchbit = touchbit;
            manager->private.state             = MANAGER_STATE_UPDATING;
            manager->private.accepted_item     = NULL;

            // stop all guesture
            for(int i = 0 ; i < manager->private.current_guesture_count ; i ++ ){
                s_guesture_end(manager,manager->private.current_guesture_item_list[i],FALSE,touch_count);
            }
        
            if(manager->private.accepted_item != NULL){
                /** doing accept one update event */
            }
            manager->private.state = MANAGER_STATE_NONE;
        }else{
            /** is in debounce time */
            manager->private.last_waitting_update = touch->update_time;
        }
    break;
    /** guesture is updating */
    case MANAGER_STATE_UPDATING:
        
        if(manager->private.accepted_item){
            // stop 
            s_guesture_end(manager,manager->private.accepted_item,FALSE,touch_count);
        
        }else{
            /** no none is acceptted now */
            // stop all active guesture
            for(int i = 0 ; i < manager->private.current_guesture_count ; i ++ ){
                s_guesture_end(manager,manager->private.current_guesture_item_list[i],FALSE,touch_count);
            }
        
            if(manager->private.accepted_item != NULL){
                /** doing accept one update event */
            }
        }
        manager->private.accepted_item = NULL;
        manager->private.current_guesture_count = 0;
        if(touch_count == 1){
            manager->private.state         = MANAGER_STATE_NONE;
        }else{
            manager->private.state         = MANAGER_STATE_CHANGING;
            timeraddms(&touch->update_time,GUESTURE_CHANGING_TIME,&manager->private.state_changing_until);
            // ready to restart
        }
    break;
    case MANAGER_STATE_CHANGING:
        if(touch_count == 1){
            manager->private.state         = MANAGER_STATE_NONE;
        }
    break;
    }
    manager->private.callback_state = GUESTURE_MANAGER_CALLBACK_NONE;
}

void guesture_manager_physical_button_update(struct guesture_manager_s *manager,int buttonbit){
    manager->physical_button = buttonbit;
}

bool guesture_manager_is_accept(struct guesture_manager_s *manager,struct guesture_s *guesture){
    if( manager->private.accepted_item ){
        return manager->private.accepted_item->guesture == guesture;
    }
    return FALSE;
}

void guesture_manager_set_guesture_state_change(struct guesture_manager_s *manager,struct guesture_s *guesture){
     if(manager->private.accepted_item == NULL){
        /** doing match one to accept */

        // if someone is essential
        // pick top priority seential one 
        int current_priority = 0;
        struct guesture_item_s *current_item = NULL;
        for(int i = 0 ; i < manager->private.current_guesture_count ; i ++ ){
            struct guesture_item_s *item = manager->private.current_guesture_item_list[i];
            if( item->guesture->status.is_essential ){
                if( item->priority > current_priority ){
                    current_item = item;
                }
            }
        }
        
        if( current_item != NULL ){
            /** essensial guesture detected */
            s_accept_guesture(manager,current_item);
        }else{
            struct guesture_item_s *item;
            enum get_matched_res_e res = s_get_matched_guesture(manager,&item);
            switch (res){
            case GET_MATCH_NONE:
                // doing nothing
            break;
            case GET_MATCH_MATCH:
                s_accept_guesture(manager,item);
            break;
            case GET_MATCH_CONFLICT:
                // needs to wait max priority on ok or failed ??
                // or time out?
                // todo://
            break;
            }
        }
    }
}

void guesture_manager_set_alt(struct guesture_manager_s *manager,struct guesture_item_s *item){
    manager->alt_guesture = item;
}

void guesture_manager_clear_alt(struct guesture_manager_s *manager,struct guesture_item_s *item){
    manager->alt_guesture = NULL;
}

static void apply_on_accept_callback(struct guesture_manager_s *manager,struct guesture_item_s *item){
    for( int i = 0; i < MAX_CALLBACK_COUNT; i++ ){
        if(manager->on_accept_callbacks[i].state == ACCEPT_CALLBACK_USING){
            manager->on_accept_callbacks[i].callback(manager,item,manager->on_accept_callbacks[i].userdata);
        }
    }
}
int guesture_manager_register_on_accept_callback(struct guesture_manager_s *manager,GuestureManagerOnAcceptFn fn,void *userdata){
    for( int i = 0; i < MAX_CALLBACK_COUNT; i++ ){
        if(manager->on_accept_callbacks[i].state == ACCEPT_CALLBACK_EMPTY){
            manager->on_accept_callbacks[i].state = ACCEPT_CALLBACK_USING;
            manager->on_accept_callbacks[i].callback = fn;
            manager->on_accept_callbacks[i].userdata = userdata;
            return 0;
        }
    }
    return 1;
}

int guesture_manager_unregister_on_accept_callback(struct guesture_manager_s *manager,GuestureManagerOnAcceptFn fn,void *userdata){
    for( int i = 0; i < MAX_CALLBACK_COUNT; i++ ){
        if(manager->on_accept_callbacks[i].state == ACCEPT_CALLBACK_USING){
            if(manager->on_accept_callbacks[i].callback == fn && manager->on_accept_callbacks[i].userdata == userdata){
                manager->on_accept_callbacks[i].state = ACCEPT_CALLBACK_EMPTY;
                return 0;
            }
        }
    }
    return 1;
}