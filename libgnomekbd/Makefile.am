lib_LTLIBRARIES = libgnomekbd.la libgnomekbdui.la

common_CFLAGS = \
	-I$(top_srcdir) -Wall -Werror			\
	$(GDK_CFLAGS)					\
	$(DBUS_CFLAGS)					\
	$(GCONF_CFLAGS)					\
	$(LIBGNOME_CFLAGS)				\
	$(LIBXKLAVIER_CFLAGS)				\
	-I$(top_srcdir)/intl				\
	-DSYS_PLUGIN_DIR=\"$(libdir)/gnomekbd/\"	\
	-DG_LOG_DOMAIN=\"GnomeKbdIndicator\"		\
	-DGLADEDIR=\"$(gladedir)\"			\
	-DDATADIR=\"$(datadir)\"			\
	-DSYSCONFDIR=\"$(sysconfdir)\"			\
	-DLIBDIR=\"$(libdir)\"				

libgnomekbd_la_CFLAGS = $(common_CFLAGS)

libgnomekbdui_la_CFLAGS = $(common_CFLAGS)		\
	$(GTK_CFLAGS)

common_LDFLAGS = -version-info @VERSION_INFO@ \
		$(DBUS_LIBS) \
		$(GDK_LIBS) \
		$(GCONF_LIBS) \
		$(LIBXKLAVIER_LIBS)

libgnomekbd_la_LDFLAGS = $(common_LDFLAGS)

libgnomekbdui_la_LDFLAGS = $(common_LDFLAGS) \
                           $(GTK_LIBS) \
                           $(LIBGNOME_LIBS) \
                           -lgnomekbd

libgnomekbd_la_SOURCES = \
                         gkbd-config-registry.c \
                         gkbd-desktop-config.c \
                         gkbd-keyboard-config.c \
                         gkbd-util.c

libgnomekbdui_la_SOURCES = \
                         gkbd-indicator-config.c \
                         gkbd-indicator.c \
                         gkbd-indicator-marshal.c \
                         gkbd-indicator-plugin-manager.c \
                         gkbd-keyboard-drawing-marshal.c \
                         gkbd-keyboard-drawing.c

BUILT_SOURCES = gkbd-indicator-marshal.c \
	gkbd-indicator-marshal.h         \
        gkbd-config-registry-server.h     \
        gkbd-config-registry-client.h     \
        gkbd-keyboard-drawing-marshal.c            \
        gkbd-keyboard-drawing-marshal.h

CLEANFILES = $(BUILT_SOURCES) $(schema_DATA)

gnomekbdincdir = $(includedir)/libgnomekbd
gnomekbdinc_HEADERS = \
                      gkbd-desktop-config.h \
                      gkbd-keyboard-config.h \
                      gkbd-config-registry-client.h \
                      gkbd-config-registry.h \
                      gkbd-indicator.h \
                      gkbd-indicator-config.h \
                      gkbd-indicator-plugin.h \
                      gkbd-keyboard-drawing.h \
                      gkbd-util.h

@INTLTOOL_DESKTOP_RULE@

@INTLTOOL_SCHEMAS_RULE@

noinst_HEADERS = gkbd-indicator-plugin-manager.h \
                 $(extra_nih) \
                 gkbd-config-private.h \
                 gkbd-config-registry-server.h

EXTRA_DIST = gkbd-indicator-marshal.list \
             gkbd-keyboard-drawing-marshal.list \
             gkbd-config-registry.xml \
             $(glade_DATA) \
             $(schema_in_files)

GLIB_GENMARSHAL = $(shell pkg-config --variable=glib_genmarshal glib-2.0)

gkbd-indicator-marshal.h: gkbd-indicator-marshal.list
	$(GLIB_GENMARSHAL) --prefix=gkbd_indicator $(srcdir)/gkbd-indicator-marshal.list --header > $@

gkbd-indicator-marshal.c: gkbd-indicator-marshal.h
	$(GLIB_GENMARSHAL) --prefix=gkbd_indicator $(srcdir)/gkbd-indicator-marshal.list --body > $@

gkbd-config-registry-server.h:  gkbd-config-registry.xml
	dbus-binding-tool --prefix=gkbd_config_registry --mode=glib-server $< > $@

gkbd-config-registry-client.h:  gkbd-config-registry.xml
	dbus-binding-tool --prefix=gkbd_config_registry --mode=glib-client $< > $@

gkbd-keyboard-drawing-marshal.h: gkbd-keyboard-drawing-marshal.list
	( $(GLIB_GENMARSHAL) --prefix=gkbd_keyboard_drawing $(srcdir)/gkbd-keyboard-drawing-marshal.list \
                            --header > gkbd-keyboard-drawing-marshal.tmp \
          && mv gkbd-keyboard-drawing-marshal.tmp gkbd-keyboard-drawing-marshal.h ) \
        || ( rm -f gkbd-keyboard-drawing-marshal.tmp && exit 1 )
 
gkbd-keyboard-drawing-marshal.c: gkbd-keyboard-drawing-marshal.h
	( $(GLIB_GENMARSHAL) --prefix=gkbd_keyboard_drawing $(srcdir)/gkbd-keyboard-drawing-marshal.list \
                            --body > gkbd-keyboard-drawing-marshal.tmp \
          && mv gkbd-keyboard-drawing-marshal.tmp gkbd-keyboard-drawing-marshal.c ) \
        || ( rm -f gkbd-keyboard-drawing-marshal.tmp && exit 1 )

schemadir   = $(GCONF_SCHEMA_FILE_DIR)
schema_in_files= desktop_gnome_peripherals_keyboard_xkb.schemas.in

schema_DATA = $(schema_in_files:.schemas.in=.schemas)

if GCONF_SCHEMAS_INSTALL
# don't do this if we are building in eg. rpm
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schema_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p; \
		done \
	fi
else
install-data-local:
endif
