noinst_PROGRAMS = gkbd-indicator-test \
                  gkbd-keyboard-drawing-test \
                  gkbd-status-test

common_CFLAGS = $(WARN_CFLAGS) -I$(top_srcdir) -Wall \
	$(GTK_CFLAGS) \
	$(LIBXKLAVIER_CFLAGS) \
	-DGNOMELOCALEDIR="\"$(datadir)/locale\""
	
common_LDFLAGS = $(GTK_LIBS) \
	$(LIBXKLAVIER_LIBS) \
	$(top_builddir)/libgnomekbd/libgnomekbd.la	\
	$(top_builddir)/libgnomekbd/libgnomekbdui.la

gkbd_indicator_test_CFLAGS=$(common_CFLAGS)

gkbd_indicator_test_LDFLAGS=$(common_LDFLAGS)

gkbd_keyboard_drawing_test_CFLAGS=$(common_CFLAGS)

gkbd_keyboard_drawing_test_LDFLAGS=$(common_LDFLAGS)

gkbd_status_test_CFLAGS=$(common_CFLAGS)

gkbd_status_test_LDFLAGS=$(common_LDFLAGS)
