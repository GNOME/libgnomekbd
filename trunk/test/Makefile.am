noinst_PROGRAMS = gkbd-indicator-test \
                  gkbd-keyboard-drawing-test \
                  gkbd-config-registry-server

gkbd_config_registry_server_CFLAGS=			\
	$(WARN_CFLAGS) -I$(top_srcdir) -Wall -Werror			\
	$(DBUS_CFLAGS)					\
	$(LIBXKLAVIER_CFLAGS)				

gkbd_config_registry_server_LDFLAGS=			\
	$(DBUS_LIBS)					\
	$(LIBXKLAVIER_LIBS)				\
	$(top_builddir)/libgnomekbd/libgnomekbd.la

gkbd_indicator_test_CFLAGS=			\
	$(WARN_CFLAGS) -I$(top_srcdir) -Wall -Werror			\
	$(GTK_CFLAGS)					\
	$(LIBGLADE_CFLAGS)				\
	$(LIBGNOME_CFLAGS)				\
	$(LIBGNOMEUI_CFLAGS)				\
	$(LIBXKLAVIER_CFLAGS)				\
	-I$(top_srcdir)/intl				\
	-DSYS_PLUGIN_DIR=\"$(libdir)/gnomekbd/\"	\
	-DG_LOG_DOMAIN=\"GnomeKbdIndicatorTest\"	\
	-DDATADIR=\"$(datadir)\"

gkbd_indicator_test_LDFLAGS=			\
	$(GTK_LIBS)					\
	$(LIBGLADE_LIBS)				\
	$(LIBGNOME_LIBS)				\
	$(LIBGNOMEUI_LIBS)				\
	$(LIBXKLAVIER_LIBS)				\
	$(top_builddir)/libgnomekbd/libgnomekbd.la	\
	$(top_builddir)/libgnomekbd/libgnomekbdui.la

gkbd_keyboard_drawing_test_CFLAGS=				\
	$(WARN_CFLAGS) -I$(top_srcdir)			\
	$(GTK_CFLAGS)

gkbd_keyboard_drawing_test_LDFLAGS=				\
	$(GTK_LIBS)					\
	$(LIBGNOME_LIBS)				\
	$(LIBXKLAVIER_LIBS)				\
	$(top_builddir)/libgnomekbd/libgnomekbd.la	\
	$(top_builddir)/libgnomekbd/libgnomekbdui.la

EXTRA_DIST = gkbd-config-registry-client.sh
