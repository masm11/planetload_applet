if HAVE_XFCE4
plugindir = $(libexecdir)/xfce4/panel-plugins
plugin_PROGRAMS = planetload_applet
endif

if HAVE_GNOME
libexec_PROGRAMS = planetload_applet
endif

# bin_PROGRAMS = test

planetload_applet_SOURCES =	\
    planetload_applet.c		\
    planetload_applet.h		\
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
    preferences.h		\
    pref_iface.c		\
    pref_iface.h		\
    debug.c			\
    debug.h			\
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

AM_CPPFLAGS = $(GSTUFF_CFLAGS) -DLOCALEDIR=\"$(localedir)\"
AM_CFLAGS = -Wall
planetload_applet_LDADD = $(GSTUFF_LIBS)

# test_LDADD = $(GSTUFF_LIBS)

#
