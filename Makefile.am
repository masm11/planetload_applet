# $Id: Makefile.am 24 2005-07-18 08:12:27Z masm $

SUBDIRS = src po

EXTRA_DIST = PlanetloadApplet.server.in.in PlanetloadApplet.xml planetload_applet.schemas.in \
             intltool-extract.in intltool-merge.in intltool-update.in \
             00todo.txt

all-local: $(server_DATA)

serverdir       = $(libdir)/bonobo/servers
server_in_files = PlanetloadApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

schemadir 	= $(GCONF_SCHEMA_FILE_DIR)
schema_DATA	= planetload_applet.schemas

uidir		= $(datadir)/gnome-2.0/ui
ui_DATA		= PlanetloadApplet.xml

$(server_in_files): $(server_in_files:.server.in=.server.in.in) Makefile
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

install-data-local: $(schema_DATA)
if GCONF_SCHEMAS_INSTALL
	for p in $(schema_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p; \
	done
endif

CLEANFILES = $(server_DATA) $(server_in_files) $(schema_DATA)

@INTLTOOL_SERVER_RULE@
@INTLTOOL_SCHEMAS_RULE@

distcheck-hook:
	$(srcdir)/check.sh $(distdir) $(top_srcdir)

ACLOCAL_AMFLAGS = -I /usr/local/share/aclocal
DISTCHECK_CONFIGURE_FLAGS = --disable-schemas-install


#
