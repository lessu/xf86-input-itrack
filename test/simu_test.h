#pragma once
#include "mtstate.h"
#include "guesture/guesture_manager.h"

// #define MT_NEW 0
// #define MT_RELEASED 1
// #define MT_INVALID 2
// #define MT_THUMB 3
// #define MT_PALM 4
// #define MT_EDGE 5
struct multi_touch_s{
    struct Touch  touches[5];
    int           touch_count;
    uint64_t      offset_unit;
};

void simu_test_guesture_manager_init(struct guesture_manager_s *manager);
void simu_test_guesture_manager_deinit(struct guesture_manager_s *manager);

void send_touch_sequence(struct guesture_manager_s *manager,struct multi_touch_s *multi_touch_list,int count);

int touch_create_new(struct Touch *touch,int x,int y,int seq_num);
void touch_update_no_change(struct Touch *touch);
void touch_update_offset(struct Touch *touch,int dx,int dy);
void touch_update_release(struct Touch *touch);

struct touch_sequence_builder_s{
    int seqence_length;
    int current_seq_num;
    struct multi_touch_s *sequence;
};
void touch_sequence_builder_init(struct touch_sequence_builder_s *builder);
void touch_sequence_builder_add(struct touch_sequence_builder_s *builder,const struct Touch *touch);
void touch_sequence_builder_next(struct touch_sequence_builder_s *builder);
void touch_sequence_builder_add_and_next(struct touch_sequence_builder_s *builder,const struct Touch *touch);
void touch_sequence_builder_release(struct touch_sequence_builder_s *builder);