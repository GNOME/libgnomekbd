bin_PROGRAMS=gkbd-keyboard-display

lib_LTLIBRARIES = libgnomekbd.la libgnomekbdui.la

uidir=$(pkgdatadir)/ui
ui_DATA = show-layout.ui

iconsdir=$(pkgdatadir)/icons
icons_DATA = kbdled-caps-lock.svg \
             kbdled-num-lock.svg \
             kbdled-scroll-lock.svg 

gsettings_SCHEMAS = org.gnome.libgnomekbd.desktop.gschema.xml \
		    org.gnome.libgnomekbd.keyboard.gschema.xml \
		    org.gnome.libgnomekbd.gschema.xml
@GSETTINGS_RULES@

common_CFLAGS = \
	-I$(top_srcdir) 				\
	$(GDK_CFLAGS)					\
	$(LIBXKLAVIER_CFLAGS)				\
	$(XLIB_CFLAGS)					\
	-I$(top_srcdir)/intl				\
	-DG_LOG_DOMAIN=\"GnomeKbdIndicator\"		\
	-DDATADIR=\"$(datadir)\"			\
	-DICONSDIR=\"$(iconsdir)\"			\
	-DSYSCONFDIR=\"$(sysconfdir)\"			\
	-DLIBDIR=\"$(libdir)\"				

libgnomekbd_la_CFLAGS = $(common_CFLAGS)

libgnomekbdui_la_CFLAGS = $(common_CFLAGS)		\
	-DUIDIR=\"$(uidir)\"				\
	$(GLIB_CFLAGS) \
	$(GTK_CFLAGS)

common_LDFLAGS = -version-info @VERSION_INFO@ -no-undefined

common_LIBADD = \
		$(GLIB_LIBS) \
		$(GDK_LIBS) \
		$(LIBXKLAVIER_LIBS) \
		$(XLIB_LIBS)

libgnomekbd_la_LDFLAGS = $(common_LDFLAGS)
libgnomekbd_la_LIBADD =  $(common_LIBADD)

libgnomekbdui_la_LDFLAGS = $(common_LDFLAGS)
libgnomekbdui_la_LIBADD = libgnomekbd.la \
                           $(common_LIBADD) \
                           $(GTK_LIBS) \
                           $(LIBM)

libgnomekbd_la_SOURCES = \
                         gkbd-desktop-config.c \
                         gkbd-keyboard-config.c \
                         gkbd-util.c

libgnomekbdui_la_SOURCES = \
                         gkbd-configuration.c \
                         gkbd-indicator-config.c \
                         gkbd-indicator.c \
                         gkbd-status.c \
                         gkbd-indicator-marshal.c \
                         gkbd-keyboard-drawing-marshal.c \
                         gkbd-keyboard-drawing.c

gkbd_keyboard_display_SOURCES=gkbd-keyboard-display.c libgnomekbdui.la

gkbd_keyboard_display_CFLAGS=$(common_CFLAGS) \
        -DGNOMELOCALEDIR="\"$(datadir)/locale\"" \
	$(GLIB_CFLAGS) \
	$(GTK_CFLAGS)

gkbd_keyboard_display_LDADD = \
                $(GCONF_LIBS)   \
                $(GTK_LIBS)     \
                $(LIBXKLAVIER_LIBS)     \
                libgnomekbd.la \
                libgnomekbdui.la

BUILT_SOURCES = gkbd-indicator-marshal.c \
	gkbd-indicator-marshal.h         \
        gkbd-keyboard-drawing-marshal.c            \
        gkbd-keyboard-drawing-marshal.h

DISTCLEANFILES = $(BUILT_SOURCES) \
        $(desktop_DATA)

MAINTAINERCLEANFILES =                  \
        $(desktop_DATA)			\
        *~                              \
        Makefile.in

gnomekbdincdir = $(includedir)/libgnomekbd
gnomekbdinc_HEADERS = \
                      gkbd-configuration.h \
                      gkbd-desktop-config.h \
                      gkbd-keyboard-config.h \
                      gkbd-indicator.h \
                      gkbd-status.h \
                      gkbd-indicator-config.h \
                      gkbd-keyboard-drawing.h \
                      gkbd-util.h

# Rule to create the .desktop files
desktopdir = $(datadir)/applications
desktop_in_files = gkbd-keyboard-display.desktop.in
desktop_DATA = gkbd-keyboard-display.desktop

$(desktop_in_files): gkbd-keyboard-display.desktop.in.in
	$(AM_V_GEN) $(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

# Substitute variables in a Desktop Entry file.
$(desktop_DATA): $(desktop_in_files)
	$(AM_V_GEN) sed -e 's|@VERSION[@]|$(VERSION)|g' $< > $@

noinst_HEADERS = $(extra_nih) \
                 gkbd-config-private.h

convertdir = $(datadir)/GConf/gsettings
convert_DATA = libgnomekbd.convert

EXTRA_DIST = gkbd-indicator-marshal.list \
             gkbd-keyboard-drawing-marshal.list \
             $(ui_DATA) \
             $(convert_DATA) \
             $(icons_DATA) \
             $(desktop_in_files) \
             $(gsettings_SCHEMAS:.xml=.xml.in)


GLIB_GENMARSHAL = $(shell pkg-config --variable=glib_genmarshal glib-2.0)

gkbd-indicator-marshal.h: gkbd-indicator-marshal.list
	$(AM_V_GEN)$(GLIB_GENMARSHAL) --prefix=gkbd_indicator $(srcdir)/gkbd-indicator-marshal.list --header > $@

gkbd-indicator-marshal.c: gkbd-indicator-marshal.h
	$(AM_V_GEN)$(GLIB_GENMARSHAL) --prefix=gkbd_indicator $(srcdir)/gkbd-indicator-marshal.list --body > $@

gkbd-keyboard-drawing-marshal.h: gkbd-keyboard-drawing-marshal.list
	$(AM_V_GEN)( $(GLIB_GENMARSHAL) --prefix=gkbd_keyboard_drawing $(srcdir)/gkbd-keyboard-drawing-marshal.list \
                            --header > gkbd-keyboard-drawing-marshal.tmp \
          && mv gkbd-keyboard-drawing-marshal.tmp gkbd-keyboard-drawing-marshal.h ) \
        || ( rm -f gkbd-keyboard-drawing-marshal.tmp && exit 1 )
 
gkbd-keyboard-drawing-marshal.c: gkbd-keyboard-drawing-marshal.h
	$(AM_V_GEN)( $(GLIB_GENMARSHAL) --prefix=gkbd_keyboard_drawing $(srcdir)/gkbd-keyboard-drawing-marshal.list \
                            --body > gkbd-keyboard-drawing-marshal.tmp \
          && mv gkbd-keyboard-drawing-marshal.tmp gkbd-keyboard-drawing-marshal.c ) \
        || ( rm -f gkbd-keyboard-drawing-marshal.tmp && exit 1 )

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = $(gnomekbdinc_HEADERS) \
        $(libgnomekbd_la_SOURCES) \
        $(libgnomekbdui_la_SOURCES)

Gkbd-3.0.gir: libgnomekbd.la libgnomekbdui.la
Gkbd_3_0_gir_INCLUDES = GObject-2.0 Gtk-3.0 xlib-2.0 Xkl-1.0
Gkbd_3_0_gir_CFLAGS = $(libgnomekbdui_la_CFLAGS)
Gkbd_3_0_gir_LIBS = libgnomekbd.la libgnomekbdui.la
Gkbd_3_0_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += Gkbd-3.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = $(gir_DATA) $(typelib_DATA)
endif
