if BUILD_TESTS
TEST = test
endif

SUBDIRS = libgnomekbd $(TEST) po

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

DISTCHECK_CONFIGURE_FLAGS = --enable-introspection

EXTRA_DIST = \
        m4/introspection.m4 \
        libgnomekbd.pc.in \
        libgnomekbdui.pc.in \
        README.cvs \
        ChangeLog.libkbdraw ChangeLog.libgswitchit

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgnomekbd.pc libgnomekbdui.pc
