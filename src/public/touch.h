#ifndef PUBLIC_TOUCH_H__
#define PUBLIC_TOUCH_H__

#define MT_NEW 0
#define MT_RELEASED 1
#define MT_INVALID 2
#define MT_THUMB 3
#define MT_PALM 4
#define MT_EDGE 5
#define MT_TAP 6
#define MT_BUTTON 7

struct Touch {
	unsigned int flags;
	// double direction;
	int tracking_id;
	int x, y, dx, dy;
	int total_dx, total_dy;
	int distance;
	struct timeval create_time;
	struct timeval update_time;
	struct timeval last_update_time;
	int touch_major,touch_minor;
	int ap_major,ap_minor;
};
#endif /** PUBLIC_TOUCH_H__ */
