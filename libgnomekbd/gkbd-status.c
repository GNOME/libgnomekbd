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

#include <memory.h>

#include <cairo.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <glib/gi18n-lib.h>

#include <gkbd-status.h>

#include <gkbd-desktop-config.h>
#include <gkbd-indicator-config.h>
#include <gkbd-configuration.h>

typedef struct _gki_globals {
	GkbdConfiguration *config;

	gint current_width;
	gint current_height;
	int real_width;

	GSList *icons;		/* list of GdkPixbuf */
} gki_globals;

static gchar *settings_signal_names[] = {
	"notify::gtk-theme-name",
	"notify::gtk-key-theme-name",
	"notify::gtk-font-name",
	"notify::font-options",
};

struct _GkbdStatusPrivate {
	gulong settings_signal_handlers[sizeof (settings_signal_names) /
					sizeof (settings_signal_names[0])];
};

/* one instance for ALL widgets */
static gki_globals globals;

G_DEFINE_TYPE (GkbdStatus, gkbd_status, GTK_TYPE_STATUS_ICON)

typedef struct {
	GtkWidget *tray_icon;
} GkbdStatusPrivHack;

static void
gkbd_status_global_init (void);
static void
gkbd_status_global_term (void);
static GdkPixbuf *
gkbd_status_prepare_drawing (GkbdStatus * gki, int group);
static void
gkbd_status_set_current_page_for_group (GkbdStatus * gki, int group);
static void
gkbd_status_set_current_page (GkbdStatus * gki);
static void
gkbd_status_reinit_globals (GkbdStatus * gki);
static void
gkbd_status_cleanup_icons (void);
static void
gkbd_status_fill_icons (GkbdStatus * gki);
static void
gkbd_status_set_tooltips (GkbdStatus * gki, const char *str);

void
gkbd_status_set_tooltips (GkbdStatus * gki, const char *str)
{
	g_assert (str == NULL || g_utf8_validate (str, -1, NULL));

	gtk_status_icon_set_tooltip_text (GTK_STATUS_ICON (gki), str);
}

void
gkbd_status_cleanup_icons ()
{
	while (globals.icons) {
		if (globals.icons->data)
			g_object_unref (G_OBJECT (globals.icons->data));
		globals.icons =
		    g_slist_delete_link (globals.icons, globals.icons);
	}
}

static void
gkbd_status_fill_icons (GkbdStatus * gki)
{
	int grp;
	int total_groups =
	    xkl_engine_get_num_groups (gkbd_configuration_get_xkl_engine
				       (globals.config));

	for (grp = 0; grp < total_groups; grp++) {
		GdkPixbuf *page = gkbd_status_prepare_drawing (gki, grp);
		globals.icons = g_slist_append (globals.icons, page);
	}
}

static void
gkbd_status_activate (GkbdStatus * gki)
{
	xkl_debug (150, "Mouse button pressed on applet\n");
	gkbd_configuration_lock_next_group (globals.config);
}

