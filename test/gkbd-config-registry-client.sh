#!/bin/sh

# 
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Library General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

#
# $Id$
#
# This tiny script is expected to test the available keyboard config registry, 
# getting descriptions out of it
#

objname=GkbdConfigRegistry
method=GetDescriptionsAsUtf8

# Old:
#objname=KeyboardConfigRegistry
#method=GetCurrentDescriptionsAsUtf8

dbus-send --session \
          --dest=org.gnome.$objname \
          --type=method_call \
          --print-reply \
          --reply-timeout=20000 \
          /org/gnome/$objname \
          org.gnome.$objname.$method \
	  array:string:"us","ru","fr","de","il" \
	  array:string:"basic","winkeys","","deadkeys","dummy"
