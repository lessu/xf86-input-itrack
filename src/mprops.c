/***************************************************************************
 *
 * Multitouch X driver
 * Copyright (C) 2011 Ryan Bourgeois <bluedragonx@gmail.com>
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

#include "mprops.h"
#include "common.h"
#include "itrack.h"
#define LOG_DEBUG_PROPS LOG_NULL

#define MAX_INT_VALUES 7
#define MAX_FLOAT_VALUES 4
#define MAX_BUTTON_VALUES 6

#define VALID_BUTTON(x) (x >= 0 && x <= 32)
#define VALID_BOOL(x) (x == 0 || x == 1)
#define VALID_PCNT(x) (x >= 0 && x <= 100)

struct MProps mprops;

static
Atom atom_init_integer(DeviceIntPtr dev, const char* name, int nvalues, int* values, int size) {
	Atom atom;
	int i;
	uint8_t uvals8[MAX_INT_VALUES];
	uint16_t uvals16[MAX_INT_VALUES];
	uint32_t uvals32[MAX_INT_VALUES];
	pointer uvals;
	nvalues = MINVAL(MAX_INT_VALUES, nvalues);

	switch(size) {
	case 8:
		for (i = 0; i < nvalues; i++) {
			uvals8[i] = values[i];
		}
		uvals = uvals8;
		break;
	case 16:
		for (i = 0; i < nvalues; i++) {
			uvals16[i] = values[i];
		}
		uvals = uvals16;
		break;
	default:
		for (i = 0; i < nvalues; i++) {
			uvals32[i] = values[i];
		}
		uvals = uvals32;
		break;
	}

	atom = MakeAtom(name, strlen(name), TRUE);
	XIChangeDeviceProperty(dev, atom, XA_INTEGER, size, PropModeReplace, nvalues, uvals, FALSE);
	XISetDevicePropertyDeletable(dev, atom, FALSE);
	return atom;
}

static
Atom atom_init_integers32(DeviceIntPtr dev, char* name, int nvalues, int* values) {
	int size = 32;
	Atom atom;

	atom = MakeAtom(name, strlen(name), TRUE);
	XIChangeDeviceProperty(dev, atom, XA_INTEGER, size, PropModeReplace, nvalues, values, FALSE);
	XISetDevicePropertyDeletable(dev, atom, FALSE);
	return atom;
}


static 
Atom atom_init_float(DeviceIntPtr dev, char* name, int nvalues, float* values, Atom float_type) {
	Atom atom = MakeAtom(name, strlen(name), TRUE);
	XIChangeDeviceProperty(dev, atom, float_type, 32, PropModeReplace, nvalues, values, FALSE);
	XISetDevicePropertyDeletable(dev, atom, FALSE);
	return atom;
}

void mprops_init(struct MConfig* cfg, InputInfoPtr local) {
	int ivals[MAX_INT_VALUES];
	// float fvals[MAX_FLOAT_VALUES];

	mprops.float_type = XIGetKnownProperty(XATOM_FLOAT);
	if (!mprops.float_type) {
		mprops.float_type = MakeAtom(XATOM_FLOAT, strlen(XATOM_FLOAT), TRUE);
		if (!mprops.float_type) {
			LOG_ERROR("%s: Failed to init float atom. Property support is disabled.\n", local->name);
			return;
		}
	}

	ivals[0] = cfg->trackpad_disable;
	mprops.trackpad_disable = atom_init_integer(local->dev, ITRACK_PROP_TRACKPAD_DISABLE, 1, ivals, 8);

	// ivals[0] = cfg->touch_down;
	// ivals[1] = cfg->touch_up;
	// mprops = atom_init_integer(local->dev, ITRACK_PROP_THUMB_DETECT, 2, ivals, 8);
	
	ivals[0] = cfg->ignore_thumb;
	ivals[1] = cfg->disable_on_thumb;
	mprops.thumb_detect = atom_init_integer(local->dev, ITRACK_PROP_THUMB_DETECT, 2, ivals, 8);

	ivals[0] = cfg->thumb_size;
	ivals[1] = cfg->thumb_ratio;
	mprops.thumb_size = atom_init_integer(local->dev, ITRACK_PROP_THUMB_SIZE, 2, ivals, 32);

	ivals[0] = cfg->ignore_palm;
	ivals[1] = cfg->disable_on_palm;
	mprops.palm_detect = atom_init_integer(local->dev, ITRACK_PROP_PALM_DETECT, 2, ivals, 8);

	ivals[0] = cfg->palm_size;
	mprops.palm_size = atom_init_integer(local->dev, ITRACK_PROP_PALM_SIZE, 1, ivals, 32);

	ivals[0] = cfg->edge_top_size;
	ivals[1] = cfg->edge_bottom_size;
	ivals[2] = cfg->edge_left_size;
	ivals[3] = cfg->edge_right_size;
	mprops.edge_sizes = atom_init_integer(local->dev, ITRACK_PROP_EDGE_SIZES, 4, ivals, 8);

	ivals[0] = cfg->axis_x_invert;
	ivals[1] = cfg->axis_y_invert;
	mprops.axis_invert = atom_init_integer(local->dev, ITRACK_PROP_AXIS_INVERT, 2, ivals, 8);
}

int mprops_set_property(DeviceIntPtr dev, Atom property, XIPropertyValuePtr prop, BOOL checkonly) {
	InputInfoPtr local = dev->public.devicePrivate;
	struct MConfig* cfg = &((itrack_t*)local->private)->props.cfg;

	uint8_t* ivals8;
	// uint16_t* ivals16;
	uint32_t* ivals32;
	// float* fvals;

	// int error_code;
	if (property == mprops.trackpad_disable) {
		if (prop->size != 1 || prop->format != 8 || prop->type != XA_INTEGER)
			return BadMatch;

		ivals8 = (uint8_t*)prop->data;
		if (ivals8[0] < 0 || ivals8[0] > 3)
			return BadMatch;

		if (!checkonly) {
			cfg->trackpad_disable = ivals8[0];
			if (cfg->trackpad_disable)
				LOG_DEBUG_PROPS("trackpad input disabled\n");
			else
				LOG_DEBUG_PROPS("trackpad input enabled\n");
		}
	}
	else if (property == mprops.thumb_detect) {
		if (prop->size != 2 || prop->format != 8 || prop->type != XA_INTEGER)
			return BadMatch;

		ivals8 = (uint8_t*)prop->data;
		if (!VALID_BOOL(ivals8[0]) || !VALID_BOOL(ivals8[1]))
			return BadMatch;

		if (!checkonly) {
			cfg->ignore_thumb = ivals8[0];
			cfg->disable_on_thumb = ivals8[1];
			LOG_DEBUG_PROPS("set thumb detect to %d %d\n",
				cfg->ignore_thumb, cfg->disable_on_thumb);
		}
	}
	else if (property == mprops.thumb_size) {
		if (prop->size != 2 || prop->format != 32 || prop->type != XA_INTEGER)
			return BadMatch;

		ivals32 = (uint32_t*)prop->data;
		if (ivals32[0] < 0 || !VALID_PCNT(ivals32[1]))
			return BadMatch;

		if (!checkonly) {
			cfg->thumb_size = ivals32[0];
			cfg->thumb_ratio = ivals32[0];
			LOG_DEBUG_PROPS("set thumb size to %d %d\n",
				cfg->thumb_size, cfg->thumb_ratio);
		}
	}
	else if (property == mprops.palm_detect) {
		if (prop->size != 2 || prop->format != 8 || prop->type != XA_INTEGER)
			return BadMatch;

		ivals8 = (uint8_t*)prop->data;
		if (!VALID_BOOL(ivals8[0]) || !VALID_BOOL(ivals8[1]))
			return BadMatch;

		if (!checkonly) {
			cfg->ignore_palm = ivals8[0];
			cfg->disable_on_palm = ivals8[1];
			LOG_DEBUG_PROPS("set palm detect to %d %d\n",
				cfg->ignore_palm, cfg->disable_on_palm);
		}
	}
	else if (property == mprops.palm_size) {
		if (prop->size != 1 || prop->format != 32 || prop->type != XA_INTEGER)
			return BadMatch;

		ivals32 = (uint32_t*)prop->data;
		if (ivals32[0] < 0)
			return BadMatch;

		if (!checkonly) {
			cfg->palm_size = ivals32[0];
			LOG_DEBUG_PROPS("set palm size to %d\n",
				cfg->palm_size);
		}
	}
	else if (property == mprops.axis_invert) {
		if (prop->size != 2 || prop->format != 8 || prop->type != XA_INTEGER)
			return BadMatch;

		ivals8 = (uint8_t*)prop->data;
		if (!VALID_BOOL(ivals8[0]) || !VALID_BOOL(ivals8[1]))
			return BadMatch;

		if (!checkonly) {
			cfg->axis_x_invert = ivals8[0];
			cfg->axis_y_invert = ivals8[1];
			LOG_DEBUG_PROPS("set axis inversion to %d %d\n",
				cfg->axis_x_invert, cfg->axis_y_invert);
		}
	}
	return Success;
}