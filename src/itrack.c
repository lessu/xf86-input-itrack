/***************************************************************************
 *
 * Multitouch X driver
 * Copyright (C) 2008 Henrik Rydberg <rydberg@euromail.se>
 * Copyright (C) 2011 Ryan Bourgeois <bluedragonx@gmail.com>
 * Copyright (C) 2015-2018 Paweł Turkowski <p2rkw0@gmail.com>
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
#include <xorg/xf86.h>
#include <xorg/xf86Module.h>
#include <xorg/xf86Xinput.h>
#include <xorg/exevents.h>
#include <xorg/xorg-server.h>
#include <xorg/xf86_OSproc.h>

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
#include <X11/Xatom.h>
#include <xorg/xserver-properties.h>
#endif
#include "config.h"

#include "itrack.h"
#include "itrack_label.h"
#include "mprops.h"
#include "itrack-main.h"
#include "itrack-config.h"
#include "debug.h"
#define LOG_DEBUG_DRIVER LOG_NULL

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) < 7
	#error "Unsupported ABI_XINPUT_VERSION"
#endif

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 12
typedef InputInfoPtr LocalDevicePtr;
#endif

// static void s_print_action(const struct itrack_action_s *action){
	// char log[2048] = {0};
	// BOOL have_change = FALSE;
	// sprintf(log,"action={\n");
	// if(action->pointer.x != 0 || action->pointer.y != 0){
	// 	sprintf(log+strlen(log),"    pointer=(%d,%d)\n",action->pointer.x,action->pointer.y);
	// 	have_change = TRUE;
	// }
	// sprintf(log+strlen(log),"}\n");
	// if(have_change){
	// 	LOG_DEBUG("%s",log);
	// }
// }


static void s_pointer_control(DeviceIntPtr dev, PtrCtrl *ctrl)
{
	LOG_DEBUG_DRIVER("s_pointer_control\n");
}

static void s_keyboard_control(DeviceIntPtr device,KeybdCtrl *ctrl )
{
	LOG_DEBUG_DRIVER("s_keyboard_control\n");
}

static int device_init(DeviceIntPtr dev, LocalDevicePtr local)
{
	struct itrack *itrack = local->private;

	CARD8 btmap[DIM_BUTTON];
	Atom axes_labels[NUM_AXES];
	Atom btn_labels[DIM_BUTTON];
	itrack_label_axes_init(axes_labels);
	itrack_label_button_init(btn_labels,btmap);

	if(!InitPointerDeviceStruct(
			(DevicePtr)dev,
			btmap, DIM_BUTTON, btn_labels,
			s_pointer_control,
			GetMotionHistorySize(),
			NUM_AXES, axes_labels
		)
	){
		LOG_ERROR("InitPointerDeviceStruct for itrack failed\n");
		return BadRequest;
	}
	/* 
	 * Always set valuator distance to 1.0 because it reported values will be
	 * adjusted accordingly by smooth scroll trigger.
	 */
	SetScrollValuator(dev, 2, SCROLL_TYPE_VERTICAL,   1.0, SCROLL_FLAG_PREFERRED);
	SetScrollValuator(dev, 3, SCROLL_TYPE_HORIZONTAL, 1.0, SCROLL_FLAG_NONE);


	// if(!InitKeyboardDeviceStruct(
	// 	dev,
	// 	NULL,NULL,
	// 	s_keyboard_control
	// )){
	// 	LOG_ERROR("InitKeyboardDeviceStruct for itrack failed");
	// 	return BadRequest;
	// }


	local->fd = xf86OpenSerial(local->options);
	if (local->fd < 0) {
		LOG_ERROR("cannot open device %s\n", local->name);
		return !Success;
	}
	int rc = read_capabilities(&itrack->props.caps, local->fd);
	if (rc < 0){
		LOG_ERROR("cannot read capabilities\n");
	}
	output_capabilities(&itrack->props.caps);
	
	xf86CloseSerial(local->fd);

	local->fd = 0;

	mconfig_update_caps(&itrack->props.cfg, &itrack->props.caps);
	mprops_init(&itrack->props.cfg, local);
	XIRegisterPropertyHandler(dev, mprops_set_property, NULL, NULL);

	struct itrack_config_s config;
	itrack_config_init(&config,dev,local->options);

#ifdef DEBUG_FIFO
	debug_fifo_enable();
#endif 

	return Success;
}

static int device_on(LocalDevicePtr local)
{
	struct itrack *itrack = local->private;
	local->fd = xf86OpenSerial(local->options);
	if (local->fd < 0) {
		LOG_ERROR("cannot open device\n");
		return !Success;
	}
	if (itrack_open(itrack, local->fd)) {
		LOG_ERROR("cannot itrack open\n");
		return !Success;
	}
	/*
	 *  xf86AddEnabledDevice() will add our device's fd to the list of SIGIO handlers.
	 *  When a SIGIO occurs, our read_input will get called.
	 */
	xf86AddEnabledDevice(local);
	return Success;
}

static int device_off(LocalDevicePtr local)
{
	struct itrack *itrack = local->private;
	xf86RemoveEnabledDevice(local);
	if (itrack_close(itrack))
		LOG_WARNING("cannot ungrab device\n");
	xf86CloseSerial(local->fd);
	return Success;
}

static int device_close(LocalDevicePtr local){
	itrack_config_deinit(local->dev);
	return Success;
}

