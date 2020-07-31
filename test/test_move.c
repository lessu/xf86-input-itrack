#include "test_move.h"
#include "simu_test.h"
#include "guesture/move.h"
#define TOUCH_SEQUENCE_COUNT 3

void test_move( void ){
    struct guesture_manager_s manager;
    struct touch_sequence_builder_s builder;

    simu_test_guesture_manager_init(&manager);

    struct Touch touch;
    touch_sequence_builder_init(&builder);
    touch_create_new(&touch,50,50,0);
    touch_sequence_builder_add_and_next(&builder,&touch);
    touch_update_offset(&touch,10,10);
    touch_sequence_builder_add_and_next(&builder,&touch);
    touch_update_offset(&touch,10,10);
    touch_sequence_builder_add_and_next(&builder,&touch);
    touch_update_offset(&touch,10,10);
    touch_sequence_builder_add_and_next(&builder,&touch);
    touch_update_offset(&touch,10,10);
    touch_sequence_builder_add_and_next(&builder,&touch);
    touch_update_offset(&touch,10,10);
    touch_sequence_builder_add_and_next(&builder,&touch);
    touch_update_offset(&touch,10,10);
    touch_sequence_builder_add_and_next(&builder,&touch);
    touch_update_offset(&touch,10,10);
    touch_sequence_builder_add_and_next(&builder,&touch);
    touch_update_offset(&touch,10,10);
    touch_sequence_builder_add_and_next(&builder,&touch);
    touch_update_offset(&touch,10,10);
    touch_sequence_builder_add_and_next(&builder,&touch);
    touch_update_offset(&touch,10,10);
    touch_sequence_builder_add_and_next(&builder,&touch);
    touch_update_release(&touch);
    touch_sequence_builder_add_and_next(&builder,&touch);
    send_touch_sequence(&manager,builder.sequence,builder.current_seq_num);

    touch_sequence_builder_release(&builder);

    simu_test_guesture_manager_deinit(&manager);
}