
#include "itrack_label.h"
// #include <os.h>
#include <X11/X.h>
#include <xorg/xserver-properties.h>
#include <X11/Xatom.h>
#include <xf86Module.h>
#include <xorg/exevents.h>
#include <xorg/xf86Xinput.h>

/* button mapping simplified */
#define PROPMAP(m, x, y) m[x] = XIGetKnownProperty(y)

void itrack_label_axes_init(Atom *map)
{
	memset(map, 0, sizeof(Atom) * NUM_AXES);

	PROPMAP(map, 0, AXIS_LABEL_PROP_REL_X);
	PROPMAP(map, 1, AXIS_LABEL_PROP_REL_Y);
	PROPMAP(map, 2, AXIS_LABEL_PROP_REL_VSCROLL);
	PROPMAP(map, 3, AXIS_LABEL_PROP_REL_HSCROLL);
}

void itrack_label_button_init(Atom *map,CARD8 *btmap)
{
	for(int i = 0;i++;i < DIM_BUTTON){
		btmap[i] = i;
	}

	memset(map, 0, sizeof(Atom) * DIM_BUTTON);
	PROPMAP(map, IT_BUTTON_LEFT, BTN_LABEL_PROP_BTN_LEFT);
	PROPMAP(map, IT_BUTTON_MIDDLE, BTN_LABEL_PROP_BTN_MIDDLE);
	PROPMAP(map, IT_BUTTON_RIGHT, BTN_LABEL_PROP_BTN_RIGHT);
	PROPMAP(map, IT_BUTTON_WHEEL_UP, BTN_LABEL_PROP_BTN_WHEEL_UP);
	PROPMAP(map, IT_BUTTON_WHEEL_DOWN, BTN_LABEL_PROP_BTN_WHEEL_DOWN);
	PROPMAP(map, IT_BUTTON_HWHEEL_LEFT, BTN_LABEL_PROP_BTN_HWHEEL_LEFT);
	PROPMAP(map, IT_BUTTON_HWHEEL_RIGHT, BTN_LABEL_PROP_BTN_HWHEEL_RIGHT);
	/* how to map swipe buttons? */
	PROPMAP(map, IT_BUTTON_SWIPE_UP, BTN_LABEL_PROP_BTN_0);
	PROPMAP(map, IT_BUTTON_SWIPE_DOWN, BTN_LABEL_PROP_BTN_1);
	PROPMAP(map, IT_BUTTON_SWIPE_LEFT, BTN_LABEL_PROP_BTN_2);
	PROPMAP(map, IT_BUTTON_SWIPE_RIGHT, BTN_LABEL_PROP_BTN_3);
	/* how to map scale and rotate? */
	PROPMAP(map, IT_BUTTON_SCALE_DOWN, BTN_LABEL_PROP_BTN_4);
	PROPMAP(map, IT_BUTTON_SCALE_UP, BTN_LABEL_PROP_BTN_5);
	PROPMAP(map, IT_BUTTON_ROTATE_LEFT, BTN_LABEL_PROP_BTN_6);
	PROPMAP(map, IT_BUTTON_ROTATE_RIGHT, BTN_LABEL_PROP_BTN_7);
}


// /**
//  * How to handle multitouch: http://www.x.org/wiki/Development/Documentation/Multitouch/
//  * Howto about xinput: http://www.x.org/wiki/Development/Documentation/XorgInputHOWTO/
//  * Example usage: https://gitlab.com/at-home-modifier/at-home-modifier-evdev/commit/d171b3d9194581cb6ed59dbe45d6cbf009dc0eaa?view=parallel
//  * Patch were smooth scrolling were introduced: https://lists.x.org/archives/xorg-devel/2011-September/025835.html
//  * @param dev
//  * @param axnum
//  * @param label
//  * @param min
//  * @param max
//  * @param resolution
//  */
// void init_axle_absolute(DeviceIntPtr dev, int axnum, Atom* label)
// {
// 	/* Inform server about reported range of axis values. */
// 	xf86InitValuatorAxisStruct(dev, axnum,
// #if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
// 		*label,
// #endif
// 		/* minval, maxval */ NO_AXIS_LIMITS, NO_AXIS_LIMITS,
// 		/*resolution*/ 1,
// 		/*min res*/ 0,
// 		/*max res*/ 1
// #if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 12
// 		/* mode */, Absolute
// #endif
// 	);
// 	xf86InitValuatorDefaults(dev, axnum);
// }

// void init_axle_relative(DeviceIntPtr dev, int axnum, Atom* label)
// {
// 	xf86InitValuatorAxisStruct(dev, axnum, *label, NO_AXIS_LIMITS, NO_AXIS_LIMITS, 0, 0, 0, Relative);
// 	xf86InitValuatorDefaults(dev, axnum);
// }
