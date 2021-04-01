/***************************************************************************
 *
 * I-Track Xf86
 * Copyright (C) 2020 Mingyi Li <lessu@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 **************************************************************************/

#pragma once
#include "xconfig.h"

struct itrack_config_s{

    int tap_dist;
    int tap_hold;
    int tap_release;
    
};

// typedef struct itrack_config_s ITrackConfig;

void itrack_config_init(struct itrack_config_s *config,DeviceIntPtr dev,XF86OptionPtr opt);

void itrack_config_deinit(DeviceIntPtr dev);