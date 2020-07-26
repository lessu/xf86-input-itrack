/***************************************************************************
 *
 * Multitouch X driver
 * Copyright (C) 2008 Henrik Rydberg <rydberg@euromail.se>
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

#ifndef BUTTON_AXLE_H
#define BUTTON_AXLE_H
#include <xorg/xf86.h>

#define DIM_BUTTON 15

#define MT_BUTTON_LEFT 0
#define MT_BUTTON_MIDDLE 1
#define MT_BUTTON_RIGHT 2
#define MT_BUTTON_WHEEL_UP 3
#define MT_BUTTON_WHEEL_DOWN 4
#define MT_BUTTON_HWHEEL_LEFT 5
#define MT_BUTTON_HWHEEL_RIGHT 6
#define MT_BUTTON_SWIPE_UP 7
#define MT_BUTTON_SWIPE_DOWN 8
#define MT_BUTTON_SWIPE_LEFT 9
#define MT_BUTTON_SWIPE_RIGHT 10
#define MT_BUTTON_SCALE_DOWN 11
#define MT_BUTTON_SCALE_UP 12
#define MT_BUTTON_ROTATE_LEFT 13
#define MT_BUTTON_ROTATE_RIGHT 14


#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7

#define NUM_AXES 4

void init_axes_labels(Atom map[NUM_AXES]);
void init_button_labels(Atom map[DIM_BUTTON]);

#endif

void init_axle_absolute(DeviceIntPtr dev, int axnum, Atom* label);

void init_axle_relative(DeviceIntPtr dev, int axnum, Atom* label);

#endif