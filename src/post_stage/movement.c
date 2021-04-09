#include "movement.h"

void movement_init(struct movement_s *movement){
    movement->cached = NULL;
}

void movement_deinit(struct movement_s *movement){
    // movement_clear(movement);
}

void movement_clear(struct movement_s *movement){
    // struct movement_cached_s *head = &movement->cached;
    // while( head != NULL ){
    //     struct movement_cached_s *item = head;
    //     head = head->next; 
    //     free(item);
    // }
    // movement->cached = NULL;
}

struct movement_action_s movement_add(struct movement_s *movement,const struct movement_action_s *act){
    // /** append to last */
    // struct movement_cached_s **head = &movement->cached;
    // int cachedLength = 0;
    // while( *head != NULL ){
    //     cachedLength ++;
    //     head = &(*head)->next;
    // }
    // (*head) = malloc(sizeof(struct movement_cached_s));
    // (*head)->next = NULL;
    // (*head)->action = *act;
    // /** if cached is full ,remove head one */
    // if (cachedLength > MOVEMENT_CACHE_COUNT){
    //     struct movement_cached_s *head = movement->cached;
    //     movement->cached = movement->cached->next;
    //     free(head);
    // }

    // /** iterate all,make avg*/
    // struct movement_action_s new_act;
    // {
    //     new_act.dx = 0;
    //     new_act.dy = 0;
    //     struct movement_cached_s *head = &movement->cached;
    //     while( head != NULL ){
    //         new_act.dx += head->action.dx / MOVEMENT_CACHE_COUNT;
    //         new_act.dy += head->action.dy / MOVEMENT_CACHE_COUNT;
    //         head = head->next; 
    //     }
    // }

    return *act;
}