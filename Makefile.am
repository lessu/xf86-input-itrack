#  Copyright 2007 Peter Hutterer
#  Copyright 2009 Przemysław Firszt
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  on the rights to use, copy, modify, merge, publish, distribute, sub
#  license, and/or sell copies of the Software, and to permit persons to whom
#  the Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice (including the next
#  paragraph) shall be included in all copies or substantial portions of the
#  Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
#  ADAM JACKSON BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

AUTOMAKE_OPTIONS = foreign

# Ensure headers are installed below $(prefix) for distcheck
DISTCHECK_CONFIGURE_FLAGS = --with-sdkdir='$${includedir}/xorg'

SUBDIRS = man

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xf86-itrack.pc

EXTRA_DIST = ChangeLog

MAINTAINERCLEANFILES=ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog






# this is obnoxious:
# -module lets us name the module exactly how we want
# -avoid-version prevents gratuitous .0.0.0 version numbers on the end
# _ladir passes a dummy rpath to libtool so the thing will actually link
# TODO: -nostdlib/-Bstatic/-lgcc platform magic, not installing the .a, etc.

AM_CFLAGS = $(XORG_CFLAGS) 

CFLAGS = -g -std=gnu99 -Wno-declaration-after-statement -I$(top_srcdir)/include/ -I$(top_srcdir)/src -I$(top_srcdir)/src/guesture

noinst_LTLIBRARIES = libguesture.la
libguesture_la_CFLAGS=$(AM_CFLAGS) -D__DEBUG__
libguesture_ladir   = @inputdir@
libguesture_la_SOURCES  = src/guesture/guesture.c \
                        src/guesture/move.c \
                        src/guesture/tap.c \
                        src/guesture/scroll.c \
                        src/guesture/drag.c \
                        src/guesture/pinch.c

@DRIVER_NAME@_drv_la_LTLIBRARIES = @DRIVER_NAME@_drv.la
@DRIVER_NAME@_drv_la_LDFLAGS = -module -avoid-version
@DRIVER_NAME@_drv_ladir = @inputdir@

#AM_CPPFLAGS=

@DRIVER_NAME@_drv_la_LIBADD    = libguesture.la
@DRIVER_NAME@_drv_la_SOURCES = src/@DRIVER_NAME@.c \
                               src/@DRIVER_NAME@.h \
                               src/button-axle.c \
                               src/capabilities.c \
                               src/itrack-main.c \
                               src/mconfig.c \
                               src/mprops.c \
                               src/mtstate.c \
                               src/hwstate.c \
                               src/common.c \
                               src/guesture/guesture_manager.c \
                               src/debug.c \
                               src/touch-handler.c \
                               src/post-stage.c \
                               src/xconfig.c \
                               src/itrack-config.c

debug_main_PROGRAMS = debug_main
debug_main_CFLAGS=$(AM_CFLAGS) -D__OFFLINE__
debug_maindir       = @inputdir@
debug_main_LDADD    = libguesture.la
debug_main_SOURCES  = test/main.c \
                      test/simu_test.c \
                      src/common.c \
                      src/guesture/guesture_manager.c \
                      test/test_move.c \
                      test/test_tap.c