/*
 * Copyright 2007 Peter Hutterer
 * Copyright 2009 Przemys�aw Firszt
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Red Hat
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.  Red
 * Hat makes no representations about the suitability of this software
 * for any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#pragma once
#include <mtdev.h>
#include <xorg/input.h>
#include "mconfig.h"
#include "hwstate.h"
#include "capabilities.h"
#include "mtstate.h"
#include "itrack-type.h"
#include "touch-handler.h"
#include "post-stage.h"

struct itrack_status_s{
	struct MTState state;
} ;

struct itrack_props_s{
	// bool   is_absolute_mode;
	struct MConfig cfg;
	struct Capabilities caps;
} ;

struct itrack_private_s{
	touch_handler_t 	touch;
	struct post_stage_s post_stage_handler;
} ;

typedef struct {
	int fd;
	struct mtdev dev;
	DeviceIntPtr local_dev;
	struct HWState hs;
	
	struct itrack_status_s status;

	struct itrack_props_s  props;

	struct itrack_private_s private;
	
	// struct Gestures gs;
	// int timer_kind;   // to avoid setting timer multiple times
	/*
	 * Timers documentation:
	 * http://www.x.org/releases/X11R7.7/doc/xorg-server/Xserver-spec.html#id2536042
	 */
	// OsTimerPtr timer;
	// int is_absolute_mode;		// Should absolute mode be enabled ? 0 or 1
	ValuatorMask* valuator_mask;
} itrack_t;