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

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <glib/gi18n-lib.h>

#include <gkbd-indicator.h>
#include <gkbd-indicator-marshal.h>

#include <gkbd-desktop-config.h>
#include <gkbd-indicator-config.h>
#include <gkbd-configuration.h>

typedef struct _gki_globals {
	GkbdConfiguration *config;

	GSList *images;
} gki_globals;

struct _GkbdIndicatorPrivate {
	gboolean set_parent_tooltips;
	gdouble angle;
};

/* one instance for ALL widgets */
static gki_globals globals;

G_DEFINE_TYPE (GkbdIndicator, gkbd_indicator, GTK_TYPE_NOTEBOOK)

static void
gkbd_indicator_global_init (void);
static void
gkbd_indicator_global_term (void);
static GtkWidget *
gkbd_indicator_prepare_drawing (GkbdIndicator * gki, int group);
static void
gkbd_indicator_set_current_page_for_group (GkbdIndicator * gki, int group);
static void
gkbd_indicator_set_current_page (GkbdIndicator * gki);
static void
gkbd_indicator_cleanup (GkbdIndicator * gki);
static void
gkbd_indicator_fill (GkbdIndicator * gki);
static void
gkbd_indicator_set_tooltips (GkbdIndicator * gki, const char *str);

void
gkbd_indicator_set_tooltips (GkbdIndicator * gki, const char *str)
{
	g_assert (str == NULL || g_utf8_validate (str, -1, NULL));

	gtk_widget_set_tooltip_text (GTK_WIDGET (gki), str);

	if (gki->priv->set_parent_tooltips) {
		GtkWidget *parent =
		    gtk_widget_get_parent (GTK_WIDGET (gki));
		if (parent) {
			gtk_widget_set_tooltip_text (parent, str);
		}
	}
}

void
gkbd_indicator_cleanup (GkbdIndicator * gki)
{
	int i;
	GtkNotebook *notebook = GTK_NOTEBOOK (gki);

	/* Do not remove the first page! It is the default page */
	for (i = gtk_notebook_get_n_pages (notebook); --i > 0;) {
		gtk_notebook_remove_page (notebook, i);
	}
}

void
gkbd_indicator_fill (GkbdIndicator * gki)
{
	int grp;
	int total_groups =
	    xkl_engine_get_num_groups (gkbd_configuration_get_xkl_engine
				       (globals.config));
	GtkNotebook *notebook = GTK_NOTEBOOK (gki);
	gchar **full_group_names =
	    gkbd_configuration_get_group_names (globals.config);

	for (grp = 0; grp < total_groups; grp++) {
		GtkWidget *page = NULL;
		gchar *full_group_name =
		    (grp <
		     g_strv_length (full_group_names)) ?
		    full_group_names[grp] : "?";
		page = gkbd_indicator_prepare_drawing (gki, grp);

		if (page == NULL)
			page = gtk_label_new ("");

		gtk_notebook_append_page (notebook, page, NULL);
		gtk_widget_show_all (page);
	}
}

