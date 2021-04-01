#include "itrack-config.h"
#include <stdio.h> 
// #include <string.h>
#include <stdbool.h>
#include "common.h"


void itrack_config_init(struct itrack_config_s *config,DeviceIntPtr dev,XF86OptionPtr opt){
    xconfig_init();

//         "tap_key", "[tap1,tap2,tap3,tap4],Which mouse key will trigger will tap with finger1,2,3,4,5"

    struct config_prop_s *tap_prop = xconfig_add_prop("tap",
        "[dist,hold_time,release_time],"
        "dist:The Maximum movement will not be recognized as a tap,"
        "hold_time:The Maximum time will not be recognized as a tap," 
        "release_time:When recognized as a tap,how long a simulated mouse down will be released",
        ConfigTypeInt
    );
    xconfig_add_int_conf(tap_prop,"TapDistance" ,&config->tap_dist,50,(struct clamp_param_s) { CLAMP_MIN , 0 });
    xconfig_add_int_conf(tap_prop,"TapHold"     ,&config->tap_hold,200,(struct clamp_param_s) { CLAMP_MIN , 0 });
    xconfig_add_int_conf(tap_prop,"TapRelease"  ,&config->tap_release,20,(struct clamp_param_s) { CLAMP_MIN , 0 });

    xconfig_post_init(dev,opt);
}

void itrack_config_deinit(DeviceIntPtr dev){
    xconfig_deinit(dev);
}