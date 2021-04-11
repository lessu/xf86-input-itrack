#ifndef MOVEMENT_H
#define MOVEMENT_H
#define MOVEMENT_CACHE_COUNT 3
#include <time.h>
#include <stdlib.h>


struct movement_action_s{
    int dx,dy;
    struct timeval time;    
};

struct movement_cached_s{
    struct movement_action_s action;
    struct movement_cached_s *next;
};

struct movement_s
{
    struct movement_cached_s *cached;
};

void movement_init(struct movement_s *movement);
void movement_deinit(struct movement_s *movement);

void movement_clear(struct movement_s *movement);
struct movement_action_s movement_add(
    struct movement_s               *movement,
    const struct movement_action_s  *act
);

#endif