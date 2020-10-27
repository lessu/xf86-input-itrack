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
 ******************************de********************************************/

#pragma once
#include <X11/Xdefs.h>
#include <X11/Xatom.h>
#include <xf86.h>
#include <xf86Xinput.h>
#include <exevents.h>

#include <stdio.h>
#include <stdbool.h>

/** predefine */
struct clamp_param_s{
    const enum{
        CLAMP_OFF,
        CLAMP_MIN,
        CLAMP_MAX,
        CLAMP_ON
    } mode;
    int min,max;
};

enum config_type_e{
    ConfigTypeUnknown = 0,
    ConfigTypeInt
};

struct config_prop_s{
    const char *prop_desc;
    enum config_type_e type;
    size_t items_count;
    Atom atom;
    struct config_item_s *items_head;   
    struct config_prop_s *next;
};


typedef int (*ConfigSetFn)(const struct config_item_s *item,void *val,void *userdata);

int xconfig_setfn_default(const struct config_item_s *item,void *val,void *userdata);
int xconfig_setfn_int_clamp(const struct config_item_s *item,void *val,void *userdata);

void xconfig_init( void );
void xconfig_deinit( DeviceIntPtr dev );

int xconfig_post_init(DeviceIntPtr dev,XF86OptionPtr opt_ptr);
int xconfig_prop_change(DeviceIntPtr dev, Atom property, XIPropertyValuePtr val, BOOL checkonly);

struct config_prop_s* xconfig_add_prop(
    const char *prop_name,
    const char *prop_desc,
    enum config_type_e type
);

bool xconfig_add_conf(
    struct config_prop_s *prop,
    void *val_ptr,
    size_t val_size,
    const char *conf_name,
    ConfigSetFn setfn,
    void *setfn_userdata,
    bool userdata_needs_free
);


/** utilities */
void xconfig_add_int_conf(
    struct config_prop_s *prop,
    const char *conf_name,
    int *val_ptr,
    int default_val,
    const struct clamp_param_s clamp
);

void xconfig_add_single_int_prop(
    const char *prop_name,
    const char *prop_desc,
    const char *conf_name,
    int *val_ptr,
    int default_val,
    const struct clamp_param_s clamp
);