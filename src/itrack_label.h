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
#ifndef ITRACK_LABEL_H
#define ITRACK_LABEL_H
#include <X11/X.h>
#include <X11/Xmd.h>

#define DIM_BUTTON 15

#define IT_BUTTON_LEFT 0
#define IT_BUTTON_MIDDLE 1
#define IT_BUTTON_RIGHT 2
#define IT_BUTTON_WHEEL_UP 3
#define IT_BUTTON_WHEEL_DOWN 4
#define IT_BUTTON_HWHEEL_LEFT 5
#define IT_BUTTON_HWHEEL_RIGHT 6
#define IT_BUTTON_SWIPE_UP 7
#define IT_BUTTON_SWIPE_DOWN 8
#define IT_BUTTON_SWIPE_LEFT 9
#define IT_BUTTON_SWIPE_RIGHT 10
#define IT_BUTTON_SCALE_DOWN 11
#define IT_BUTTON_SCALE_UP 12
#define IT_BUTTON_ROTATE_LEFT 13
#define IT_BUTTON_ROTATE_RIGHT 14


#define NUM_AXES 4

void itrack_label_axes_init(Atom *map);
void itrack_label_button_init(Atom *map,CARD8 *btmap);

#endif /** ITRACK_LABEL_H */