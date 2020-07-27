#include "guesture.h"
#include "guesture_manager.h"

void guesture_init(struct guesture_s *guesture,const char *name,const struct guesture_callbacks_s *callbacks,void* userdata){
    bzero(guesture,sizeof(struct guesture_s));
    strncpy(guesture->name,name,MAX_GUESTURE_NAME_LENGTH);
    guesture->callbacks = *callbacks;
}

void guesture_set_match(struct guesture_s *guesture,enum guesture_set_match_status_e set_match,struct timeval time){
    assert(guesture->status.match_state == GUESTURE_MATHING);
    guesture->status.state_update_time = time;
    switch (set_match){
    case GUESTURE_SET_MATCH_STATUS_ESSENTIAL:
        guesture->status.is_essential = TRUE;
        guesture->status.match_state= GUESTURE_OK;
        LOG_DEBUG("guesture<%p> ask essential\n",guesture);
    break;
    case GUESTURE_SET_MATCH_STATUS_MATCH:
        guesture->status.match_state= GUESTURE_OK;
        LOG_DEBUG("guesture<%p> ask match\n",guesture);
    break;
    case GUESTURE_SET_MATCH_STATUS_NOT_MATCH:
        guesture->status.match_state= GUESTURE_FAILED;
        LOG_DEBUG("guesture<%p> ask not match\n",guesture);
    break;
    }
    guesture_manager_set_guesture_state_change(guesture->manager,guesture);
}

// Bool guesture_set_alt(struct guesture_s *guesture){

// }

// void guesture_clear_alt(struct guesture_s *guesture){

// }