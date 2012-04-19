#!/usr/bin/python
#
# Copyright (C) 2012  Red Hat, Inc.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions of
# the GNU General Public License v.2, or (at your option) any later version.
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY expressed or implied, including the implied warranties of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.  You should have received a copy of the
# GNU General Public License along with this program; if not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.  Any Red Hat trademarks that are incorporated in the
# source code or documentation are not subject to the GNU General Public
# License and may only be used or replicated with the express permission of
# Red Hat, Inc.
#
# Red Hat Author(s): Vratislav Podzimek <vpodzime@redhat.com>
#

import sys
from gi.repository import Gkbd, Gtk, Xkl, Gdk, GdkX11

if len(sys.argv) < 2:
    print "Layout expected as the first argument!"
    sys.exit(1)

dialog = Gkbd.KeyboardDrawing.dialog_new()
dialog.connect("destroy", lambda x: Gtk.main_quit())

display = GdkX11.x11_get_default_xdisplay()
engine = Xkl.Engine.get_instance(display)
registry = Xkl.ConfigRegistry.get_instance(engine)
registry.load(False)

Gkbd.KeyboardDrawing.dialog_set_layout(dialog, registry, sys.argv[1])

dialog.show_all()
Gtk.main()
