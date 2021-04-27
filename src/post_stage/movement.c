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
        struct movement_cached_s *p = movement->cached;
        while( p->next != NULL ){
            p = p->next;
        }
        int diff = time_diff_ms(&act->time,&p->action.time);
        if(diff > 20 * MOVEMENT_CACHE_COUNT){
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

        new_act = oldest->action;

        /** if cached is full ,remove head one */
        if (cachedLength > MOVEMENT_CACHE_COUNT){
            movement->cached = movement->cached->next;
            free(oldest);
        }
    }

    /** todo:// filter tiny movement */
    // sum_all dx,dy
    struct movement_cached_s *p = movement->cached;
    float sum_dx = 0,sum_dy = 0;
    while( p != NULL ){
        sum_dx = sum_dx + p->action.dx;
        sum_dy = sum_dy + p->action.dy;
        p = p->next;
    }

    if((sum_dx < 2 && sum_dx>-2) && (sum_dy < 2 && sum_dy > -2)){
        new_act.dx = 0;
        new_act.dy = 0;
    }

    // todo:// slow down pointer move when dx/dy is small enough
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