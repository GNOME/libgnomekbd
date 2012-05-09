/*
 * Copyright (C) 2010 Canonical Ltd.
 * Copyright (C) 2010-2011 Sergey V. Udaltsov <svu@gnome.org>
 * 
 * Authors: Jan Arne Petersen <jpetersen@openismus.com>
 *          Sergey V. Udaltsov <svu@gnome.org>
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

#include <memory.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <X11/XKBlib.h>
#include <glib/gi18n-lib.h>

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

	GSList *widget_instances;

	Atom caps_lock_atom;
	Atom num_lock_atom;
	Atom scroll_lock_atom;
};

enum {
	SIGNAL_CHANGED,
	SIGNAL_GROUP_CHANGED,
	SIGNAL_INDICATORS_CHANGED,
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
							  &priv->short_group_names,
							  &priv->full_group_names))
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
	xkl_debug (150, "change type: %d, group is now %d, restore: %d\n",
		   changeType, group, restore);

	switch (changeType) {
	case GROUP_CHANGED:
		g_signal_emit (configuration,
			       signals[SIGNAL_GROUP_CHANGED], 0, group);
		break;
	case INDICATORS_CHANGED:
		g_signal_emit (configuration,
			       signals[SIGNAL_INDICATORS_CHANGED], 0);
		break;
	}
}

static void
gkbd_configuration_init (GkbdConfiguration * configuration)
{
	Display *display;
	GkbdConfigurationPrivate *priv;
	XklConfigRec *xklrec = xkl_config_rec_new ();

	xkl_debug (100, "The config startup process for %p started\n",
		   configuration);

	priv = GKBD_CONFIGURATION_GET_PRIVATE (configuration);
	configuration->priv = priv;

	/* Initing some global vars */
	priv->tooltips_format = "%s";

	display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
	priv->engine = xkl_engine_get_instance (display);
	if (priv->engine == NULL) {
		xkl_debug (0, "Libxklavier initialization error");
		return;
	}

	priv->caps_lock_atom = XInternAtom (display, "Caps Lock", False);
	priv->num_lock_atom = XInternAtom (display, "Num Lock", False);
	priv->scroll_lock_atom =
	    XInternAtom (display, "Scroll Lock", False);

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

	xkl_debug (100, "The config startup process for %p completed\n",
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
	signals[SIGNAL_INDICATORS_CHANGED] =
	    g_signal_new ("indicators-changed", GKBD_TYPE_CONFIGURATION,
			  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
			  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	g_type_class_add_private (klass,
				  sizeof (GkbdConfigurationPrivate));
}

/**
 * gkbd_configuration_get:
 *
 * Returns: (transfer full): singleton of GkbdConfiguration
 */
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

/**
 * gkbd_configuration_get_xkl_engine:
 *
 * Returns: (transfer none): The engine used by GkbdConfiguration object
 */
XklEngine *
gkbd_configuration_get_xkl_engine (GkbdConfiguration * configuration)
{
	return configuration->priv->engine;
}

/**
 * gkbd_configuration_get_group_names:
 *
 * Returns: (transfer none) (array zero-terminated=1): full group names
 */
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
	return (gchar *) g_slist_nth_data (configuration->priv->
					   ind_cfg.image_filenames, group);
}

/**
 * gkbd_configuration_get_short_group_names:
 *
 * Returns: (transfer none) (array zero-terminated=1): short group names
 */
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

	if (state == NULL || state->group < 0
	    || state->group >=
	    g_strv_length (configuration->priv->full_group_names))
		return NULL;

	return g_strdup_printf (configuration->priv->tooltips_format,
				configuration->
				priv->full_group_names[state->group]);
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
			    configuration->priv->
			    kbd_cfg.layouts_variants[group];
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

void
gkbd_configuration_lock_group (GkbdConfiguration * configuration,
			       guint group)
{
	xkl_engine_lock_group (configuration->priv->engine, group);
}

guint
gkbd_configuration_get_current_group (GkbdConfiguration * configuration)
{
	XklState *state =
	    xkl_engine_get_current_state (configuration->priv->engine);
	return state ? state->group : 0;
}

/**
 * gkbd_configuration_get_indicator_config:
 *
 * Returns: (transfer none): indicator config
 */
GkbdIndicatorConfig *
gkbd_configuration_get_indicator_config (GkbdConfiguration * configuration)
{
	return &configuration->priv->ind_cfg;
}

/**
 * gkbd_configuration_get_keyboard_config:
 *
 * Returns: (transfer none): keyboard config
 */
GkbdKeyboardConfig *
gkbd_configuration_get_keyboard_config (GkbdConfiguration * configuration)
{
	return &configuration->priv->kbd_cfg;
}

/**
 * gkbd_configuration_get_all_objects:
 *
 * Returns: (transfer none) (element-type GObject): list of widgets/status icons/...
 */
GSList *
gkbd_configuration_get_all_objects (GkbdConfiguration * configuration)
{
	return configuration->priv->widget_instances;
}

