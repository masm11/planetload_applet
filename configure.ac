#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# $Id:$

AC_PREREQ(2.57)

AC_INIT(planetload_applet, 20050718, masm@flowernet.gr.jp)
AC_CONFIG_SRCDIR(src/planetload_applet.c)
AM_INIT_AUTOMAKE([foreign dist-bzip2])

AM_MAINTAINER_MODE

AC_CONFIG_HEADER([config.h])

AC_PREFIX_PROGRAM(gnome-panel)

AC_PROG_INTLTOOL

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB(m, floor,,
[AC_MSG_ERROR([floor() is not in -lm.])])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h libintl.h locale.h netinet/in.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_CHECK_TYPES([int64_t, u_int64_t], ,
[AC_MSG_ERROR([This type is not available.])] ,
[#include <sys/types.h>])


# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([dup2 floor memset strchr strerror])

#
AC_ARG_ENABLE(debug, [  --enable-debug                Enable debug output],
[AC_DEFINE(DEBUG, 1, [Define to 1 if you want to debug.])])

# 
PKG_CHECK_MODULES(GSTUFF, libpanelapplet-2.0 libgtop-2.0 libgnomeui-2.0)
AC_PATH_PROG(GCONFTOOL, gconftool-2, no)
if test x"$GCONFTOOL" = xno; then
  AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
fi
AM_GCONF_SOURCE_2

#
dnl AM_WITH_NLS
ALL_LINGUAS="ja"

AM_GLIB_GNU_GETTEXT

GETTEXT_PACKAGE=planetload_applet
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Package name for GNU gettext])

gnomelocaledir='${prefix}/${DATADIRNAME}/locale'
AC_SUBST(gnomelocaledir)

#
AC_CONFIG_FILES([Makefile m4/Makefile src/Makefile po/Makefile.in
                 intltool-extract intltool-merge intltool-update])

AC_OUTPUT
