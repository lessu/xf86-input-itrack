#include "rotate.h"

static void on_start(struct rotate_guesture_s *guesture){

}

static void on_update(struct rotate_guesture_s *guesture){

}

static Bool on_end(struct rotate_guesture_s *guesture){

}

static struct guesture_callbacks_s s_callbacks = {
    .on_start   = on_start,
    .on_update = on_update,
    .on_end    = on_end
};

void rotate_guesture_init(struct rotate_guesture_s *guesture){
    guesture_init(&guesture->guesture,2,0,&s_callbacks,guesture);
}