extern void
gkbd_configuration_append_object (GkbdConfiguration * configuration,
				  GObject * obj)
{
	configuration->priv->widget_instances =
	    g_slist_append (configuration->priv->widget_instances, obj);
}

extern void
gkbd_configuration_remove_object (GkbdConfiguration * configuration,
				  GObject * obj)
{
	configuration->priv->widget_instances =
	    g_slist_remove (configuration->priv->widget_instances, obj);
}

/**
 * gkbd_configuration_load_images:
 *
 * Returns: (transfer full) (element-type GdkPixbuf): list of images
 */
GSList *
gkbd_configuration_load_images (GkbdConfiguration * configuration)
{
	int i;
	GSList *image_filename, *images = NULL;

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

/**
 * gkbd_configuration_free_images:
 * @images: (element-type GdkPixbuf): list of images
 */
void
gkbd_configuration_free_images (GkbdConfiguration * configuration,
				GSList * images)
{
	GdkPixbuf *pi;
	GSList *img_node;

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

gchar *
gkbd_configuration_create_label_title (int group, GHashTable ** ln2cnt_map,
				       gchar * layout_name)
{
	gpointer pcounter = NULL;
	char *prev_layout_name = NULL;
	char *lbl_title = NULL;
	int counter = 0;

	if (group == 0) {
		*ln2cnt_map =
		    g_hash_table_new_full (g_str_hash, g_str_equal,
					   g_free, NULL);
	}

	/* Process layouts with repeating description */
	if (g_hash_table_lookup_extended
	    (*ln2cnt_map, layout_name, (gpointer *) & prev_layout_name,
	     &pcounter)) {
		/* "next" same description */
		gchar appendix[10] = "";
		gint utf8length;
		gunichar cidx;
		counter = GPOINTER_TO_INT (pcounter);
		/* Unicode subscript 2, 3, 4 */
		cidx = 0x2081 + counter;
		utf8length = g_unichar_to_utf8 (cidx, appendix);
		appendix[utf8length] = '\0';
		lbl_title = g_strconcat (layout_name, appendix, NULL);
	} else {
		/* "first" time this description */
		lbl_title = g_strdup (layout_name);
	}
	g_hash_table_insert (*ln2cnt_map, layout_name,
			     GINT_TO_POINTER (counter + 1));
	return lbl_title;
}

extern gboolean
gkbd_configuration_if_any_object_exists (GkbdConfiguration * configuration)
{
	return (configuration != NULL)
	    && (g_slist_length (configuration->priv->widget_instances) !=
		0);
}

static GdkFilterReturn
gkbd_configuration_filter_x_evt (GdkXEvent * xev, GdkEvent * event,
				 GkbdConfiguration * configuration)
{
	xkl_engine_filter_events (configuration->priv->engine,
				  (XEvent *) xev);
	return GDK_FILTER_CONTINUE;
}

void
gkbd_configuration_start_listen (GkbdConfiguration * configuration)
{
	gdk_window_add_filter (NULL, (GdkFilterFunc)
			       gkbd_configuration_filter_x_evt,
			       configuration);
	gdk_window_add_filter (gdk_get_default_root_window (),
			       (GdkFilterFunc)
			       gkbd_configuration_filter_x_evt,
			       configuration);
}

void
gkbd_configuration_stop_listen (GkbdConfiguration * configuration)
{
	gdk_window_remove_filter (NULL, (GdkFilterFunc)
				  gkbd_configuration_filter_x_evt,
				  configuration);
	gdk_window_remove_filter (gdk_get_default_root_window (),
				  (GdkFilterFunc)
				  gkbd_configuration_filter_x_evt,
				  configuration);
}

/**
 * gkbd_configuration_get_group_name:
 *
 * Returns: (transfer full): group name
 */
gchar *
gkbd_configuration_get_group_name (GkbdConfiguration * configuration,
				   guint group)
{
	gchar *layout, *variant;
	gchar **lv;

	if (configuration == NULL)
		return NULL;

	lv = configuration->priv->kbd_cfg.layouts_variants;
	if (group >= g_strv_length (lv))
		return NULL;

	if (gkbd_keyboard_config_split_items
	    (lv[group], &layout, &variant)) {
		return g_strdup (layout);
	}
	return NULL;
}

gboolean
gkbd_configuration_get_caps_lock_state (GkbdConfiguration * configuration)
{
	Bool state;
	Display *display =
	    GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
	XkbGetNamedIndicator (display, configuration->priv->caps_lock_atom,
			      NULL, &state, NULL, NULL);
	return state;
}

gboolean
gkbd_configuration_get_num_lock_state (GkbdConfiguration * configuration)
{
	Bool state;
	Display *display =
	    GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
	XkbGetNamedIndicator (display, configuration->priv->num_lock_atom,
			      NULL, &state, NULL, NULL);
	return state;
}

gboolean
gkbd_configuration_get_scroll_lock_state (GkbdConfiguration *
					  configuration)
{
	Bool state;
	Display *display =
	    GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
	XkbGetNamedIndicator (display,
			      configuration->priv->scroll_lock_atom, NULL,
			      &state, NULL, NULL);
	return state;
}
