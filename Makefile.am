SUBDIRS = src po

EXTRA_DIST = PlanetloadApplet.server.in.in PlanetloadApplet.xml planetload_applet.schemas.in \
             intltool-extract.in intltool-merge.in intltool-update.in \
             00todo.txt

CLEANFILES = $(server_DATA) $(server_in_files) $(schema_DATA) $(desktop_DATA)

distcheck-hook:
	$(srcdir)/check.sh $(distdir) $(top_srcdir)

DISTCHECK_CONFIGURE_FLAGS = --with-xfce4

####
#### for GNOME.
####

if HAVE_GNOME

all-local: $(server_files)

serverdir           = $(libdir)/bonobo/servers
server_in_in_files  = PlanetloadApplet.server.in.in
server_in_files     = $(server_in_in_files:.server.in.in=.server.in)
server_files        = $(server_in_files:.server.in=.server)
server_DATA         = $(server_files)

schemadir           = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA         = planetload_applet.schemas

uidir               = $(datadir)/gnome-2.0/ui
ui_DATA             = PlanetloadApplet.xml

$(server_in_files): $(server_in_in_files) Makefile
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

install-data-local: $(schema_DATA)
if GCONF_SCHEMAS_INSTALL
	for p in $(schema_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p; \
	done
endif

@INTLTOOL_SERVER_RULE@
@INTLTOOL_SCHEMAS_RULE@

endif

####
#### for Xfce4.
####

if HAVE_XFCE4

all-local: $(desktop_files)

desktopdir          = $(datadir)/xfce4/panel-plugins
desktop_in_in_files = planetload_applet.desktop.in.in
desktop_in_files    = $(desktop_in_in_files:.desktop.in.in=.desktop.in)
desktop_files       = $(desktop_in_files:.desktop.in=.desktop)
desktop_DATA        = $(desktop_files)

@INTLTOOL_DESKTOP_RULE@

$(desktop_in_files): $(desktop_in_in_files) Makefile
	sed -e "s^@PLUGIN_PATH@^$(libexecdir)/xfce4/panel-plugins^" $< > $@.new
	mv $@.new $@

endif
