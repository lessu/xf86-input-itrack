#pragma once
#include "common.h"
#include "itrack.h"
#include "itrack-type.h"

int itrack_open(itrack_t *itrack, int fd);

int itrack_close(itrack_t *itrack);

bool itrack_read(itrack_t *itrack);
