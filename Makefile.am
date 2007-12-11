if BUILD_TESTS
TEST = test
endif

SUBDIRS = libgnomekbd $(TEST) capplet po

ACLOCAL_AMFLAGS = -I m4

DISTCLEANFILES = \
  intltool-extract \
  intltool-merge \
  intltool-update
 
EXTRA_DIST = \
        libgnomekbd.pc.in \
        libgnomekbdui.pc.in \
        intltool-extract.in \
        intltool-merge.in \
        intltool-update.in \
        README.cvs \
        ChangeLog.libkbdraw ChangeLog.libgswitchit

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgnomekbd.pc libgnomekbdui.pc