static gboolean
gkbd_indicator_key_pressed (GtkWidget *
			    widget, GdkEventKey * event,
			    GkbdIndicator * gki)
{
	switch (event->keyval) {
	case GDK_KEY_KP_Enter:
	case GDK_KEY_ISO_Enter:
	case GDK_KEY_3270_Enter:
	case GDK_KEY_Return:
	case GDK_KEY_space:
	case GDK_KEY_KP_Space:
		gkbd_configuration_lock_next_group (globals.config);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

static gboolean
gkbd_indicator_button_pressed (GtkWidget *
			       widget,
			       GdkEventButton * event, GkbdIndicator * gki)
{
	GtkWidget *img = gtk_bin_get_child (GTK_BIN (widget));
	GtkAllocation allocation;
	gtk_widget_get_allocation (img, &allocation);
	xkl_debug (150, "Flag img size %d x %d\n",
		   allocation.width, allocation.height);
	if (event->button == 1 && event->type == GDK_BUTTON_PRESS) {
		xkl_debug (150, "Mouse button pressed on applet\n");
		gkbd_configuration_lock_next_group (globals.config);
		return TRUE;
	}
	return FALSE;
}

static void
draw_flag (GtkWidget * flag, cairo_t * cr, GdkPixbuf * image)
{
	/* Image width and height */
	int iw = gdk_pixbuf_get_width (image);
	int ih = gdk_pixbuf_get_height (image);
	GtkAllocation allocation;
	double xwiratio, ywiratio, wiratio;

	gtk_widget_get_allocation (flag, &allocation);

	/* widget-to-image scales, X and Y */
	xwiratio = 1.0 * allocation.width / iw;
	ywiratio = 1.0 * allocation.height / ih;
	wiratio = xwiratio < ywiratio ? xwiratio : ywiratio;

	/* transform cairo context */
	cairo_translate (cr, allocation.width / 2.0,
			 allocation.height / 2.0);
	cairo_scale (cr, wiratio, wiratio);
	cairo_translate (cr, -iw / 2.0, -ih / 2.0);

	gdk_cairo_set_source_pixbuf (cr, image, 0, 0);
	cairo_paint (cr);
}

static GtkWidget *
gkbd_indicator_prepare_drawing (GkbdIndicator * gki, int group)
{
	gpointer pimage;
	GdkPixbuf *image;
	GtkWidget *ebox;

	pimage = g_slist_nth_data (globals.images, group);
	ebox = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (ebox), FALSE);
	if (gkbd_configuration_if_flags_shown (globals.config)) {
		GtkWidget *flag;
		if (pimage == NULL)
			return NULL;
		image = GDK_PIXBUF (pimage);
		flag = gtk_drawing_area_new ();
		gtk_widget_add_events (GTK_WIDGET (flag),
				       GDK_BUTTON_PRESS_MASK);
		g_signal_connect (G_OBJECT (flag), "draw",
				  G_CALLBACK (draw_flag), image);
		gtk_container_add (GTK_CONTAINER (ebox), flag);
	} else {
		char *lbl_title = NULL;
		char *layout_name = NULL;
		GtkWidget *align, *label;
		static GHashTable *ln2cnt_map = NULL;

		layout_name =
		    gkbd_configuration_extract_layout_name (globals.config,
							    group);

		lbl_title =
		    gkbd_configuration_create_label_title (group,
							   &ln2cnt_map,
							   layout_name);

		align = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
		label = gtk_label_new (lbl_title);
		g_free (lbl_title);
		gtk_label_set_angle (GTK_LABEL (label), gki->priv->angle);

		if (group + 1 ==
		    xkl_engine_get_num_groups
		    (gkbd_configuration_get_xkl_engine (globals.config))) {
			g_hash_table_destroy (ln2cnt_map);
			ln2cnt_map = NULL;
		}

		gtk_container_add (GTK_CONTAINER (align), label);
		gtk_container_add (GTK_CONTAINER (ebox), align);

		gtk_container_set_border_width (GTK_CONTAINER (align), 2);
	}

	g_signal_connect (G_OBJECT (ebox),
			  "button_press_event",
			  G_CALLBACK (gkbd_indicator_button_pressed), gki);

	g_signal_connect (G_OBJECT (gki),
			  "key_press_event",
			  G_CALLBACK (gkbd_indicator_key_pressed), gki);

	/* We have everything prepared for that size */

	return ebox;
}

static void
gkbd_indicator_update_tooltips (GkbdIndicator * gki)
{
	gchar *buf =
	    gkbd_configuration_get_current_tooltip (globals.config);
	if (buf != NULL) {
		gkbd_indicator_set_tooltips (gki, buf);
		g_free (buf);
	}
}

static void
gkbd_indicator_parent_set (GtkWidget * gki, GtkWidget * previous_parent)
{
	gkbd_indicator_update_tooltips (GKBD_INDICATOR (gki));
}


void
gkbd_indicator_reinit_ui (GkbdIndicator * gki)
{
	gkbd_indicator_cleanup (gki);
	gkbd_indicator_fill (gki);

	gkbd_indicator_set_current_page (gki);

	g_signal_emit_by_name (gki, "reinit-ui");
}


/* Should be called once for all widgets */
static void
gkbd_indicator_cfg_callback (GkbdConfiguration * configuration)
{
	ForAllObjects (configuration) {
		gkbd_indicator_reinit_ui (GKBD_INDICATOR (gki));
	} NextObject ()
}

/* Should be called once for all applets */
static void
gkbd_indicator_state_callback (GkbdConfiguration * configuration,
			       gint group)
{
	ForAllObjects (configuration) {
		xkl_debug (200, "do repaint\n");
		gkbd_indicator_set_current_page_for_group (GKBD_INDICATOR
							   (gki), group);
	}
	NextObject ()
}


void
gkbd_indicator_set_current_page (GkbdIndicator * gki)
{
	XklEngine *engine =
	    gkbd_configuration_get_xkl_engine (globals.config);
	XklState *cur_state = xkl_engine_get_current_state (engine);
	if (cur_state->group >= 0)
		gkbd_indicator_set_current_page_for_group (gki,
							   cur_state->
							   group);
}

void
gkbd_indicator_set_current_page_for_group (GkbdIndicator * gki, int group)
{
	xkl_debug (200, "Revalidating for group %d\n", group);

	gtk_notebook_set_current_page (GTK_NOTEBOOK (gki), group + 1);

	gkbd_indicator_update_tooltips (gki);
}

/* Should be called once for all widgets */
static GdkFilterReturn
gkbd_indicator_filter_x_evt (GdkXEvent * xev, GdkEvent * event)
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
				GdkWindow *w =
				    gtk_widget_get_parent_window
				    (GTK_WIDGET (gki));

				/* compare the indicator's parent window with the even window */
				if (w != NULL
				    && GDK_WINDOW_XID (w) == rne->window) {
					/* if so - make it transparent... */
					xkl_engine_set_window_transparent
					    (engine, rne->window, TRUE);
				}
			}
			NextObject ()
		}
		break;
	}
	return GDK_FILTER_CONTINUE;
}


