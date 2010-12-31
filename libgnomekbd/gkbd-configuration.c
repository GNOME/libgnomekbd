/*
 * Copyright (C) 2010 Canonical Ltd.
 * 
 * Authors: Jan Arne Petersen <jpetersen@openismus.com>
 * 
 * Based on gkbd-status.c by Sergey V. Udaltsov <svu@gnome.org>
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

#include <memory.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>

#include <gkbd-configuration.h>

#include <gkbd-desktop-config.h>

struct _GkbdConfigurationPrivate {
	XklEngine *engine;
	XklConfigRegistry *registry;

	GkbdDesktopConfig cfg;
	GkbdIndicatorConfig ind_cfg;
	GkbdKeyboardConfig kbd_cfg;

	gchar **full_group_names;
	gchar **short_group_names;

	const gchar *tooltips_format;

	gulong state_changed_handler;
	gulong config_changed_handler;
};

enum {
	SIGNAL_CHANGED,
	SIGNAL_GROUP_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

#define GKBD_CONFIGURATION_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), GKBD_TYPE_CONFIGURATION, GkbdConfigurationPrivate))

G_DEFINE_TYPE (GkbdConfiguration, gkbd_configuration, G_TYPE_OBJECT)

/* Should be called once for all widgets */
static void
gkbd_configuration_cfg_changed (GSettings * settings, gchar * key,
				GkbdConfiguration * configuration)
{
	GkbdConfigurationPrivate *priv = configuration->priv;

	xkl_debug (100,
		   "General configuration changed in GConf - reiniting...\n");
	gkbd_desktop_config_load (&priv->cfg);
	gkbd_desktop_config_activate (&priv->cfg);

	g_signal_emit (configuration, signals[SIGNAL_CHANGED], 0);
}

/* Should be called once for all widgets */
static void
gkbd_configuration_ind_cfg_changed (GSettings * settings, gchar * key,
				    GkbdConfiguration * configuration)
{
	GkbdConfigurationPrivate *priv = configuration->priv;
	xkl_debug (100,
		   "Applet configuration changed in GConf - reiniting...\n");
	gkbd_indicator_config_load (&priv->ind_cfg);

	gkbd_indicator_config_free_image_filenames (&priv->ind_cfg);
	gkbd_indicator_config_load_image_filenames (&priv->ind_cfg,
						    &priv->kbd_cfg);

	gkbd_indicator_config_activate (&priv->ind_cfg);

	g_signal_emit (configuration, signals[SIGNAL_CHANGED], 0);
}

static void
gkbd_configuration_load_group_names (GkbdConfiguration * configuration,
				     XklConfigRec * xklrec)
{
	GkbdConfigurationPrivate *priv = configuration->priv;

	if (!gkbd_desktop_config_load_group_descriptions (&priv->cfg,
							  priv->registry,
							  (const char **)
							  xklrec->layouts,
							  (const char **)
							  xklrec->variants,
							  &priv->
							  short_group_names,
							  &priv->
							  full_group_names))
	{
		/* We just populate no short names (remain NULL) - 
		 * full names are going to be used anyway */
		gint i, total_groups =
		    xkl_engine_get_num_groups (priv->engine);
		xkl_debug (150, "group descriptions loaded: %d!\n",
			   total_groups);

		if (xkl_engine_get_features (priv->engine) &
		    XKLF_MULTIPLE_LAYOUTS_SUPPORTED) {
			priv->full_group_names =
			    g_strdupv (priv->kbd_cfg.layouts_variants);
		} else {
			priv->full_group_names =
			    g_new0 (char *, total_groups + 1);
			for (i = total_groups; --i >= 0;) {
				priv->full_group_names[i] =
				    g_strdup_printf ("Group %d", i);
			}
		}
	}
}