static void
gkbd_status_render_cairo (GkbdStatusPrivHack * gkh, cairo_t * cr, int group)
{
	double r, g, b;
	GdkRGBA *fg_color;
	gchar *font_family;
	int font_size;
	PangoFontDescription *pfd;
	PangoContext *pcc;
	PangoLayout *pl;
	int lwidth, lheight;
	gchar *layout_name, *lbl_title;
	cairo_font_options_t *fo;
	static GHashTable *ln2cnt_map = NULL;

	GkbdIndicatorConfig *ind_cfg =
	    gkbd_configuration_get_indicator_config (globals.config);

	xkl_debug (160, "Rendering cairo for group %d\n", group);
	if (ind_cfg->background_color != NULL &&
	    ind_cfg->background_color[0] != 0) {
		if (sscanf
		    (ind_cfg->background_color, "%lg %lg %lg", &r,
		     &g, &b) == 3) {
			cairo_set_source_rgb (cr, r, g, b);
			cairo_rectangle (cr, 0, 0, globals.current_width,
					 globals.current_height);
			cairo_fill (cr);
		}
	}

	g_object_get (gkh->tray_icon, "fg-color", &fg_color, NULL);
	cairo_set_source_rgb (cr, fg_color->red, fg_color->green, fg_color->blue);
	gdk_rgba_free (fg_color);

	gkbd_indicator_config_get_font_for_widget (ind_cfg,
						   gkh->tray_icon,
						   &font_family,
						   &font_size);

	if (font_family != NULL && font_family[0] != 0) {
		cairo_select_font_face (cr, font_family,
					CAIRO_FONT_SLANT_NORMAL,
					CAIRO_FONT_WEIGHT_NORMAL);
	}

	pfd = pango_font_description_new ();
	pango_font_description_set_family (pfd, font_family);
	pango_font_description_set_style (pfd, PANGO_STYLE_NORMAL);
	pango_font_description_set_weight (pfd, PANGO_WEIGHT_NORMAL);
	pango_font_description_set_size (pfd,
					 ind_cfg->font_size * PANGO_SCALE);

	g_free (font_family);

	pcc = pango_cairo_create_context (cr);

	fo = cairo_font_options_copy (gdk_screen_get_font_options
				      (gdk_screen_get_default ()));
	/* SUBPIXEL antialiasing gives bad results on in-memory images */
	if (cairo_font_options_get_antialias (fo) ==
	    CAIRO_ANTIALIAS_SUBPIXEL)
		cairo_font_options_set_antialias (fo,
						  CAIRO_ANTIALIAS_GRAY);
	pango_cairo_context_set_font_options (pcc, fo);

	pl = pango_layout_new (pcc);

	layout_name =
	    gkbd_configuration_extract_layout_name (globals.config, group);
	lbl_title =
	    gkbd_configuration_create_label_title (group, &ln2cnt_map,
						   layout_name);

	if (group + 1 ==
	    xkl_engine_get_num_groups (gkbd_configuration_get_xkl_engine
				       (globals.config))) {
		g_hash_table_destroy (ln2cnt_map);
		ln2cnt_map = NULL;
	}

	pango_layout_set_text (pl, lbl_title, -1);

	g_free (lbl_title);

	pango_layout_set_font_description (pl, pfd);
	pango_layout_get_size (pl, &lwidth, &lheight);

	cairo_move_to (cr,
		       (globals.current_width - lwidth / PANGO_SCALE) / 2,
		       (globals.current_height -
			lheight / PANGO_SCALE) / 2);

	pango_cairo_show_layout (cr, pl);

	pango_font_description_free (pfd);
	g_object_unref (pl);
	g_object_unref (pcc);
	cairo_font_options_destroy (fo);
	cairo_destroy (cr);

	globals.real_width = (lwidth / PANGO_SCALE) + 4;
	if (globals.real_width > globals.current_width)
		globals.real_width = globals.current_width;
	if (globals.real_width < globals.current_height)
		globals.real_width = globals.current_height;
}

static inline guint8
convert_color_channel (guint8 src, guint8 alpha)
{
	return alpha ? ((((guint) src) << 8) - src) / alpha : 0;
}

static void
convert_bgra_to_rgba (guint8 const *src, guint8 * dst, int width,
		      int height, int new_width)
{
	int xoffset = width - new_width;

	/* *4 */
	int ptr_step = xoffset << 2;

	int x, y;

	/* / 2 * 4 */
	src = src + ((xoffset >> 1) << 2);

	for (y = height; --y >= 0; src += ptr_step) {
		for (x = new_width; --x >= 0;) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
			dst[0] = convert_color_channel (src[2], src[3]);
			dst[1] = convert_color_channel (src[1], src[3]);
			dst[2] = convert_color_channel (src[0], src[3]);
			dst[3] = src[3];
#else
			dst[0] = convert_color_channel (src[1], src[0]);
			dst[1] = convert_color_channel (src[2], src[0]);
			dst[2] = convert_color_channel (src[3], src[0]);
			dst[3] = src[0];
#endif
			dst += 4;
			src += 4;
		}
	}
}

