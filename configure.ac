dnl ***************************************************************************
dnl *** configure.in for libgnomekbd                                        ***
dnl ***************************************************************************
AC_INIT(libgnomekbd, 3.28.1, [https://gitlab.gnome.org/GNOME/libgnomekbd/-/issues])
AC_PREREQ(2.62)

VERSION_INFO=8:0:0
AC_SUBST(VERSION_INFO)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE([-Wno-portability])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR(m4)

dnl ***************************************************************************
dnl *** Minimum library versions for libgnomekbd                            ***
dnl ***************************************************************************
GLIB_REQUIRED=2.44
GDK_REQUIRED=2.91.7
GTK_REQUIRED=2.90
LIBXKLAVIER_REQUIRED=5.2

AC_SUBST([GDK_REQUIRED])
AC_SUBST([GTK_REQUIRED])
AC_SUBST([LIBXKLAVIER_REQUIRED])

dnl ***************************************************************************

GLIB_GSETTINGS

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CC_C_O
AC_STDC_HEADERS
AM_PROG_LIBTOOL

# ****
# i18n
# ****
# FIXME: Remove AM_GNU_GETTEXT_VERSION once autoreconf supports REQUIRE_VERSION
AM_GNU_GETTEXT_VERSION([0.19.6])
AM_GNU_GETTEXT_REQUIRE_VERSION([0.19.6])
AM_GNU_GETTEXT([external])

GETTEXT_PACKAGE=libgnomekbd
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])

# *****************************************************************************

AC_CHECK_FUNCS(setlocale)

AC_CHECK_LIBM
AC_SUBST(LIBM)

PKG_CHECK_MODULES(GLIB, glib-2.0 >= $GLIB_REQUIRED gio-2.0 >= $GLIB_REQUIRED)

AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(GDK, gdk-3.0 >= $GDK_REQUIRED)

AC_SUBST(GDK_CFLAGS)
AC_SUBST(GDK_LIBS)

PKG_CHECK_MODULES(GTK, gtk+-3.0 >= $GTK_REQUIRED)

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(LIBXKLAVIER, libxklavier >= $LIBXKLAVIER_REQUIRED)

AC_SUBST(LIBXKLAVIER_CFLAGS)
AC_SUBST(LIBXKLAVIER_LIBS)

AC_PATH_XTRA
XLIB_CFLAGS="$X_CFLAGS"
XLIB_LIBS="$X_LIBS -lX11 $X_EXTRA_LIBS"
AC_SUBST(XLIB_CFLAGS)
AC_SUBST(XLIB_LIBS)

AC_ARG_ENABLE([tests],
              AC_HELP_STRING([--disable-tests], [Dont build the test apps]),,
              [enable_tests=yes])

AM_CONDITIONAL(BUILD_TESTS, test "$enable_tests" = "yes")

GOBJECT_INTROSPECTION_CHECK([0.6.7])

AC_OUTPUT([
Makefile
libgnomekbd.pc
libgnomekbdui.pc
po/Makefile.in
libgnomekbd/Makefile
libgnomekbd/org.gnome.libgnomekbd.desktop.gschema.xml
libgnomekbd/org.gnome.libgnomekbd.keyboard.gschema.xml
libgnomekbd/org.gnome.libgnomekbd.gschema.xml
test/Makefile
])
