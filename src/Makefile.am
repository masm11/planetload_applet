# $Id: Makefile.am 21 2005-07-18 07:57:00Z masm $

libexec_PROGRAMS = planetload_applet
# bin_PROGRAMS = test

planetload_applet_SOURCES =	\
    planetload_applet.c		\
    nsaiface.c			\
    nsaiface.h			\
    nsapainter.c		\
    nsapainter.h		\
    nsafinepainter.c		\
    nsafinepainter.h		\
    nsalightpainter.c		\
    nsalightpainter.h		\
    nsasplitpainter.c		\
    nsasplitpainter.h		\
    preferences.c		\
    pref_iface.c		\
    debug.c			\
    appconf.c			\
    appconf.h			\
    app.h			\
    color.c			\
    color.h			\
    about.c			\
    about.h			\
    ifstat_linux.c		\
    ifstat.h			\
    i18n-support.h

noinst_HEADERS =

# test_SOURCES =		\
#    test.c			\
#    ifstat_linux.c

gnomelocaledir = @gnomelocaledir@
AM_CPPFLAGS = $(GSTUFF_CFLAGS) -DGNOMELOCALEDIR=\"$(gnomelocaledir)\"
AM_CFLAGS = -Wall
planetload_applet_LDADD = $(GSTUFF_LIBS)

# test_LDADD = $(GSTUFF_LIBS)

#
