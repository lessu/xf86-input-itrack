#include "xconfig.h"
#include "common.h"
#include <stdio.h> 
#include <stdint.h>

#define LOG_CONFIG_DEBUG LOG_DEBUG
#define LOG_CONFIG_ERROR LOG_DEBUG
#define LOG_CONFIG_WARNING LOG_DEBUG

struct config_item_s{
    char conf_name[256];
    void *val_ptr;
    size_t val_size;
    ConfigSetFn setfn;
    void *setfn_userdata;
    BOOL userdata_needs_free;
    const struct config_prop_s *prop;
    struct config_item_s *next;
};

struct config_def_s{
    struct config_prop_s *props_head;
    long prop_change_handler;
};

typedef struct config_def_s     ConfigDef;

/** api predefine */
static ConfigDef *s_default_config( void );

static ConfigDef *s_default_config( void ){
    static ConfigDef default_config;
    return &default_config;
}

static void s_prop_post_init(struct config_prop_s *prop,DeviceIntPtr dev,XF86OptionPtr opt_ptr){
    Atom atom = 0;

    switch (prop->type){
    case ConfigTypeInt:{
        int32_t *uvals = malloc(sizeof(int32_t) * prop->items_count);
        uint32_t idx = 0;
        struct config_item_s *item = prop->items_head;
        // LOG_DEBUG("item config_name=%s,prop name=%s\n",item->conf_name,item->prop->prop_desc);
        while( item != NULL ){

            int new_value = xf86SetIntOption(opt_ptr, item->conf_name, *(int *)item->val_ptr);
            int ret = item->setfn(item,&new_value,item->setfn_userdata);
            if(ret != 0 ){
                LOG_CONFIG_ERROR("set value failed\n");
            }
            uvals[idx] = *(int32_t *)item->val_ptr;

            item = item -> next;
            idx++;

        }
        atom = MakeAtom(prop->prop_desc, strlen(prop->prop_desc), TRUE);
        XIChangeDeviceProperty(dev, atom, XA_INTEGER, /** now only support 32bit */32, PropModeReplace, prop->items_count, uvals, FALSE);
        XISetDevicePropertyDeletable(dev, atom, FALSE);
        free(uvals);
        uvals = NULL;

        break;
    }
    case ConfigTypeUnknown:
        break;
    }
    //  XIChangeDeviceProperty(dev, name, XA_STRING, 8, PropModeReplace, len,pval, True);

    // save this atom
    prop->atom = atom;
}
void xconfig_init(void ){
    ConfigDef *cfg = s_default_config();
    cfg->props_head = NULL;
    cfg->prop_change_handler = 0;
}

void xconfig_deinit( DeviceIntPtr dev ){
    ConfigDef *cfg = s_default_config();
    struct config_prop_s *prop = cfg->props_head;
    while( prop != NULL ){
        
        // todo://howto free atom that from MakeAtom;
        LOG_DEBUG("free prop %p\n",prop);
        struct config_item_s *item = prop->items_head;
        while (item != NULL){
            LOG_DEBUG("free item %p\n",item);
            if( item->userdata_needs_free && item->setfn_userdata != NULL ){
                free( item-> setfn_userdata );
                item-> setfn_userdata = NULL;
            }
            struct config_item_s *t_item = item;
            item = item -> next;
            free(t_item);
        }
        
        struct config_prop_s *t = prop;
        prop = prop -> next;
        free(t);
    }
    if(cfg -> prop_change_handler != 0 ){
        // XIUnregisterPropertyHandler(dev,cfg -> prop_change_handler);
    }
    cfg->props_head = NULL;
    cfg->prop_change_handler = 0;
}

int xconfig_post_init(DeviceIntPtr dev,XF86OptionPtr opt_ptr){
    ConfigDef *cfg = s_default_config();
    struct config_prop_s *props = cfg->props_head;
    while( props != NULL ){
        s_prop_post_init(props,dev,opt_ptr);
        props = props->next;
    }
    // cfg -> prop_change_handler = XIRegisterPropertyHandler(dev, xconfig_prop_change, NULL, NULL);
    return 0;
}

int xconfig_prop_change(DeviceIntPtr dev, Atom property, XIPropertyValuePtr val, BOOL checkonly){
    ConfigDef *cfg = s_default_config();
    struct config_prop_s *prop = cfg->props_head;
    while( prop != NULL ){
        if( prop -> atom == property ){
            break;
        }
        prop = prop->next;
    }
    if(prop == NULL){
        LOG_CONFIG_WARNING("config_prop_change,property %ld not found\n",property);
        return BadAtom;
    }
    switch( prop->type ){
    case ConfigTypeInt:{
        if (val->size != prop->items_count || val->format != 32 || val->type != XA_INTEGER)
            return BadMatch;

        int32_t *vals = (int32_t *) val->data;
        struct config_item_s *item = prop->items_head;
        int count = 0;
        while( item != NULL && count < val->size){
            int ret = item->setfn(item,&vals[count],item->setfn_userdata);
            if( ret != 0 ){
                return BadMatch;
            }
            count++;
            item = item->next;
        }
        break;
    }
    case ConfigTypeUnknown:
        return BadAtom;
    }
    return Success;
}

