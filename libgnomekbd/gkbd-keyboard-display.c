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

#include <stdlib.h>
#include <libintl.h>
#include <config.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <libxklavier/xklavier.h>
#include <libgnomekbd/gkbd-keyboard-drawing.h>

static GMainLoop *loop;
static gint group = 0;
static gchar *layout = NULL;
static GOptionEntry options[] = {
	{"group", 'g', 0, G_OPTION_ARG_INT, &group, "Group to display",
	 "group number (1, 2, 3, 4)"},
	{"layout", 'l', 0, G_OPTION_ARG_STRING, &layout,
	 "Layout to display", "layout (with optional variant)"},
	{ NULL }
};

static void
destroy_dialog ()
{
	g_main_loop_quit (loop);
}

int
main (int argc, char **argv)
{
	Display *display;
	GError *error = NULL;
	XklEngine *engine = NULL;
	GtkWidget *dlg = NULL;
	XklConfigRegistry *registry;

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gdk_set_allowed_backends ("x11");
	gtk_init_with_args (&argc, &argv, NULL, options, NULL, &error);

	if (error != NULL) {
		g_critical ("Error initializing GTK: %s", error->message);
		exit (1);
	}

	if (layout == NULL && group == 0) {
		g_critical ("Either layout or group have to be specified");
		exit (1);
	}

	display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
	engine = xkl_engine_get_instance (display);

	if (group < 0 || group > xkl_engine_get_num_groups (engine)) {
		g_critical ("The group number is invalid: %d", group);
		exit (2);
	}

	dlg = gkbd_keyboard_drawing_dialog_new ();
	registry = xkl_config_registry_get_instance (engine);
	xkl_config_registry_load (registry, TRUE);
	if (layout != NULL) {
		gkbd_keyboard_drawing_dialog_set_layout (dlg, registry,
							 layout);
	} else
		gkbd_keyboard_drawing_dialog_set_group (dlg, registry,
							group - 1);
	g_object_unref (registry);

	g_signal_connect (G_OBJECT (dlg), "destroy", destroy_dialog, NULL);

	gtk_widget_show_all (dlg);

	loop = g_main_loop_new (NULL, TRUE);

	g_main_loop_run (loop);

	return 0;
}
