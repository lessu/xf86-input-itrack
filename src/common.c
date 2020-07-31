#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <xf86.h>


static int s_last_log_number = 0;

int get_next_log_number( void )
{
	return ++s_last_log_number;
}

#ifdef __OFFLINE__
void xf86Msg(int type ,const char *format , ...){
    va_list argptr;
    va_start(argptr, format);
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wsuggest-attribute=format"
    
    vfprintf(stdout, format, argptr);

    // #pragma GCC diagnostic pop

    va_end(argptr);
}
#else

// void itrak_msg(int type ,const char *format , ...){
//     va_list argptr;
//     va_start(argptr, format);
//     switch(type){
//     case IT_LOG_ERROR_TYPE:
//         xf86Msg(X_ERROR,format,argptr);
//     break;
//     case  IT_LOG_WARNING_TYPE:
//         xf86Msg(X_WARNING,format,argptr);
//     break;
//     case IT_LOG_INFO_TYPE:
//         xf86Msg(X_INFO,format,argptr);
//     break;
//     case IT_LOG_DEBUG_TYPE:
//         xf86Msg(X_DEBUG,format,argptr);
//     break;
//     default:
//     break;
//     }
//     va_end(argptr);
// }
#endif