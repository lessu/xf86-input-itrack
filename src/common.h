/***************************************************************************
 *
 * Multitouch X driver
 * Copyright (C) 2008 Henrik Rydberg <rydberg@euromail.se>
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

#ifndef COMMON_H
#define COMMON_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xorg-server.h>
#include <xf86.h>
#include <xf86_OSproc.h>
#include <xf86Xinput.h>
#include <errno.h>
#include <mtdev-mapping.h>
#include <stdint.h>
#include <sys/time.h>

#define DIM_FINGER 16
#define DIM_TOUCHES 16
#define ONE_SECOND_MS (1000)
/* year-proof millisecond event time */
typedef __u64 mstime_t;

/* all bit masks have this type */
typedef unsigned int bitmask_t;

/**
 * m - bit set (integer)
 * x - modified bit
 * b - new value
 * @{
 */
#define BITMASK(x) (1U << (x))
#define BITONES(x) (BITMASK(x) - 1U)
#define GETBIT(m, x) (((m) >> (x)) & 1U)
#define SETBIT(m, x) (m |= BITMASK(x))
#define CLEARBIT(m, x) (m &= ~BITMASK(x))
#define MODBIT(m, x, b) ((b) ? SETBIT(m, x) : CLEARBIT(m, x))
/** }@ */

#define ABSVAL(x) ((x) < 0 ? -1*(x) : (x))
#define MINVAL(x, y) ((x) < (y) ? (x) : (y))
#define MAXVAL(x, y) ((x) > (y) ? (x) : (y))
#define MODVAL(x, y) ((x) - ((int)((x) / (y))) * (y))
#define SQRVAL(x) ((x) * (x))
#define CLAMPVAL(x, min, max) MAXVAL(MINVAL(x, max), min)
#define SGNVAL (x) ((x) < 0 ? -1 : (((x) > 0) ? 1 : 0))
// #ifdef __DEBUG__
// void xf86Msg(int type ,const char *format , ...);
// #endif
#define LOG_ERROR(...) \
	do{ \
		xf86Msg(X_ERROR, "itrack[%i] %s:%i: ", get_next_log_number(), __FILE__, __LINE__); \
		xf86Msg(X_ERROR, __VA_ARGS__); \
	}while(0)

#define LOG_WARNING(...) \
	do{ \
		xf86Msg(X_WARNING, "itrack[%i] %s:%i: ", get_next_log_number(), __FILE__, __LINE__); \
		xf86Msg(X_WARNING, __VA_ARGS__); \
	}while(0)

#define LOG_INFO(...) \
	do{ \
		xf86Msg(X_INFO, "itrack[%i] %s:%i: ", get_next_log_number(), __FILE__, __LINE__); \
		xf86Msg(X_INFO, __VA_ARGS__); \
	}while(0)
#define LOG_INFO_CONT(...) xf86Msg(X_INFO, "itrack[...]: " __VA_ARGS__)

#define LOG_DISABLED(...) do { } while(0)

#define LOG_INFO_ENABLED(...) LOG_INFO(__VA_ARGS__)
#define LOG_INFO_DISABLED(...)

#define LOG_INFO2(ENABLED_or_DISABLED, ...) LOG_INFO_##ENABLED_or_DISABLED(__VA_ARGS__)

#if 1
# define LOG_DEBUG LOG_INFO
#else
# define LOG_DEBUG LOG_DISABLED
#endif

int get_next_log_number( void );

/**
 * 
 * TIME COMMON OPERATION
 * 
 */ 
/* Retrieve the current time and place it in tv.
 */
inline void microtime(struct timeval* tv)
{
	gettimeofday(tv, NULL);
}

/* Copy one time value to another.
 */
static inline void timercp(struct timeval* dest, const struct timeval* src)
{
	memcpy(dest, src, sizeof(struct timeval));
}

/* Convert a timeval to milliseconds since the epoch. Truncates additional
 * timer resolution effectively rounding down.
 */
static inline mstime_t timertoms(const struct timeval* tv)
{
	return (mstime_t)(tv->tv_sec*1000) + (mstime_t)(tv->tv_usec/1000);
}

/* Convert a value in milliseconds to a timeval and place the value in tv.
 */
static inline void timerfromms(struct timeval* tv, const mstime_t ms)
{
	tv->tv_sec = (time_t)(ms/1000);
	tv->tv_usec = (suseconds_t)((ms%1000)*1000);
}

/* Convert a timeval to microseconds.
 */
static inline suseconds_t timertomicro(const struct timeval* tv)
{
	return tv->tv_sec * 1000000 + tv->tv_usec;
}

/* Add milliseconds to a timeval and place the resulting value in dest.
 */
static inline void timeraddms(const struct timeval* a, const mstime_t b, struct timeval* dest)
{
	struct timeval tv;
	timerfromms(&tv, b);
	timeradd(a, &tv, dest);
}

/* Check if given timeval a is set to epoch time.
 */
static inline int isepochtime(const struct timeval* a)
{
	struct timeval epoch;
	timerclear(&epoch);

	return timercmp(a, &epoch, ==);
}

static inline int64_t time_diff_ms(const struct timeval *nowtime,const struct timeval *basetime){
	return (int64_t)timertoms(nowtime) - (int64_t)timertoms(basetime);
}
/** 
 * 
 * MATH UTILS
 * 
 */ 
/* Clamp value to 15 bits.
 */
static inline int math_clamp15(int x)
{
	return x < -32767 ? -32767 : x > 32767 ? 32767 : x;
}

/* Absolute scale is assumed to fit in 15 bits.
 */
static inline uint64_t math_dist2(int dx, int dy)
{
	dx = math_clamp15(dx);
	dy = math_clamp15(dy);
	return dx * dx + dy * dy;
}

#define __nullable
#define __nonull
#define __output

/* Return index of first bit [0-31], -1 on zero\
 */
#define firstbit(v) (__builtin_ffs(v) - 1)

/* Boost-style foreach bit.
 */
#define foreach_bit(i, m)						\
	for (i = firstbit(m); i >= 0; i = firstbit((m) & (~0U << (i + 1))))
#endif
