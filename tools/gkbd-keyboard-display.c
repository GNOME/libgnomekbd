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
#include <string.h>
#include <libintl.h>
#include <config.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <glib/gi18n-lib.h>

#include <libxklavier/xklavier.h>
#include <libgnomekbd/gkbd-keyboard-drawing.h>

#define GTK_RESPONSE_PRINT 2

GtkWidget *gkbd_keyboard_drawing_dialog_new (void);

void gkbd_keyboard_drawing_dialog_set_group (GtkWidget * dialog,
					     XklConfigRegistry * registry,
					     gint group);

void gkbd_keyboard_drawing_dialog_set_layout (GtkWidget * dialog,
					      XklConfigRegistry * registry,
					      const gchar * layout);

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

static GkbdKeyboardDrawingGroupLevel defaultGroupsLevels[] = {
	{0, 1},
	{0, 3},
	{0, 0},
	{0, 2}
};

static GkbdKeyboardDrawingGroupLevel *pGroupsLevels[] = {
	defaultGroupsLevels,
	defaultGroupsLevels + 1,
	defaultGroupsLevels + 2,
	defaultGroupsLevels + 3
};

extern gboolean xkl_xkb_config_native_prepare (XklEngine * engine,
					       const XklConfigRec * data,
					       gpointer component_names);

extern void xkl_xkb_config_native_cleanup (XklEngine * engine,
					   gpointer component_names);

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


static void
gkbd_keyboard_drawing_dialog_set_layout_name (GtkWidget * dialog,
					      const gchar * layout_name)
{
	gtk_window_set_title (GTK_WINDOW (dialog), layout_name);
	g_object_set_data_full (G_OBJECT (dialog), "layout_name",
				g_strdup (layout_name), g_free);
}

static void
gkbd_keyboard_drawing_dialog_response (GtkWidget * dialog, gint resp)
{
	GtkWidget *kbdraw;
	const gchar *groupName;

	switch (resp) {
	case GTK_RESPONSE_CLOSE:
		gtk_widget_destroy (dialog);
		break;
	case GTK_RESPONSE_PRINT:
		kbdraw =
		    GTK_WIDGET (g_object_get_data
				(G_OBJECT (dialog), "kbdraw"));
		groupName =
		    (const gchar *) g_object_get_data (G_OBJECT (dialog),
						       "groupName");
		gkbd_keyboard_drawing_print (GKBD_KEYBOARD_DRAWING
					     (kbdraw), GTK_WINDOW (dialog),
					     groupName ? groupName :
					     _("Unknown"));
	}
}

void
gkbd_keyboard_drawing_dialog_set_group (GtkWidget * dialog,
					XklConfigRegistry * registry,
					gint group)
{
	XkbComponentNamesRec component_names;
	XklConfigRec *xkl_data;
	XklEngine *engine =
	    xkl_engine_get_instance (GDK_DISPLAY_XDISPLAY
				     (gdk_display_get_default ()));

	xkl_data = xkl_config_rec_new ();
	if (xkl_config_rec_get_from_server (xkl_data, engine)) {
		int num_layouts = g_strv_length (xkl_data->layouts);
		int num_variants = g_strv_length (xkl_data->variants);
		if (group >= 0 && group < num_layouts
		    && group < num_variants) {
			XklConfigItem *xki = xkl_config_item_new ();
			gchar *l = g_strdup (xkl_data->layouts[group]);
			gchar *v = g_strdup (xkl_data->variants[group]);
			const gchar *layout_name = NULL;
			gchar **p;
			int i;

			if ((p = xkl_data->layouts) != NULL)
				for (i = num_layouts; --i >= 0;)
					g_free (*p++);

			if ((p = xkl_data->variants) != NULL)
				for (i = num_variants; --i >= 0;)
					g_free (*p++);

			xkl_data->layouts =
			    g_realloc (xkl_data->layouts,
				       sizeof (char *) * 2);
			xkl_data->variants =
			    g_realloc (xkl_data->variants,
				       sizeof (char *) * 2);
			xkl_data->layouts[0] = l;
			xkl_data->variants[0] = v;
			xkl_data->layouts[1] = xkl_data->variants[1] =
			    NULL;

			if (v[0] != 0) {
				strncpy (xki->name, v,
					 XKL_MAX_CI_NAME_LENGTH);
				xki->name[XKL_MAX_CI_NAME_LENGTH - 1] = 0;
				if (xkl_config_registry_find_variant
				    (registry, l, xki))
					layout_name = xki->description;
			} else {
				strncpy (xki->name, l,
					 XKL_MAX_CI_NAME_LENGTH);
				xki->name[XKL_MAX_CI_NAME_LENGTH - 1] = 0;
				if (xkl_config_registry_find_layout
				    (registry, xki))
					layout_name = xki->description;
			}
			gkbd_keyboard_drawing_dialog_set_layout_name
			    (dialog, layout_name);
			g_object_unref (xki);
		}

		if (xkl_xkb_config_native_prepare
		    (engine, xkl_data, &component_names)) {
			GtkWidget *kbdraw =
			    g_object_get_data (G_OBJECT (dialog),
					       "kbdraw");
			if (!gkbd_keyboard_drawing_set_keyboard
			    (GKBD_KEYBOARD_DRAWING (kbdraw),
			     &component_names))
				gkbd_keyboard_drawing_set_keyboard
				    (GKBD_KEYBOARD_DRAWING (kbdraw), NULL);
			xkl_xkb_config_native_cleanup (engine,
						       &component_names);
		}
	}

	g_object_unref (G_OBJECT (xkl_data));
}