static GdkPixbuf *
gkbd_status_prepare_drawing (GkbdStatus * gki, int group)
{
	GError *gerror = NULL;
	char *image_filename;
	GdkPixbuf *image;

	if (globals.current_width == 0)
		return NULL;

	if (gkbd_configuration_if_flags_shown (globals.config)) {

		image_filename =
		    gkbd_configuration_get_image_filename (globals.config,
							   group);

		image = gdk_pixbuf_new_from_file_at_size (image_filename,
							  globals.current_width,
							  globals.current_height,
							  &gerror);

		if (image == NULL) {
			GtkWidget *dialog = gtk_message_dialog_new (NULL,
								    GTK_DIALOG_DESTROY_WITH_PARENT,
								    GTK_MESSAGE_ERROR,
								    GTK_BUTTONS_OK,
								    _
								    ("There was an error loading an image: %s"),
								    gerror
								    ==
								    NULL ?
								    "Unknown"
								    :
								    gerror->message);
			g_signal_connect (G_OBJECT (dialog), "response",
					  G_CALLBACK (gtk_widget_destroy),
					  NULL);

			gtk_window_set_resizable (GTK_WINDOW (dialog),
						  FALSE);

			gtk_widget_show (dialog);
			g_error_free (gerror);

			return NULL;
		}
		xkl_debug (150,
			   "Image %d[%s] loaded -> %p[%dx%d], alpha: %d\n",
			   group, image_filename, image,
			   gdk_pixbuf_get_width (image),
			   gdk_pixbuf_get_height (image),
			   gdk_pixbuf_get_has_alpha (image));

		return image;
	} else {
		cairo_surface_t *cs =
		    cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
						globals.current_width,
						globals.current_height);
		unsigned char *cairo_data;
		guchar *pixbuf_data;
		gkbd_status_render_cairo ((GkbdStatusPrivHack *) GTK_STATUS_ICON (gki)->priv,
					  cairo_create (cs), group);
		cairo_data = cairo_image_surface_get_data (cs);
#if 0
		char pngfilename[20];
		g_sprintf (pngfilename, "label%d.png", group);
		cairo_surface_write_to_png (cs, pngfilename);
		xkl_debug (150, "file %s is created\n", pngfilename);
#endif
		pixbuf_data =
		    g_new0 (guchar,
			    4 * globals.real_width *
			    globals.current_height);
		convert_bgra_to_rgba (cairo_data, pixbuf_data,
				      globals.current_width,
				      globals.current_height,
				      globals.real_width);

		cairo_surface_destroy (cs);

		image = gdk_pixbuf_new_from_data (pixbuf_data,
						  GDK_COLORSPACE_RGB,
						  TRUE,
						  8,
						  globals.real_width,
						  globals.current_height,
						  globals.real_width *
						  4,
						  (GdkPixbufDestroyNotify)
						  g_free, NULL);
		xkl_debug (150,
			   "Image %d created -> %p[%dx%d], alpha: %d\n",
			   group, image, gdk_pixbuf_get_width (image),
			   gdk_pixbuf_get_height (image),
			   gdk_pixbuf_get_has_alpha (image));

		return image;
	}
	return NULL;
}

static void
gkbd_status_update_tooltips (GkbdStatus * gki)
{
	gchar *buf =
	    gkbd_configuration_get_current_tooltip (globals.config);
	if (buf != NULL) {
		gkbd_status_set_tooltips (gki, buf);
		g_free (buf);
	}
}

static void
gkbd_status_reinit_globals (GkbdStatus * gki)
{
	gkbd_status_cleanup_icons ();
	gkbd_status_fill_icons (gki);
}

void
gkbd_status_reinit_ui (GkbdStatus * gki)
{
	gkbd_status_set_current_page (gki);
        /* To work around combined bugs in notification-area
         * and GtkStatusIcon, reshow the icon here, to ensure
         * size changes are picked up.
         */
        gtk_status_icon_set_visible (GTK_STATUS_ICON (gki), FALSE);
        gtk_status_icon_set_visible (GTK_STATUS_ICON (gki), TRUE);
}

/* Should be called once for all widgets */
static void
gkbd_status_cfg_callback (GkbdConfiguration * configuration)
{
	GSList *objects;
	xkl_debug (150, "Config changed: reinit ui\n");
	objects = gkbd_configuration_get_all_objects (configuration);
	if (objects)
		gkbd_status_reinit_globals (objects->data);
	ForAllObjects (configuration) {
		gkbd_status_reinit_ui (GKBD_STATUS (gki));
	} NextObject ()
}

