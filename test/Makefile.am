noinst_PROGRAMS = gkbd-indicator-test \
                  gkbd-keyboard-drawing-test \
                  gkbd-status-test

CFLAGS_common = $(WARN_CFLAGS) -I$(top_srcdir) -Wall \
	$(GTK_CFLAGS) \
	$(LIBXKLAVIER_CFLAGS) \
	-DGNOMELOCALEDIR="\"$(datadir)/locale\""
	
LDFLAGS_common = $(GTK_LIBS) \
	$(LIBXKLAVIER_LIBS) \
	$(top_builddir)/libgnomekbd/libgnomekbd.la	\
	$(top_builddir)/libgnomekbd/libgnomekbdui.la

gkbd_indicator_test_CFLAGS=$(CFLAGS_common)

gkbd_indicator_test_LDFLAGS=$(LDFLAGS_common)

gkbd_keyboard_drawing_test_CFLAGS=$(CFLAGS_common)

gkbd_keyboard_drawing_test_LDFLAGS=$(LDFLAGS_common)

gkbd_status_test_CFLAGS=$(CFLAGS_common)

gkbd_status_test_LDFLAGS=$(LDFLAGS_common)
