#include "common.h"

static int s_last_log_number = 0;

int get_next_log_number( void )
{
	return ++s_last_log_number;
}

#ifdef __DEBUG__
void xf86Msg(int type ,const char *format , ...){
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);
}
#endif