/* Should be called once for all applets */
static void
gkbd_status_state_callback (GkbdConfiguration * configuration, gint group)
{
	xkl_debug (150, "Set page to group %d\n", group);
	ForAllObjects (configuration) {
		xkl_debug (150, "do repaint for icon %p\n", gki);
		gkbd_status_set_current_page_for_group (GKBD_STATUS (gki),
							group);
	}
	NextObject ()
}

void
gkbd_status_set_current_page (GkbdStatus * gki)
{
	XklEngine *engine =
	    gkbd_configuration_get_xkl_engine (globals.config);
	XklState *cur_state = xkl_engine_get_current_state (engine);
	if (cur_state->group >= 0)
		gkbd_status_set_current_page_for_group (gki,
							cur_state->group);
}

void
gkbd_status_set_current_page_for_group (GkbdStatus * gki, int group)
{
	GdkPixbuf *page =
	    GDK_PIXBUF (g_slist_nth_data (globals.icons, group));
	xkl_debug (150, "Revalidating for group %d: %p\n", group, page);

	if (page == NULL) {
		xkl_debug (0, "Page for group %d is not ready\n", group);
		return;
	}

	gtk_status_icon_set_from_pixbuf (GTK_STATUS_ICON (gki), page);

	gkbd_status_update_tooltips (gki);
}

/* Should be called once for all widgets */
static GdkFilterReturn
gkbd_status_filter_x_evt (GdkXEvent * xev, GdkEvent * event)
{
	XEvent *xevent = (XEvent *) xev;
	XklEngine *engine =
	    gkbd_configuration_get_xkl_engine (globals.config);

	xkl_engine_filter_events (engine, xevent);
	switch (xevent->type) {
	case ReparentNotify:
		{
			XReparentEvent *rne = (XReparentEvent *) xev;

			ForAllObjects (globals.config) {
				guint32 xid =
				    gtk_status_icon_get_x11_window_id
				    (GTK_STATUS_ICON (gki));

				/* compare the indicator's parent window with the even window */
				if (xid == rne->window) {
					/* if so - make it transparent... */
					xkl_engine_set_window_transparent
					    (engine, rne->window, TRUE);
				}
			}
		NextObject ()}
		break;
	}
	return GDK_FILTER_CONTINUE;
}


/* Should be called once for all widgets */
static void
gkbd_status_start_listen (void)
{
	gdk_window_add_filter (NULL, (GdkFilterFunc)
			       gkbd_status_filter_x_evt, NULL);
	gdk_window_add_filter (gdk_get_default_root_window (),
			       (GdkFilterFunc) gkbd_status_filter_x_evt,
			       NULL);
}

/* Should be called once for all widgets */
static void
gkbd_status_stop_listen (void)
{
	gdk_window_remove_filter (NULL, (GdkFilterFunc)
				  gkbd_status_filter_x_evt, NULL);
	gdk_window_remove_filter
	    (gdk_get_default_root_window (),
	     (GdkFilterFunc) gkbd_status_filter_x_evt, NULL);
}

static void
gkbd_status_size_changed (GkbdStatus * gki, gint size)
{
	xkl_debug (150, "Size changed to %d\n", size);
        /* Ignore the initial size 200 that we get before
         * we are embedded
         */
        if (!gtk_status_icon_is_embedded (GTK_STATUS_ICON (gki)))
                return;
	if (globals.current_height != size) {
		globals.current_height = size;
		globals.current_width = size * 3 / 2;
		gkbd_status_reinit_globals (gki);
		gkbd_status_reinit_ui (gki);
	}
}

static void
gkbd_status_theme_changed (GtkSettings * settings, GParamSpec * pspec,
			   GkbdStatus * gki)
{
	xkl_debug (150, "Theme changed\n");
	gkbd_indicator_config_refresh_style
	    (gkbd_configuration_get_indicator_config (globals.config));
	gkbd_status_reinit_globals (gki);
	gkbd_status_reinit_ui (gki);
}

