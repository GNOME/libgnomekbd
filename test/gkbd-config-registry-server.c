/*
 * Copyright (C) 2006 Sergey V. Udaltsov <svu@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <stdio.h>
#include <X11/Xlib.h>
#include <libxklavier/xklavier.h>
#include <dbus/dbus-glib-bindings.h>

#ifdef HAVE_SETLOCALE
# include <locale.h>
#endif

#include "libgnomekbd/gkbd-config-registry.h"

static GMainLoop *loop;

int
main ()
{
	g_type_init_with_debug_flags (G_TYPE_DEBUG_OBJECTS |
				      G_TYPE_DEBUG_SIGNALS);

#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL, "");
#endif

	GkbdConfigRegistry *reg =
	    GKBD_CONFIG_REGISTRY (g_object_new
				  (gkbd_config_registry_get_type
				   (), NULL));

	loop = g_main_loop_new (NULL, FALSE);

	g_main_loop_run (loop);

	g_object_unref (reg);

	return 0;
}
