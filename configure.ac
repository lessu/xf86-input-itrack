#  Copyright 2007 Peter Hutterer
#  Copyright 2009 Przemysław Firszt
#
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
#  AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Process this file with autoconf to produce a configure script

AC_PREREQ([2.71])
AC_INIT([xf86-itrack],[0.0.1],[https://bugs.freedesktop.org/enter_bug.cgi?product=xorg],[xf86-itrack])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR(.)

# Initialize Automake
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_MAINTAINER_MODE

# Initialize libtool
AC_DISABLE_STATIC
LT_INIT

# Initialize X.Org macros 1.8 or later for MAN_SUBSTS set by XORG_MANPAGE_SECTIONS
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.8 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.8)
XORG_DEFAULT_OPTIONS


AH_TOP([#include "xorg-server.h"])
# configure option for module install directory
AC_ARG_WITH(xorg-module-dir, AS_HELP_STRING([--with-xorg-module-dir=DIR],[Default xorg module directory [[default=$libdir/xorg/modules]]]),
	[moduledir="$withval"],
	[moduledir="$libdir/xorg/modules"])

inputdir=${moduledir}/input
AC_SUBST(inputdir)

# Checks for pkg-config packages. We need to be able to override sdkdir
# to satisfy silly distcheck requirements.
PKG_CHECK_MODULES(XORG, xorg-server xproto $REQUIRED_MODULES)
XORG_CFLAGS="$CWARNFLAGS $XORG_CFLAGS"

AC_ARG_WITH([sdkdir], [],
    [sdkdir="$withval"],
    [sdkdir=`$PKG_CONFIG --variable=sdkdir xorg-server`])
AC_SUBST([sdkdir])

# Checks for libraries.

# Checks for header files.
m4_warn([obsolete],
[The preprocessor macro `STDC_HEADERS' is obsolete.
  Except in unusual embedded environments, you can safely include all
  ISO C90 headers unconditionally.])dnl
# Autoupdate added the next two lines to ensure that your configure
# script's behavior did not change.  They are probably safe to remove.
AC_CHECK_INCLUDES_DEFAULT
AC_PROG_EGREP


DRIVER_NAME=itrack
AC_SUBST([DRIVER_NAME])

#AC_CONFIG_FILES([Makefile
#           man/Makefile
#           xf86-itrack.pc])
AC_OUTPUT

# Everything else
AC_PROG_CC
AC_PROG_INSTALL
AC_CONFIG_FILES([Makefile])
AC_OUTPUT