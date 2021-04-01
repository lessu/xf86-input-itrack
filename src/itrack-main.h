#pragma once
#include "common.h"

struct itrack;

int itrack_open(struct itrack *itrack, int fd);

int itrack_close(struct itrack *itrack);

BOOL itrack_read(struct itrack *itrack);