/*
 *  Called for each full received packet from the touchpad.
 * Any xf86 input event generated by int this callback function fill be queued by
 * xorg server, and fired when control return from this function.
 * So to fire event as early as possible this function should return quickly.
 * For delayed events we can't simply wait in this function, because it will delay
 * all events generated by 'post_gestures'.
 * Moreover we don't know when next input event will occur, so to guarantee proper
 * timing I have to use timer.
 *
 * More on input event processing:
 * http://www.x.org/wiki/Development/Documentation/InputEventProcessing/
 *
 * HowTo:
 * https://www.x.org/wiki/Development/Documentation/XorgInputHOWTO/
 */
static void read_input(LocalDevicePtr local)
{
	struct itrack *itrack = local->private;
	// int timer_kind;

	itrack->local_dev = local->dev;
	while ( itrack_read(itrack) ){
		// s_print_action(&status);
		while ( itrack_post_read( &itrack->private.post_stage_handler , itrack->local_dev , &itrack->hs.evtime) != 0){
			// pass
		}
	}
}

static int switch_mode(ClientPtr client, DeviceIntPtr dev, int mode)
{
	// LocalDevicePtr local = dev->public.devicePrivate;
	// struct itrack *itrack = local->private;

	// switch (mode) {
	// case Absolute:
	// 	itrack->props.is_absolute_mode = TRUE;
	// 	LOG_INFO("switch_mode: switing to absolute mode\n");
	// 	break;
	// case Relative:
		// itrack->props.is_absolute_mode = FALSE;
	// 	LOG_INFO("switch_mode: switing to relative mode\n");
	// 	break;
	// default:
	// 	return XI_BadMode;
	// }
	return Success;
}


static Bool device_control(DeviceIntPtr dev, int mode)
{
	LocalDevicePtr local = dev->public.devicePrivate;
	switch (mode) {
	case DEVICE_INIT:
		LOG_INFO("device control: init\n");
		return device_init(dev, local);
	case DEVICE_ON:
		LOG_INFO("device control: on\n");
		return device_on(local);
	case DEVICE_OFF:
		LOG_INFO("device control: off\n");
		return device_off(local);
	case DEVICE_CLOSE:
		LOG_INFO("device control: close\n");
		return device_close(local);
	default:
		LOG_INFO("device control: default\n");
		return BadValue;
	}
}

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 12
static int preinit(InputDriverPtr drv, InputInfoPtr pInfo, int flags)
{
	struct itrack *itrack;

	itrack = calloc(1, sizeof(*itrack));
	if (!itrack)
		return BadAlloc;

	pInfo->private = itrack;
	pInfo->type_name = XI_TOUCHPAD;
	pInfo->device_control = device_control;
	pInfo->read_input = read_input;
	pInfo->switch_mode = switch_mode;

	xf86CollectInputOptions(pInfo, NULL);
	xf86OptionListReport(pInfo->options);
	xf86ProcessCommonOptions(pInfo, pInfo->options);
	mconfig_init_with_configure(&itrack->props.cfg, pInfo->options); // set the defaults

	return Success;
}
#else
static InputInfoPtr preinit(InputDriverPtr drv, IDevPtr dev, int flags)
{
	struct MTouch *itrack;
	InputInfoPtr local = xf86AllocateInput(drv, 0);
	if (!local)
		goto error;
	itrack = calloc(1, sizeof(struct MTouch));
	if (!itrack)
		goto error;

	local->name = dev->identifier;
	local->type_name = XI_TOUCHPAD;
	local->device_control = device_control;
	local->read_input = read_input;
	local->switch_mode = switch_mode;
	local->private = itrack;
	local->flags = XI86_POINTER_CAPABLE | XI86_SEND_DRAG_EVENTS;
	local->conf_idev = dev;

	xf86CollectInputOptions(local, NULL, NULL);
	xf86OptionListReport(local->options);
	xf86ProcessCommonOptions(local, local->options);
	mconfig_init_with_configure(&itrack->props.cfg, local->options);
	itrack->vm = NULL;

	local->flags |= XI86_CONFIGURED;
 error:
	return local;
}
#endif

static void uninit(InputDriverPtr drv, InputInfoPtr local, int flags)
{
	struct itrack *itrack = local->private;

	free(local->private);
	local->private = 0;
	xf86DeleteInput(local, 0);
	(void)itrack;
}

/* About xorg drivers, modules:
 * http://www.x.org/wiki/Development/Documentation/XorgInputHOWTO/
 */
static InputDriverRec ITRACK = {
	.driverVersion = 1,
	.driverName    = "itrack",
	.Identify      = NULL,
	.PreInit       = preinit,
	.UnInit        = uninit,
	.module        = NULL,
	.default_options = NULL,
    .capabilities  = 0
};

static XF86ModuleVersionInfo moduleVersion = {
	"itrack",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	PACKAGE_VERSION_MAJOR, PACKAGE_VERSION_MINOR, PACKAGE_VERSION_PATCHLEVEL,
	ABI_CLASS_XINPUT,
	ABI_XINPUT_VERSION,
	MOD_CLASS_XINPUT,
	{0, 0, 0, 0}
};

static pointer plugin(pointer module, pointer options, int *errmaj, int *errmin)
{
	xf86AddInputDriver(&ITRACK, module, 0);
	return module;
}
static void unplug(pointer p)
{
    // void ;
}

_X_EXPORT XF86ModuleData itrackModuleData = {&moduleVersion, &plugin, &unplug };