GtkWidget *
gkbd_keyboard_drawing_dialog_new ()
{
	GtkBuilder *builder;
	GtkWidget *dialog, *kbdraw;
	GdkRectangle *rect;
	GError *error = NULL;

	builder = gtk_builder_new ();
	gtk_builder_add_from_file (builder, UIDIR "/show-layout.ui",
				   &error);

	if (error) {
		g_error ("building ui from %s failed: %s",
			 UIDIR "/show-layout.ui", error->message);
		g_clear_error (&error);
	}

	dialog =
	    GTK_WIDGET (gtk_builder_get_object
			(builder, "gswitchit_layout_view"));
	kbdraw = gkbd_keyboard_drawing_new ();

	gkbd_keyboard_drawing_set_groups_levels (GKBD_KEYBOARD_DRAWING
						 (kbdraw), pGroupsLevels);

	g_object_set_data (G_OBJECT (dialog), "builderData", builder);
	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK
			  (gkbd_keyboard_drawing_dialog_response), NULL);

	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

	gtk_box_pack_start (GTK_BOX
			    (gtk_builder_get_object
			     (builder, "preview_vbox")), kbdraw, TRUE,
			    TRUE, 0);

	g_object_set_data (G_OBJECT (dialog), "kbdraw", kbdraw);

	g_signal_connect_swapped (dialog, "destroy",
				  G_CALLBACK (g_object_unref),
				  g_object_get_data (G_OBJECT (dialog),
						     "builderData"));

	return dialog;
}

void
gkbd_keyboard_drawing_dialog_set_layout (GtkWidget * dialog,
					 XklConfigRegistry * registry,
					 const gchar * full_layout)
{
	const gchar *layout_name = "?";
	XklConfigItem *xki = xkl_config_item_new ();
	gchar *layout = NULL, *variant = NULL;

	GkbdKeyboardDrawing *kbdraw =
	    GKBD_KEYBOARD_DRAWING (g_object_get_data
				   (G_OBJECT (dialog), "kbdraw"));

	if (full_layout == NULL || full_layout[0] == 0)
		return;

	gkbd_keyboard_drawing_set_layout (kbdraw, full_layout);

	if (gkbd_keyboard_config_split_items
	    (full_layout, &layout, &variant)) {
		if (variant != NULL) {
			strncpy (xki->name, variant,
				 XKL_MAX_CI_NAME_LENGTH);
			xki->name[XKL_MAX_CI_NAME_LENGTH - 1] = 0;
			if (xkl_config_registry_find_variant
			    (registry, layout, xki))
				layout_name = xki->description;
		} else {
			strncpy (xki->name, layout,
				 XKL_MAX_CI_NAME_LENGTH);
			xki->name[XKL_MAX_CI_NAME_LENGTH - 1] = 0;
			if (xkl_config_registry_find_layout
			    (registry, xki))
				layout_name = xki->description;
		}
	}

	gkbd_keyboard_drawing_dialog_set_layout_name (dialog, layout_name);
	g_object_unref (xki);
}