/* Should be called once for all widgets */
static void
gkbd_configuration_kbd_cfg_callback (XklEngine * engine,
				     GkbdConfiguration * configuration)
{
	GkbdConfigurationPrivate *priv = configuration->priv;
	XklConfigRec *xklrec = xkl_config_rec_new ();
	xkl_debug (100,
		   "XKB configuration changed on X Server - reiniting...\n");

	gkbd_keyboard_config_load_from_x_current (&priv->kbd_cfg, xklrec);

	gkbd_indicator_config_free_image_filenames (&priv->ind_cfg);
	gkbd_indicator_config_load_image_filenames (&priv->ind_cfg,
						    &priv->kbd_cfg);

	g_strfreev (priv->full_group_names);
	priv->full_group_names = NULL;

	g_strfreev (priv->short_group_names);
	priv->short_group_names = NULL;

	gkbd_configuration_load_group_names (configuration, xklrec);

	g_signal_emit (configuration, signals[SIGNAL_CHANGED], 0);

	g_object_unref (G_OBJECT (xklrec));
}

/* Should be called once for all applets */
static void
gkbd_configuration_state_callback (XklEngine * engine,
				   XklEngineStateChange changeType,
				   gint group, gboolean restore,
				   GkbdConfiguration * configuration)
{
	xkl_debug (150, "group is now %d, restore: %d\n", group, restore);

	if (changeType == GROUP_CHANGED) {
		g_signal_emit (configuration,
			       signals[SIGNAL_GROUP_CHANGED], 0, group);
	}
}

static void
gkbd_configuration_init (GkbdConfiguration * configuration)
{
	GkbdConfigurationPrivate *priv;
	XklConfigRec *xklrec = xkl_config_rec_new ();

	priv = GKBD_CONFIGURATION_GET_PRIVATE (configuration);
	configuration->priv = priv;

	/* Initing some global vars */
	priv->tooltips_format = "%s";

	priv->engine = xkl_engine_get_instance (GDK_DISPLAY_XDISPLAY
						(gdk_display_get_default
						 ()));
	if (priv->engine == NULL) {
		xkl_debug (0, "Libxklavier initialization error");
		return;
	}

	priv->state_changed_handler =
	    g_signal_connect (priv->engine, "X-state-changed",
			      G_CALLBACK
			      (gkbd_configuration_state_callback),
			      configuration);
	priv->config_changed_handler =
	    g_signal_connect (priv->engine, "X-config-changed",
			      G_CALLBACK
			      (gkbd_configuration_kbd_cfg_callback),
			      configuration);

	gkbd_desktop_config_init (&priv->cfg, priv->engine);
	gkbd_keyboard_config_init (&priv->kbd_cfg, priv->engine);
	gkbd_indicator_config_init (&priv->ind_cfg, priv->engine);

	gkbd_desktop_config_load (&priv->cfg);
	gkbd_desktop_config_activate (&priv->cfg);

	priv->registry = xkl_config_registry_get_instance (priv->engine);
	xkl_config_registry_load (priv->registry,
				  priv->cfg.load_extra_items);

	gkbd_keyboard_config_load_from_x_current (&priv->kbd_cfg, xklrec);

	gkbd_indicator_config_load (&priv->ind_cfg);

	gkbd_indicator_config_load_image_filenames (&priv->ind_cfg,
						    &priv->kbd_cfg);

	gkbd_indicator_config_activate (&priv->ind_cfg);

	gkbd_configuration_load_group_names (configuration, xklrec);
	g_object_unref (G_OBJECT (xklrec));

	gkbd_desktop_config_start_listen (&priv->cfg,
					  G_CALLBACK
					  (gkbd_configuration_cfg_changed),
					  configuration);
	gkbd_indicator_config_start_listen (&priv->ind_cfg,
					    G_CALLBACK
					    (gkbd_configuration_ind_cfg_changed),
					    configuration);
	xkl_engine_start_listen (priv->engine, XKLL_TRACK_KEYBOARD_STATE);

	xkl_debug (100, "Initiating the widget startup process for %p\n",
		   configuration);
}