/* Should be called once for all widgets */
static void
gkbd_indicator_start_listen (void)
{
	gdk_window_add_filter (NULL, (GdkFilterFunc)
			       gkbd_indicator_filter_x_evt, NULL);
	gdk_window_add_filter (gdk_get_default_root_window (),
			       (GdkFilterFunc)
			       gkbd_indicator_filter_x_evt, NULL);
}

/* Should be called once for all widgets */
static void
gkbd_indicator_stop_listen (void)
{
	gdk_window_remove_filter (NULL, (GdkFilterFunc)
				  gkbd_indicator_filter_x_evt, NULL);
	gdk_window_remove_filter
	    (gdk_get_default_root_window (),
	     (GdkFilterFunc) gkbd_indicator_filter_x_evt, NULL);
}

static gboolean
gkbd_indicator_scroll (GtkWidget * gki, GdkEventScroll * event)
{
	/* mouse wheel events should be ignored, otherwise funny effects appear */
	return TRUE;
}

static void
gkbd_indicator_init (GkbdIndicator * gki)
{
	GtkWidget *def_drawing;
	GtkNotebook *notebook;

	if (!gkbd_configuration_if_any_object_exists (globals.config))
		gkbd_indicator_global_init ();

	gki->priv = g_new0 (GkbdIndicatorPrivate, 1);

	notebook = GTK_NOTEBOOK (gki);

	xkl_debug (100, "Initiating the widget startup process for %p\n",
		   gki);

	gtk_notebook_set_show_tabs (notebook, FALSE);
	gtk_notebook_set_show_border (notebook, FALSE);

	def_drawing =
	    gtk_image_new_from_stock (GTK_STOCK_STOP,
				      GTK_ICON_SIZE_BUTTON);

	gtk_notebook_append_page (notebook, def_drawing,
				  gtk_label_new (""));

	if (gkbd_configuration_get_xkl_engine (globals.config) == NULL) {
		gkbd_indicator_set_tooltips (gki,
					     _
					     ("XKB initialization error"));
		return;
	}

	gkbd_indicator_set_tooltips (gki, NULL);

	gkbd_indicator_fill (gki);
	gkbd_indicator_set_current_page (gki);

	gtk_widget_add_events (GTK_WIDGET (gki), GDK_BUTTON_PRESS_MASK);

	/* append AFTER all initialization work is finished */
	gkbd_configuration_append_object (globals.config, G_OBJECT (gki));
}