/**
 * 
 * we reference `prop_name`,make sure its not freed
 * 
 */ 
struct config_prop_s* xconfig_add_prop( const char *prop_name,const char *prop_desc,enum config_type_e type ){
    ConfigDef *cfg = s_default_config();
    struct config_prop_s *props = malloc( sizeof(struct config_prop_s));
    props->next = NULL;

    /** append to item list */
    if(cfg->props_head == NULL){
        cfg->props_head = props;
    }else{
        struct config_prop_s *it = cfg->props_head;
        while ( it -> next != NULL){
            it = it -> next;
        }
        it -> next = props;
    }

    props->prop_desc = prop_name;
    props->type = type;
    props->items_count = 0;
    props->atom = 0;
    props->items_head = NULL;
    
    return props;
}


BOOL xconfig_add_conf(
    struct config_prop_s *prop,
    void *val_ptr,
    size_t val_size,
    const char *conf_name,
    // void *default_val,
    ConfigSetFn setfn,
    void *setfn_userdata,
    BOOL userdata_needs_free
){

    struct config_item_s *item = malloc( sizeof(struct config_item_s));
    item->next = NULL;

    /** append to item list */
    if(prop->items_head == NULL){
        prop->items_head = item;
    }else{
        struct config_item_s *it = prop->items_head;
        while ( it -> next != NULL){
            it = it -> next;
        }
        it -> next = item;
    }

    LOG_DEBUG("set item[%p] prop=%p\n",item,prop);
    item->prop = prop;
    prop->items_count ++;

    item->val_ptr   = val_ptr;
    item->val_size  = val_size;
    item->setfn     = setfn;
    item->setfn_userdata        = setfn_userdata;
    item->userdata_needs_free   = userdata_needs_free;
    strncpy( item->conf_name, conf_name, sizeof(item->conf_name) - 1 );

    if(item->setfn == NULL){
        item->setfn = xconfig_setfn_default;
    }
    return TRUE;

}

int xconfig_setfn_default(const struct config_item_s *item,void *val,void *userdata){
    memcpy(item->val_ptr,val,item->val_size);
    return 0;
}

int xconfig_setfn_int_clamp(const struct config_item_s *item,void *val,void *userdata){
    struct clamp_param_s *param = userdata;
    LOG_DEBUG("item=%p\n",item);
    LOG_DEBUG("item -> prop=%p\n",item -> prop);
    if(item->prop && item -> prop -> type == ConfigTypeInt){
        int new_value = *(int *)val;
        switch (param->mode){
        case CLAMP_OFF:
            /* code */
            break;
        case CLAMP_MAX:
            if(new_value > param->max) new_value = param->max;
            break;
        case CLAMP_MIN:
            if(new_value < param->min) new_value = param->min;
            break;
        case CLAMP_ON:
            if(new_value > param->max) new_value = param->max;
            if(new_value < param->min) new_value = param->min;
        }
        memcpy(item->val_ptr,val,item->val_size);
        return 0;
    }else{
        // doing nothing
        return 1;
    }
}

void xconfig_add_int_conf(struct config_prop_s *prop,const char *conf_name,int *val_ptr,int default_val,const struct clamp_param_s clamp){
    struct clamp_param_s *pclamp = NULL;
    if(clamp.mode != None){
        pclamp = malloc( sizeof (struct clamp_param_s) );
        memcpy(pclamp,&clamp,sizeof (struct clamp_param_s));
    }
    *val_ptr = default_val;
    xconfig_add_conf(prop,val_ptr,4,conf_name,xconfig_setfn_int_clamp,pclamp,TRUE);
}

void xconfig_add_single_int_prop(
    const char *prop_name,
    const char *prop_desc,
    const char *conf_name,
    int *val_ptr,
    int default_val,
    const struct clamp_param_s clamp
){
    struct clamp_param_s *pclamp = NULL;
    if(clamp.mode != None){
        pclamp = malloc( sizeof (struct clamp_param_s) );
        memcpy(pclamp,&clamp,sizeof (struct clamp_param_s));
    }
    *val_ptr = default_val;
    struct config_prop_s *prop = xconfig_add_prop(prop_name,prop_desc,ConfigTypeInt);
    xconfig_add_conf(prop,val_ptr,4,conf_name,xconfig_setfn_int_clamp,pclamp,TRUE);
}