static void
gkbd_configuration_finalize (GObject * obj)
{
	GkbdConfiguration *configuration = GKBD_CONFIGURATION (obj);
	GkbdConfigurationPrivate *priv = configuration->priv;

	xkl_debug (100,
		   "Starting the gnome-kbd-configuration widget shutdown process for %p\n",
		   configuration);

	xkl_engine_stop_listen (priv->engine, XKLL_TRACK_KEYBOARD_STATE);

	gkbd_desktop_config_stop_listen (&priv->cfg);
	gkbd_indicator_config_stop_listen (&priv->ind_cfg);

	gkbd_indicator_config_term (&priv->ind_cfg);
	gkbd_keyboard_config_term (&priv->kbd_cfg);
	gkbd_desktop_config_term (&priv->cfg);

	if (g_signal_handler_is_connected (priv->engine,
					   priv->state_changed_handler)) {
		g_signal_handler_disconnect (priv->engine,
					     priv->state_changed_handler);
		priv->state_changed_handler = 0;
	}
	if (g_signal_handler_is_connected (priv->engine,
					   priv->config_changed_handler)) {
		g_signal_handler_disconnect (priv->engine,
					     priv->config_changed_handler);
		priv->config_changed_handler = 0;
	}

	g_object_unref (priv->registry);
	priv->registry = NULL;
	g_object_unref (priv->engine);
	priv->engine = NULL;

	G_OBJECT_CLASS (gkbd_configuration_parent_class)->finalize (obj);
}

