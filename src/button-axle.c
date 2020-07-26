#include <xorg/xf86.h>
#include "button-axle.h"
#include <xorg/xserver-properties.h>
#include <X11/Xatom.h>
#include <xf86Module.h>
#include "os.h"
#include <xorg/exevents.h>
#include <xorg/xf86Xinput.h>
/* button mapping simplified */
#define PROPMAP(m, x, y) m[x] = XIGetKnownProperty(y)

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
void init_axes_labels(Atom map[NUM_AXES])
{
	memset(map, 0, NUM_AXES * sizeof(Atom));
	PROPMAP(map, 0, AXIS_LABEL_PROP_REL_X);
	PROPMAP(map, 1, AXIS_LABEL_PROP_REL_Y);
	PROPMAP(map, 2, AXIS_LABEL_PROP_REL_VSCROLL);
	PROPMAP(map, 3, AXIS_LABEL_PROP_REL_HSCROLL);
}

void init_button_labels(Atom map[DIM_BUTTON])
{
	memset(map, 0, DIM_BUTTON * sizeof(Atom));
	PROPMAP(map, MT_BUTTON_LEFT, BTN_LABEL_PROP_BTN_LEFT);
	PROPMAP(map, MT_BUTTON_MIDDLE, BTN_LABEL_PROP_BTN_MIDDLE);
	PROPMAP(map, MT_BUTTON_RIGHT, BTN_LABEL_PROP_BTN_RIGHT);
	PROPMAP(map, MT_BUTTON_WHEEL_UP, BTN_LABEL_PROP_BTN_WHEEL_UP);
	PROPMAP(map, MT_BUTTON_WHEEL_DOWN, BTN_LABEL_PROP_BTN_WHEEL_DOWN);
	PROPMAP(map, MT_BUTTON_HWHEEL_LEFT, BTN_LABEL_PROP_BTN_HWHEEL_LEFT);
	PROPMAP(map, MT_BUTTON_HWHEEL_RIGHT, BTN_LABEL_PROP_BTN_HWHEEL_RIGHT);
	/* how to map swipe buttons? */
	PROPMAP(map, MT_BUTTON_SWIPE_UP, BTN_LABEL_PROP_BTN_0);
	PROPMAP(map, MT_BUTTON_SWIPE_DOWN, BTN_LABEL_PROP_BTN_1);
	PROPMAP(map, MT_BUTTON_SWIPE_LEFT, BTN_LABEL_PROP_BTN_2);
	PROPMAP(map, MT_BUTTON_SWIPE_RIGHT, BTN_LABEL_PROP_BTN_3);
	/* how to map scale and rotate? */
	PROPMAP(map, MT_BUTTON_SCALE_DOWN, BTN_LABEL_PROP_BTN_4);
	PROPMAP(map, MT_BUTTON_SCALE_UP, BTN_LABEL_PROP_BTN_5);
	PROPMAP(map, MT_BUTTON_ROTATE_LEFT, BTN_LABEL_PROP_BTN_6);
	PROPMAP(map, MT_BUTTON_ROTATE_RIGHT, BTN_LABEL_PROP_BTN_7);
}
#endif

/**
 * How to handle multitouch: http://www.x.org/wiki/Development/Documentation/Multitouch/
 * Howto about xinput: http://www.x.org/wiki/Development/Documentation/XorgInputHOWTO/
 * Example usage: https://gitlab.com/at-home-modifier/at-home-modifier-evdev/commit/d171b3d9194581cb6ed59dbe45d6cbf009dc0eaa?view=parallel
 * Patch were smooth scrolling were introduced: https://lists.x.org/archives/xorg-devel/2011-September/025835.html
 * @param dev
 * @param axnum
 * @param label
 * @param min
 * @param max
 * @param resolution
 */
void init_axle_absolute(DeviceIntPtr dev, int axnum, Atom* label)
{
	/* Inform server about reported range of axis values. */
	xf86InitValuatorAxisStruct(dev, axnum,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
		*label,
#endif
		/* minval, maxval */ NO_AXIS_LIMITS, NO_AXIS_LIMITS,
		/*resolution*/ 1,
		/*min res*/ 0,
		/*max res*/ 1
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 12
		/* mode */, Absolute
#endif
	);
	xf86InitValuatorDefaults(dev, axnum);
}

void init_axle_relative(DeviceIntPtr dev, int axnum, Atom* label)
{
	xf86InitValuatorAxisStruct(dev, axnum, *label, NO_AXIS_LIMITS, NO_AXIS_LIMITS, 0, 0, 0, Relative);
	xf86InitValuatorDefaults(dev, axnum);
}
