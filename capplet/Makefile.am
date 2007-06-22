bin_PROGRAMS = gkbd-indicator-plugins-capplet

gladedir="$(pkgdatadir)/glade"
glade_DATA = gkbd-indicator-plugins.glade

@INTLTOOL_DESKTOP_RULE@

desktopdir = $(datadir)/applications
Desktop_in_files = gkbd-indicator-plugins-capplet.desktop.in
desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

gkbd_indicator_plugins_capplet_CFLAGS = \
	-I$(top_srcdir)	-Wall -Werror				\
	$(LIBGLADE_CFLAGS)				\
	$(LIBGNOME_CFLAGS)				\
	$(LIBGNOMEUI_CFLAGS)				\
	$(LIBXKLAVIER_CFLAGS)				\
	-I$(top_srcdir)/intl				\
	-DSYS_PLUGIN_DIR=\"$(libdir)/gnomekbd/\"	\
	-DG_LOG_DOMAIN=\"GnomeKbdIndicatorPluginsCapplet\"	\
        -DPREFIX=\"$(prefix)\"				\
        -DGLADEDIR=\"$(gladedir)\"                      \
        -DDATADIR=\"$(datadir)\"                        \
        -DSYSCONFDIR=\"$(sysconfdir)\"                  \
        -DGNOMELOCALEDIR="\"$(datadir)/locale\""	\
        -DLIBDIR=\"$(libdir)\"

gkbd_indicator_plugins_capplet_SOURCES = gkbd-indicator-plugins-capplet.c \
                                    gkbd-indicator-plugins-add.c \
                                    gkbd-indicator-plugins-capplet.h

gkbd_indicator_plugins_capplet_LDFLAGS = -export-dynamic

gkbd_indicator_plugins_capplet_LDADD = \
            	$(DBUS_LIBS) 	\
            	$(GCONF_LIBS) 	\
            	$(LIBGNOME_LIBS) 	\
            	$(LIBGNOMEUI_LIBS) 	\
            	$(LIBGLADE_LIBS) 	\
            	$(LIBXKLAVIER_LIBS)	\
		$(top_builddir)/libgnomekbd/libgnomekbd.la \
		$(top_builddir)/libgnomekbd/libgnomekbdui.la

EXTRA_DIST = gkbd-indicator-plugins-capplet.desktop.in.in \
             $(glade_DATA) \
             $(pixmap_DATA)

DISTCLEANFILES = $(desktop_DATA)

pixmapdir = $(datadir)/icons/hicolor/48x48/apps
pixmap_DATA = gkbd-indicator-plugins-capplet.png
