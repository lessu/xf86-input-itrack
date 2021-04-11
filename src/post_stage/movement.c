#include "movement.h"
#include "common.h"

void movement_init(struct movement_s *movement){
    movement->cached = NULL;
}

void movement_deinit(struct movement_s *movement){
    movement_clear(movement);
}

void movement_clear(struct movement_s *movement){
    struct movement_cached_s *head = movement->cached;
    while( head != NULL ){
        struct movement_cached_s *item = head;
        head = head->next; 
        free(item);
    }
    movement->cached = NULL;
}

struct movement_action_s movement_add(struct movement_s *movement,const struct movement_action_s *act){
    // if cached action is too old,clear them
    if(movement->cached != NULL){
        int diff = time_diff_ms(&movement->cached->action.time,&act->time);
        if(diff < 20 * MOVEMENT_CACHE_COUNT){
            movement_clear(movement);
        }
    }

    /** append to last */
    struct movement_cached_s **head = &movement->cached;
    int cachedLength = 0;
    while( *head != NULL ){
        cachedLength ++;
        head = &(*head)->next;
    }
    (*head) = malloc(sizeof(struct movement_cached_s));
    (*head)->next = NULL;
    (*head)->action = *act;


    struct movement_action_s new_act;
    new_act.dx = 0;
    new_act.dy = 0;
    
    struct movement_cached_s *oldest = movement->cached;
    if(oldest != NULL){

        /** if cached is full ,remove head one */
        if (cachedLength > MOVEMENT_CACHE_COUNT){
            movement->cached = movement->cached->next;
        }
    
        new_act = oldest->action;

        if (cachedLength > MOVEMENT_CACHE_COUNT){
            free(oldest);
        }
    }

    /** todo:// filter tiny movement */


    return new_act;

    // /** iterate all,make avg*/
    // struct movement_action_s new_act;
    // new_act.dx = 0;
    // new_act.dy = 0;
    // struct movement_cached_s *node = movement->cached;
    // while( node != NULL ){
    //     new_act.dx += node->action.dx / MOVEMENT_CACHE_COUNT;
    //     new_act.dy += node->action.dy / MOVEMENT_CACHE_COUNT;
    //     node = node->next; 
    // }

    // return new_act;
}