static void
gkbd_configuration_class_init (GkbdConfigurationClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	/* Initing vtable */
	object_class->finalize = gkbd_configuration_finalize;

	/* Signals */
	signals[SIGNAL_CHANGED] = g_signal_new ("changed",
						GKBD_TYPE_CONFIGURATION,
						G_SIGNAL_RUN_LAST,
						0,
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
	signals[SIGNAL_GROUP_CHANGED] = g_signal_new ("group-changed",
						      GKBD_TYPE_CONFIGURATION,
						      G_SIGNAL_RUN_LAST,
						      0,
						      NULL, NULL,
						      g_cclosure_marshal_VOID__INT,
						      G_TYPE_NONE,
						      1, G_TYPE_INT);

	g_type_class_add_private (klass,
				  sizeof (GkbdConfigurationPrivate));
}

GkbdConfiguration *
gkbd_configuration_get (void)
{
	static gpointer instance = NULL;

	if (!instance) {
		instance = g_object_new (GKBD_TYPE_CONFIGURATION, NULL);
		g_object_add_weak_pointer (instance, &instance);
	} else {
		g_object_ref (instance);
	}

	return instance;
}

XklEngine *
gkbd_configuration_get_xkl_engine (GkbdConfiguration * configuration)
{
	return configuration->priv->engine;
}

gchar **
gkbd_configuration_get_group_names (GkbdConfiguration * configuration)
{
	return configuration->priv->full_group_names;
}

gchar *
gkbd_configuration_get_image_filename (GkbdConfiguration * configuration,
				       guint group)
{
	if (!configuration->priv->ind_cfg.show_flags)
		return NULL;
	return (gchar *) g_slist_nth_data (configuration->priv->ind_cfg.
					   image_filenames, group);
}

gchar **
gkbd_configuration_get_short_group_names (GkbdConfiguration *
					  configuration)
{
	return configuration->priv->short_group_names;
}

gchar *
gkbd_configuration_get_current_tooltip (GkbdConfiguration * configuration)
{
	XklState *state =
	    xkl_engine_get_current_state (configuration->priv->engine);
	gchar *buf;
	if (state == NULL || state->group < 0
	    || state->group >=
	    g_strv_length (configuration->priv->full_group_names))
		return NULL;

	return g_strdup_printf (configuration->priv->tooltips_format,
				configuration->priv->
				full_group_names[state->group]);
}

gboolean
gkbd_configuration_if_flags_shown (GkbdConfiguration * configuration)
{
	return configuration->priv->ind_cfg.show_flags;
}

gchar *
gkbd_configuration_extract_layout_name (GkbdConfiguration * configuration,
					int group)
{
	char *layout_name = NULL;
	gchar **short_group_names = configuration->priv->short_group_names;
	gchar **full_group_names = configuration->priv->full_group_names;
	XklEngine *engine = configuration->priv->engine;
	if (group < g_strv_length (short_group_names)) {
		if (xkl_engine_get_features (engine) &
		    XKLF_MULTIPLE_LAYOUTS_SUPPORTED) {
			char *full_layout_name =
			    configuration->priv->kbd_cfg.
			    layouts_variants[group];
			char *variant_name;
			if (!gkbd_keyboard_config_split_items
			    (full_layout_name, &layout_name,
			     &variant_name))
				/* just in case */
				layout_name = full_layout_name;

			/* make it freeable */
			layout_name = g_strdup (layout_name);

			if (short_group_names != NULL) {
				char *short_group_name =
				    short_group_names[group];
				if (short_group_name != NULL
				    && *short_group_name != '\0') {
					/* drop the long name */
					g_free (layout_name);
					layout_name =
					    g_strdup (short_group_name);
				}
			}
		} else {
			layout_name = g_strdup (full_group_names[group]);
		}
	}

	if (layout_name == NULL)
		layout_name = g_strdup ("");

	return layout_name;
}

void
gkbd_configuration_lock_next_group (GkbdConfiguration * configuration)
{
	gkbd_desktop_config_lock_next_group (&configuration->priv->cfg);
}

GkbdIndicatorConfig *
gkbd_configuration_get_indicator_config (GkbdConfiguration * configuration)
{
	return &configuration->priv->ind_cfg;
}

GkbdKeyboardConfig *
gkbd_configuration_get_keyboard_config (GkbdConfiguration * configuration)
{
	return &configuration->priv->kbd_cfg;
}

GSList *
gkbd_configuration_load_images (GkbdConfiguration * configuration)
{
	int i;
	GSList *image_filename, *images;

	images = NULL;
	gkbd_indicator_config_load_image_filenames (&configuration->
						    priv->ind_cfg,
						    &configuration->
						    priv->kbd_cfg);

	if (!configuration->priv->ind_cfg.show_flags)
		return NULL;

	image_filename = configuration->priv->ind_cfg.image_filenames;

	for (i =
	     xkl_engine_get_max_num_groups (configuration->priv->engine);
	     --i >= 0; image_filename = image_filename->next) {
		GdkPixbuf *image = NULL;
		char *image_file = (char *) image_filename->data;

		if (image_file != NULL) {
			GError *gerror = NULL;
			image =
			    gdk_pixbuf_new_from_file (image_file, &gerror);
			xkl_debug (150,
				   "Image %d[%s] loaded -> %p[%dx%d]\n",
				   i, image_file, image,
				   gdk_pixbuf_get_width (image),
				   gdk_pixbuf_get_height (image));
		}
		/* We append the image anyway - even if it is NULL! */
		images = g_slist_append (images, image);
	}
	return images;
}

void
gkbd_configuration_free_images (GkbdConfiguration * configuration,
				GSList * images)
{
	GdkPixbuf *pi;
	GSList *img_node;

	gkbd_indicator_config_free_image_filenames (&configuration->
						    priv->ind_cfg);

	while ((img_node = images) != NULL) {
		pi = GDK_PIXBUF (img_node->data);
		/* It can be NULL - some images may be missing */
		if (pi != NULL) {
			g_object_unref (pi);
		}
		images = g_slist_remove_link (images, img_node);
		g_slist_free_1 (img_node);
	}
}