static void
gkbd_indicator_finalize (GObject * obj)
{
	GkbdIndicator *gki = GKBD_INDICATOR (obj);
	xkl_debug (100,
		   "Starting the gnome-kbd-indicator widget shutdown process for %p\n",
		   gki);

	/* remove BEFORE all termination work is finished */
	gkbd_configuration_remove_object (globals.config, G_OBJECT (gki));

	gkbd_indicator_cleanup (gki);

	xkl_debug (100,
		   "The instance of gnome-kbd-indicator successfully finalized\n");

	g_free (gki->priv);

	G_OBJECT_CLASS (gkbd_indicator_parent_class)->finalize (obj);

	if (!gkbd_configuration_if_any_object_exists (globals.config))
		gkbd_indicator_global_term ();
}

static void
gkbd_indicator_global_term (void)
{
	xkl_debug (100, "*** Last  GkbdIndicator instance *** \n");

	gkbd_configuration_free_images (globals.config, globals.images);
	globals.images = NULL;

	gkbd_indicator_stop_listen ();
	g_object_unref (globals.config);
	globals.config = NULL;

	xkl_debug (100, "*** Terminated globals *** \n");
}

static void
gkbd_indicator_class_init (GkbdIndicatorClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	xkl_debug (100, "*** First GkbdIndicator instance *** \n");

	memset (&globals, 0, sizeof (globals));

	/* Initing vtable */
	object_class->finalize = gkbd_indicator_finalize;

	widget_class->scroll_event = gkbd_indicator_scroll;
	widget_class->parent_set = gkbd_indicator_parent_set;

	/* Signals */
	g_signal_new ("reinit-ui", GKBD_TYPE_INDICATOR,
		      G_SIGNAL_RUN_LAST,
		      G_STRUCT_OFFSET (GkbdIndicatorClass, reinit_ui),
		      NULL, NULL, gkbd_indicator_VOID__VOID,
		      G_TYPE_NONE, 0);
}

static void
gkbd_indicator_global_init (void)
{
	globals.config = gkbd_configuration_get ();

	g_signal_connect (globals.config, "group-changed",
			  G_CALLBACK (gkbd_indicator_state_callback),
			  NULL);
	g_signal_connect (globals.config, "changed",
			  G_CALLBACK (gkbd_indicator_cfg_callback), NULL);

	globals.images = gkbd_configuration_load_images (globals.config);

	gkbd_indicator_start_listen ();

	xkl_debug (100, "*** Inited globals *** \n");
}

GtkWidget *
gkbd_indicator_new (void)
{
	return
	    GTK_WIDGET (g_object_new (gkbd_indicator_get_type (), NULL));
}

void
gkbd_indicator_set_parent_tooltips (GkbdIndicator * gki, gboolean spt)
{
	gki->priv->set_parent_tooltips = spt;
	gkbd_indicator_update_tooltips (gki);
}

/**
 * gkbd_indicator_get_xkl_engine:
 *
 * Returns: (transfer none): The engine shared by all GkbdIndicator objects
 */
XklEngine *
gkbd_indicator_get_xkl_engine ()
{
	return gkbd_configuration_get_xkl_engine (globals.config);
}

/**
 * gkbd_indicator_get_group_names:
 *
 * Returns: (transfer none) (array zero-terminated=1): List of group names
 */
gchar **
gkbd_indicator_get_group_names ()
{
	return (gchar **)
	    gkbd_configuration_get_group_names (globals.config);
}

gchar *
gkbd_indicator_get_image_filename (guint group)
{
	return gkbd_configuration_get_image_filename (globals.config,
						      group);
}

gdouble
gkbd_indicator_get_max_width_height_ratio (void)
{
	gdouble rv = 0.0;
	GSList *ip = globals.images;
	if (!gkbd_configuration_if_flags_shown (globals.config))
		return 0;
	while (ip != NULL) {
		GdkPixbuf *img = GDK_PIXBUF (ip->data);
		gdouble r =
		    1.0 * gdk_pixbuf_get_width (img) /
		    gdk_pixbuf_get_height (img);
		if (r > rv)
			rv = r;
		ip = ip->next;
	}
	return rv;
}

void
gkbd_indicator_set_angle (GkbdIndicator * gki, gdouble angle)
{
	gki->priv->angle = angle;
}

