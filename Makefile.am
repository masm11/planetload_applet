SUBDIRS = src po

EXTRA_DIST = PlanetloadApplet.server.in.in PlanetloadApplet.xml planetload_applet.schemas.in \
             intltool-extract.in intltool-merge.in intltool-update.in \
             00todo.txt

if HAVE_GNOME
all-local: $(server_files)
endif

if HAVE_XFCE4
all-local: $(desktop_files)
endif

serverdir           = $(libdir)/bonobo/servers
server_in_in_files  = PlanetloadApplet.server.in.in
server_in_files     = $(server_in_in_files:.server.in.in=.server.in)
server_files        = $(server_in_files:.server.in=.server)

schemadir           = $(GCONF_SCHEMA_FILE_DIR)

uidir               = $(datadir)/gnome-2.0/ui

if HAVE_GNOME
server_DATA         = $(server_files)
schema_DATA         = planetload_applet.schemas
ui_DATA             = PlanetloadApplet.xml
endif

desktopdir          = $(datadir)/xfce4/panel-plugins
desktop_in_in_files = planetload_applet.desktop.in.in
desktop_in_files    = $(desktop_in_in_files:.desktop.in.in=.desktop.in)
desktop_files       = $(desktop_in_files:.desktop.in=.desktop)

if HAVE_XFCE4
desktop_DATA        = $(desktop_files)
endif

$(server_in_files): $(server_in_in_files) Makefile
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

if HAVE_GNOME
install-data-local: $(schema_DATA)
if GCONF_SCHEMAS_INSTALL
	for p in $(schema_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p; \
	done
endif
endif

@INTLTOOL_SERVER_RULE@
@INTLTOOL_SCHEMAS_RULE@
@INTLTOOL_DESKTOP_RULE@

$(desktop_in_files): $(desktop_in_in_files) Makefile
	sed -e "s^@PLUGIN_PATH@^$(libexecdir)/xfce4/panel-plugins^" $< > $@.new
	mv $@.new $@

CLEANFILES = $(server_DATA) $(server_in_files) $(schema_DATA) $(desktop_DATA)

distcheck-hook:
	$(srcdir)/check.sh $(distdir) $(top_srcdir)

DISTCHECK_CONFIGURE_FLAGS = --disable-schemas-install

#
