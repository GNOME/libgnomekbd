noinst_PROGRAMS = gkbd-indicator-test \
                  gkbd-keyboard-drawing-test

gkbd_indicator_test_CFLAGS=			\
	$(WARN_CFLAGS) -I$(top_srcdir) -Wall 		\
	$(GTK_CFLAGS)					\
	$(LIBGLADE_CFLAGS)				\
	$(LIBXKLAVIER_CFLAGS)				\
	-I$(top_srcdir)/intl				\
	-DSYS_PLUGIN_DIR=\"$(libdir)/gnomekbd/\"	\
	-DG_LOG_DOMAIN=\"GnomeKbdIndicatorTest\"	\
        -DGNOMELOCALEDIR="\"$(datadir)/locale\""	\
	-DDATADIR=\"$(datadir)\"

gkbd_indicator_test_LDFLAGS=			\
	$(GTK_LIBS)					\
	$(LIBGLADE_LIBS)				\
	$(LIBXKLAVIER_LIBS)				\
	$(top_builddir)/libgnomekbd/libgnomekbd.la	\
	$(top_builddir)/libgnomekbd/libgnomekbdui.la

gkbd_keyboard_drawing_test_CFLAGS=			\
	$(WARN_CFLAGS) -I$(top_srcdir)			\
	$(GTK_CFLAGS) $(GLIB_CFLAGS)

gkbd_keyboard_drawing_test_LDFLAGS=			\
	$(GLIB_LIBS)					\
	$(GTK_LIBS)					\
	$(LIBXKLAVIER_LIBS)				\
	$(top_builddir)/libgnomekbd/libgnomekbd.la	\
	$(top_builddir)/libgnomekbd/libgnomekbdui.la
