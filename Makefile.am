if BUILD_TESTS
TEST = test
endif

SUBDIRS = libgnomekbd $(TEST) po

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

DISTCHECK_CONFIGURE_FLAGS = --enable-introspection

DISTCLEANFILES = \
  intltool-extract \
  intltool-merge \
  intltool-update
 
EXTRA_DIST = \
        m4/introspection.m4 \
        libgnomekbd.pc.in \
        libgnomekbdui.pc.in \
        intltool-extract.in \
        intltool-merge.in \
        intltool-update.in \
        README.cvs \
        ChangeLog.libkbdraw ChangeLog.libgswitchit

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgnomekbd.pc libgnomekbdui.pc
