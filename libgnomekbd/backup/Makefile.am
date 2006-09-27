lib_LTLIBRARIES = libgnomekbd.la

libgnomekbd_la_CFLAGS = \
	-I$(top_srcdir) -Wall -Werror			\
	$(DBUS_CFLAGS)					\
	$(GTK_CFLAGS)					\
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

libgnomekbd_la_LDFLAGS = \
	$(GCONF_LIBS)

libgnomekbd_la_SOURCES = gkb-keyboard-config.c \
                         gkb-desktop-config.c \
                         gkb-indicator-config.c \
                         gkb-util.c \
                         gkb-indicator.c \
                         gkb-indicator-marshal.c \
                         gkb-indicator-plugin-manager.c \
                         gkb-config-registry.c \
                         gkb-keyboard-drawing-marshal.c \
                         gkb-keyboard-drawing.c

BUILT_SOURCES = gkb-indicator-marshal.c \
	gkb-indicator-marshal.h         \
        gkb-config-registry-server.h     \
        gkb-config-registry-client.h     \
        gkb-keyboard-drawing-marshal.c            \
        gkb-keyboard-drawing-marshal.h

CLEANFILES = \
	$(BUILT_SOURCES)

gnomekbdincdir = $(includedir)/libgnomekbd
gnomekbdinc_HEADERS = \
                      gkb-desktop-config.h \
                      gkb-keyboard-config.h \
                      gkb-config-registry-client.h \
                      gkb-config-registry.h \
                      gkb-indicator.h \
                      gkb-indicator-config.h \
                      gkb-indicator-plugin.h \
                      gkb-keyboard-drawing.h \
                      gkb-util.h

@INTLTOOL_DESKTOP_RULE@

noinst_HEADERS = gkb-indicator-plugin-manager.h \
                 $(extra_nih) \
                 gkb-config-private.h \
                 gkb-config-registry-server.h

EXTRA_DIST = gkb-indicator-marshal.list \
             gkb-keyboard-drawing-marshal.list \
             gkb-config-registry.xml \
             $(glade_DATA)

GLIB_GENMARSHAL = $(shell pkg-config --variable=glib_genmarshal glib-2.0)

gkb-indicator-marshal.h: gkb-indicator-marshal.list
	$(GLIB_GENMARSHAL) --prefix=gkb_indicator $(srcdir)/gkb-indicator-marshal.list --header > $@

gkb-indicator-marshal.c: gkb-indicator-marshal.h
	$(GLIB_GENMARSHAL) --prefix=gkb_indicator $(srcdir)/gkb-indicator-marshal.list --body > $@

gkb-config-registry-server.h:  gkb-config-registry.xml
	dbus-binding-tool --prefix=gkb_config_registry --mode=glib-server $< > $@

gkb-config-registry-client.h:  gkb-config-registry.xml
	dbus-binding-tool --prefix=gkb_config_registry --mode=glib-client $< > $@

gkb-keyboard-drawing-marshal.h: gkb-keyboard-drawing-marshal.list
	( $(GLIB_GENMARSHAL) --prefix=gkb_keyboard_drawing $(srcdir)/gkb-keyboard-drawing-marshal.list \
                            --header > gkb-keyboard-drawing-marshal.tmp \
          && mv gkb-keyboard-drawing-marshal.tmp gkb-keyboard-drawing-marshal.h ) \
        || ( rm -f gkb-keyboard-drawing-marshal.tmp && exit 1 )
 
gkb-keyboard-drawing-marshal.c: gkb-keyboard-drawing-marshal.h
	( $(GLIB_GENMARSHAL) --prefix=gkb_keyboard_drawing $(srcdir)/gkb-keyboard-drawing-marshal.list \
                            --body > gkb-keyboard-drawing-marshal.tmp \
          && mv gkb-keyboard-drawing-marshal.tmp gkb-keyboard-drawing-marshal.c ) \
        || ( rm -f gkb-keyboard-drawing-marshal.tmp && exit 1 )

