#include "test_tap.h"
#include "simu_test.h"
#include "guesture/tap.h"
#include "guesture/move.h"
#define TOUCH_SEQUENCE_COUNT 3
static void touch_up(struct touch_sequence_builder_s *builder,struct Touch *touch){
    touch_update_release(touch);
    touch_sequence_builder_add_and_next(builder,touch);
}
static void touch_down(struct touch_sequence_builder_s *builder,struct Touch *touch){
    touch_create_new(touch,50,50,0);
    touch_sequence_builder_add_and_next(builder,touch);
}
static void move(struct touch_sequence_builder_s *builder,struct Touch *touch,int dx,int dy){
    touch_update_offset(touch,dx,dy);
    touch_sequence_builder_add_and_next(builder,touch);
}
void test_tap( void ){
    struct guesture_manager_s manager;
    simu_test_guesture_manager_init(&manager);

    struct touch_sequence_builder_s builder;
    struct Touch touch;
    touch_sequence_builder_init(&builder);
    
    touch_down(&builder,&touch);
    move(&builder,&touch,2,-1);
    move(&builder,&touch,1,1);
    move(&builder,&touch,1,0);
    move(&builder,&touch,1,-1);
    move(&builder,&touch,-2,2);
    touch_up(&builder,&touch);

    send_touch_sequence(&manager,builder.sequence,builder.current_seq_num);

    touch_sequence_builder_release(&builder);
}