static void
gkbd_status_init (GkbdStatus * gki)
{
	int i;

	if (!gkbd_configuration_if_any_object_exists (globals.config))
		gkbd_status_global_init ();

	gki->priv = g_new0 (GkbdStatusPrivate, 1);

	/* This should give Notification Area a hint about the order of icons */
	gtk_status_icon_set_name (GTK_STATUS_ICON (gki), "keyboard");

	xkl_debug (100, "The status icon startup process for %p started\n",
		   gki);

	if (gkbd_configuration_get_xkl_engine (globals.config) == NULL) {
		gkbd_status_set_tooltips (gki,
					  _("XKB initialization error"));
		return;
	}

	/* append AFTER all initialization work is finished */
	gkbd_configuration_append_object (globals.config, G_OBJECT (gki));

	g_signal_connect (gki, "size-changed",
			  G_CALLBACK (gkbd_status_size_changed), NULL);
	g_signal_connect (gki, "activate",
			  G_CALLBACK (gkbd_status_activate), NULL);

	for (i = sizeof (settings_signal_names) /
	     sizeof (settings_signal_names[0]); --i >= 0;)
		gki->priv->settings_signal_handlers[i] =
		    g_signal_connect_after (gtk_settings_get_default (),
					    settings_signal_names[i],
					    G_CALLBACK
					    (gkbd_status_theme_changed),
					    gki);

	xkl_debug (100,
		   "The status icon startup process for %p completed\n",
		   gki);
}

static void
gkbd_status_finalize (GObject * obj)
{
	int i;
	GkbdStatus *gki = GKBD_STATUS (obj);
	xkl_debug (100,
		   "Starting the gnome-kbd-status widget shutdown process for %p\n",
		   gki);

	for (i = sizeof (settings_signal_names) /
	     sizeof (settings_signal_names[0]); --i >= 0;)
		g_signal_handler_disconnect (gtk_settings_get_default (),
					     gki->
					     priv->settings_signal_handlers
					     [i]);

	/* remove BEFORE all termination work is finished */
	gkbd_configuration_remove_object (globals.config, G_OBJECT (gki));

	gkbd_status_cleanup_icons ();

	xkl_debug (100,
		   "The instance of gnome-kbd-status successfully finalized\n");

	g_free (gki->priv);

	G_OBJECT_CLASS (gkbd_status_parent_class)->finalize (obj);

	if (!gkbd_configuration_if_any_object_exists (globals.config))
		gkbd_status_global_term ();
}

static void
gkbd_status_global_term (void)
{
	xkl_debug (100, "*** Last  GkbdStatus instance *** \n");
	gkbd_status_stop_listen ();

	g_object_unref (globals.config);
	globals.config = NULL;

	xkl_debug (100, "*** Terminated globals *** \n");
}

static void
gkbd_status_class_init (GkbdStatusClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	xkl_debug (100, "*** First GkbdStatus instance *** \n");

	memset (&globals, 0, sizeof (globals));

	/* Initing vtable */
	object_class->finalize = gkbd_status_finalize;
}

static void
gkbd_status_global_init (void)
{
	globals.config = gkbd_configuration_get ();

	g_signal_connect (globals.config, "group-changed",
			  G_CALLBACK (gkbd_status_state_callback), NULL);
	g_signal_connect (globals.config, "changed",
			  G_CALLBACK (gkbd_status_cfg_callback), NULL);

	gkbd_status_start_listen ();

	xkl_debug (100, "*** Inited globals *** \n");
}

GtkStatusIcon *
gkbd_status_new (void)
{
	return
	    GTK_STATUS_ICON (g_object_new (gkbd_status_get_type (), NULL));
}

/**
 * gkbd_status_get_xkl_engine:
 *
 * Returns: (transfer none): The engine shared by all GkbdStatus objects
 */
XklEngine *
gkbd_status_get_xkl_engine ()
{
	return gkbd_configuration_get_xkl_engine (globals.config);
}

/**
 * gkbd_status_get_group_names:
 *
 * Returns: (transfer none) (array zero-terminated=1): List of group names
 */
gchar **
gkbd_status_get_group_names ()
{
	return (gchar **)
	    gkbd_configuration_get_group_names (globals.config);
}

gchar *
gkbd_status_get_image_filename (guint group)
{
	return gkbd_configuration_get_image_filename (globals.config,
						      group);
}
