#include <memory.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>

#include <gkb-indicator.h>
#include <gkb-indicator-marshal.h>

#include <gkb-desktop-config.h>
#include <gkb-indicator-config.h>

#include <gkb-indicator-plugin-manager.h>

#include <gkb-config-registry-client.h>

typedef struct _gki_globals {
	XklEngine *engine;

	GkbDesktopConfig cfg;
	GkbIndicatorConfig ind_cfg;
	GkbKeyboardConfig kbd_cfg;

	GkbIndicatorPluginContainer plugin_container;
	GkbIndicatorPluginManager plugin_manager;

	const gchar *tooltips_format;
	gchar **full_group_names;
	gchar **short_group_names;
	GSList *widget_instances;
} gki_globals;

struct _GkbIndicatorPrivate {
	gboolean set_parent_tooltips;
	gdouble angle;
};

/* one instance for ALL widgets */
static gki_globals globals;

#define ForAllIndicators() \
	{ \
		GSList* cur; \
		for (cur = globals.widget_instances; cur != NULL; cur = cur->next) { \
			GkbIndicator * gki = (GkbIndicator*)cur->data;
#define NextIndicator() \
		} \
	}

G_DEFINE_TYPE (GkbIndicator, gkb_indicator, GTK_TYPE_NOTEBOOK)

static void
gkb_indicator_global_init (void);
static void
gkb_indicator_global_term (void);
static GtkWidget *
gkb_indicator_prepare_drawing (GkbIndicator * gki, int group);
static void
gkb_indicator_set_current_page_for_group (GkbIndicator * gki, int group);
static void
gkb_indicator_set_current_page (GkbIndicator * gki);
static void
gkb_indicator_cleanup (GkbIndicator * gki);
static void
gkb_indicator_fill (GkbIndicator * gki);
static void
gkb_indicator_set_tooltips (GkbIndicator * gki, const char *str);

void
gkb_indicator_set_tooltips (GkbIndicator * gki, const char *str)
{
	GtkTooltips *tooltips;

	if (str == NULL)
		return;
	tooltips = gtk_tooltips_new ();
	g_object_ref (G_OBJECT (tooltips));
	gtk_object_sink (GTK_OBJECT (tooltips));
	g_object_set_data_full (G_OBJECT (gki), "tooltips",
				tooltips, (GDestroyNotify) g_object_unref);
	gtk_tooltips_set_tip (tooltips, GTK_WIDGET (gki), str, NULL);

	if (gki->priv->set_parent_tooltips) {
		GtkWidget *parent =
		    gtk_widget_get_parent (GTK_WIDGET (gki));
		if (parent != NULL) {
			gtk_tooltips_set_tip (tooltips,
					      GTK_WIDGET (parent), str,
					      NULL);
			g_object_ref (G_OBJECT (tooltips));
			g_object_set_data_full (G_OBJECT (parent),
						"gnome-kbd-indicator.tooltips",
						tooltips, (GDestroyNotify)
						g_object_unref);
		}
	}
	gtk_tooltips_enable (tooltips);
}

void
gkb_indicator_cleanup (GkbIndicator * gki)
{
	int i;
	GtkNotebook *notebook = GTK_NOTEBOOK (gki);

	/* Do not remove the first page! It is the default page */
	for (i = gtk_notebook_get_n_pages (notebook); --i > 0;) {
		gtk_notebook_remove_page (notebook, i);
	}
}

void
gkb_indicator_fill (GkbIndicator * gki)
{
	int grp;
	int total_groups = xkl_engine_get_num_groups (globals.engine);
	GtkNotebook *notebook = GTK_NOTEBOOK (gki);

	for (grp = 0; grp < total_groups; grp++) {
		GtkWidget *page, *decorated_page;
		page = gkb_indicator_prepare_drawing (gki, grp);

		if (page == NULL)
			page = gtk_label_new ("");

		decorated_page =
		    gkb_indicator_plugin_manager_decorate_widget (&globals.
							      plugin_manager,
							      page, grp,
							      globals.
							      full_group_names
							      [grp],
							      &globals.
							      kbd_cfg);

		page = decorated_page == NULL ? page : decorated_page;

		gtk_notebook_append_page (notebook, page, NULL);
		gtk_widget_show_all (page);
	}
}

static gboolean
gkb_indicator_key_pressed (GtkWidget *
			   widget, GdkEventKey * event, GkbIndicator * gki)
{
	switch (event->keyval) {
	case GDK_KP_Enter:
	case GDK_ISO_Enter:
	case GDK_3270_Enter:
	case GDK_Return:
	case GDK_space:
	case GDK_KP_Space:
		gkb_desktop_config_lock_next_group (&globals.cfg);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

static gboolean
gkb_indicator_button_pressed (GtkWidget *
			      widget,
			      GdkEventButton * event, GkbIndicator * gki)
{
	GtkWidget *img = gtk_bin_get_child (GTK_BIN (widget));
	xkl_debug (150, "Flag img size %d x %d\n",
		   img->allocation.width, img->allocation.height);
	if (event->button == 1 && event->type == GDK_BUTTON_PRESS) {
		xkl_debug (150, "Mouse button pressed on applet\n");
		gkb_desktop_config_lock_next_group (&globals.cfg);
		return TRUE;
	}
	return FALSE;
}

static void
flag_exposed (GtkWidget * flag, GdkEventExpose * event, GdkPixbuf * image)
{
	/* Image width and height */
	int iw = gdk_pixbuf_get_width (image);
	int ih = gdk_pixbuf_get_height (image);
	/* widget-to-image scales, X and Y */
	double xwiratio = 1.0 * flag->allocation.width / iw;
	double ywiratio = 1.0 * flag->allocation.height / ih;
	double wiratio = xwiratio < ywiratio ? xwiratio : ywiratio;

	/* scaled width and height */
	int sw = iw * wiratio;
	int sh = ih * wiratio;

	/* offsets */
	int ox = (flag->allocation.width - sw) >> 1;
	int oy = (flag->allocation.height - sh) >> 1;

	GdkPixbuf *scaled = gdk_pixbuf_scale_simple (image, sw, sh,
						     GDK_INTERP_HYPER);

	gdk_draw_pixbuf (GDK_DRAWABLE (flag->window),
			 NULL,
			 scaled,
			 0, 0,
			 ox, oy, sw, sh, GDK_RGB_DITHER_NORMAL, 0, 0);
	g_object_unref (G_OBJECT (scaled));
}

static GtkWidget *
gkb_indicator_prepare_drawing (GkbIndicator * gki, int group)
{
	gpointer pimage;
	GdkPixbuf *image;
	GtkWidget *ebox;

	pimage = g_slist_nth_data (globals.ind_cfg.images, group);
	ebox = gtk_event_box_new ();
	if (globals.ind_cfg.show_flags) {
		GtkWidget *flag;
		if (pimage == NULL)
			return NULL;
		image = GDK_PIXBUF (pimage);
		flag = gtk_drawing_area_new ();
		g_signal_connect (G_OBJECT (flag),
				  "expose_event",
				  G_CALLBACK (flag_exposed), image);
		gtk_container_add (GTK_CONTAINER (ebox), flag);
	} else {
		gpointer pcounter = NULL;
		char *prev_layout_name = NULL, **ppln;
		char *lbl_title = NULL;
		int counter = 0;
		char *layout_name = NULL;
		XklConfigItem cfg_item;
		GtkWidget *align, *label;
		/**
		 * Map "short desciption" -> 
		 * number of layouts in the configuration 
		 * having this short description
		 */
		static GHashTable *short_descrs = NULL;

		if (group == 0)
			short_descrs =
			    g_hash_table_new_full (g_str_hash, g_str_equal,
						   g_free, NULL);

		if (xkl_engine_get_features (globals.engine) &
		    XKLF_MULTIPLE_LAYOUTS_SUPPORTED) {
			char *full_layout_name =
			    (char *) g_slist_nth_data (globals.kbd_cfg.
						       layouts,
						       group);
			char *variant_name;
			if (!gkb_keyboard_config_split_items
			    (full_layout_name, &layout_name,
			     &variant_name))
				/* just in case */
				layout_name = g_strdup (full_layout_name);

			g_snprintf (cfg_item.name,
				    sizeof (cfg_item.name), "%s",
				    layout_name);

			if (globals.short_group_names != NULL) {
				char *short_group_name =
				    globals.short_group_names[group];
				if (short_group_name != NULL
				    && *short_group_name != '\0') {
					layout_name =
					    g_strdup (short_group_name);
				}
			}
		} else
			layout_name =
			    g_strdup (globals.full_group_names[group]);

		if (layout_name == NULL)
			layout_name = g_strdup ("?");

		/* Process layouts with repeating description */
		ppln = &prev_layout_name;
		if (g_hash_table_lookup_extended
		    (short_descrs, layout_name,
		     (gpointer *) ppln, &pcounter)) {
			/* "next" same description */
			gchar appendix[10] = "";
			gint utf8length;
			gunichar cidx;
			counter = GPOINTER_TO_INT (pcounter);
			/* Unicode subscript 2, 3, 4 */
			cidx = 0x2081 + counter;
			utf8length = g_unichar_to_utf8 (cidx, appendix);
			appendix[utf8length] = '\0';
			lbl_title =
			    g_strconcat (layout_name, appendix, NULL);
		} else {
			/* "first" time this description */
			lbl_title = g_strdup (layout_name);
		}
		g_hash_table_insert (short_descrs, layout_name,
				     GINT_TO_POINTER (counter + 1));

		align = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
		label = gtk_label_new (lbl_title);
		g_free (lbl_title);
		gtk_label_set_angle (GTK_LABEL (label), gki->priv->angle);

		if (group == xkl_engine_get_num_groups (globals.engine)) {
			g_hash_table_destroy (short_descrs);
			short_descrs = NULL;
		}

		gtk_container_add (GTK_CONTAINER (align), label);
		gtk_container_add (GTK_CONTAINER (ebox), align);

		gtk_container_set_border_width (GTK_CONTAINER (align), 2);
	}
	g_signal_connect (G_OBJECT (ebox),
			  "button_press_event",
			  G_CALLBACK (gkb_indicator_button_pressed), gki);

	g_signal_connect (G_OBJECT (gki),
			  "key_press_event",
			  G_CALLBACK (gkb_indicator_key_pressed), gki);

	/* We have everything prepared for that size */

	return ebox;
}

static void
gkb_indicator_update_tooltips (GkbIndicator * gki)
{
	XklState *state = xkl_engine_get_current_state (globals.engine);
	gchar *buf;
	if (state == NULL || state->group < 0)
		return;

	buf = g_strdup_printf (globals.tooltips_format,
			       globals.full_group_names[state->group]);

	gkb_indicator_set_tooltips (gki, buf);
	g_free (buf);
}

static void
gkb_indicator_parent_set (GtkWidget * gki, GtkWidget * previous_parent)
{
	gkb_indicator_update_tooltips (GKB_INDICATOR (gki));
}


void
gkb_indicator_reinit_ui (GkbIndicator * gki)
{
	gkb_indicator_cleanup (gki);
	gkb_indicator_fill (gki);

	gkb_indicator_set_current_page (gki);

	g_signal_emit_by_name (gki, "reinit-ui");
}

/* Should be called once for all widgets */
static void
gkb_indicator_cfg_changed (GConfClient * client,
			   guint cnxn_id, GConfEntry * entry)
{
	xkl_debug (100,
		   "General configuration changed in GConf - reiniting...\n");
	gkb_desktop_config_load_from_gconf (&globals.cfg);
	gkb_desktop_config_activate (&globals.cfg);
	ForAllIndicators () {
		gkb_indicator_reinit_ui (gki);
	} NextIndicator ();
}

/* Should be called once for all widgets */
static void
gkb_indicator_ind_cfg_changed (GConfClient * client,
			       guint cnxn_id, GConfEntry * entry)
{
	xkl_debug (100,
		   "Applet configuration changed in GConf - reiniting...\n");
	gkb_indicator_config_load_from_gconf (&globals.ind_cfg);
	gkb_indicator_config_update_images (&globals.ind_cfg,
					    &globals.kbd_cfg);
	gkb_indicator_config_activate (&globals.ind_cfg);

	gkb_indicator_plugin_manager_toggle_plugins (&globals.plugin_manager,
						 &globals.plugin_container,
						 globals.ind_cfg.
						 enabled_plugins);

	ForAllIndicators () {
		gkb_indicator_reinit_ui (gki);
	} NextIndicator ();
}

static void
gkb_indicator_load_group_names (void)
{
	if (!gkb_desktop_config_load_remote_group_descriptions_utf8
	    (&globals.cfg, &globals.short_group_names,
	     &globals.full_group_names)) {
		gint i, total_groups =
		    xkl_engine_get_num_groups (globals.engine);
		globals.full_group_names =
		    g_new0 (char *, total_groups + 1);

		if (xkl_engine_get_features (globals.engine) &
		    XKLF_MULTIPLE_LAYOUTS_SUPPORTED) {
			GSList *lst = globals.kbd_cfg.layouts;
			for (i = 0; lst; lst = lst->next) {
				globals.full_group_names[i++] =
				    g_strdup ((char *) lst->data);
			}
		} else {
			for (i = total_groups; --i >= 0;) {
				globals.full_group_names[i] =
				    g_strdup_printf ("Group %d", i);
			}
		}
	}
}

/* Should be called once for all widgets */
static void
gkb_indicator_kbd_cfg_callback (GkbIndicator * gki)
{
	xkl_debug (100,
		   "XKB configuration changed on X Server - reiniting...\n");

	gkb_keyboard_config_load_from_x_current (&globals.kbd_cfg);
	gkb_indicator_config_update_images (&globals.ind_cfg,
					    &globals.kbd_cfg);

	g_strfreev (globals.full_group_names);
	g_strfreev (globals.short_group_names);
	gkb_indicator_load_group_names ();

	ForAllIndicators () {
		gkb_indicator_reinit_ui (gki);
	} NextIndicator ();
}

/* Should be called once for all applets */
static void
gkb_indicator_state_callback (XklEngine * engine,
			      XklEngineStateChange changeType,
			      gint group, gboolean restore)
{
	xkl_debug (150, "group is now %d, restore: %d\n", group, restore);

	if (changeType == GROUP_CHANGED) {
		ForAllIndicators () {
			gkb_indicator_plugin_manager_group_changed (&globals.
								plugin_manager,
								GTK_WIDGET
								(gki),
								group);
			xkl_debug (200, "do repaint\n");
			gkb_indicator_set_current_page_for_group
			    (gki, group);
		}
		NextIndicator ();
	}
}


void
gkb_indicator_set_current_page (GkbIndicator * gki)
{
	XklState *cur_state;
	cur_state = xkl_engine_get_current_state (globals.engine);
	if (cur_state->group >= 0)
		gkb_indicator_set_current_page_for_group (gki,
							  cur_state->
							  group);
}

void
gkb_indicator_set_current_page_for_group (GkbIndicator * gki, int group)
{
	xkl_debug (200, "Revalidating for group %d\n", group);

	gtk_notebook_set_current_page (GTK_NOTEBOOK (gki), group + 1);

	gkb_indicator_update_tooltips (gki);
}

/* Should be called once for all widgets */
static GdkFilterReturn
gkb_indicator_filter_x_evt (GdkXEvent * xev, GdkEvent * event)
{
	XEvent *xevent = (XEvent *) xev;

	xkl_engine_filter_events (globals.engine, xevent);
	switch (xevent->type) {
	case ReparentNotify:
		{
			XReparentEvent *rne = (XReparentEvent *) xev;

			ForAllIndicators () {
				GdkWindow *w =
				    gtk_widget_get_parent_window
				    (GTK_WIDGET (gki));

				/* compare the indicator's parent window with the even window */
				if (w != NULL
				    && GDK_WINDOW_XID (w) == rne->window) {
					/* if so - make it transparent... */
					xkl_engine_set_window_transparent
					    (globals.engine, rne->window,
					     TRUE);
				}
			}
			NextIndicator ()
		}
		break;
	}
	return GDK_FILTER_CONTINUE;
}


/* Should be called once for all widgets */
static void
gkb_indicator_start_listen (void)
{
	gdk_window_add_filter (NULL, (GdkFilterFunc)
			       gkb_indicator_filter_x_evt, NULL);
	gdk_window_add_filter (gdk_get_default_root_window (),
			       (GdkFilterFunc)
			       gkb_indicator_filter_x_evt, NULL);

	xkl_engine_start_listen (globals.engine,
				 XKLL_TRACK_KEYBOARD_STATE);
}

/* Should be called once for all widgets */
static void
gkb_indicator_stop_listen (void)
{
	xkl_engine_stop_listen (globals.engine);

	gdk_window_remove_filter (NULL, (GdkFilterFunc)
				  gkb_indicator_filter_x_evt, NULL);
	gdk_window_remove_filter
	    (gdk_get_default_root_window (),
	     (GdkFilterFunc) gkb_indicator_filter_x_evt, NULL);
}

static gboolean
gkb_indicator_scroll (GtkWidget * gki, GdkEventScroll * event)
{
	/* mouse wheel events should be ignored, otherwise funny effects appear */
	return TRUE;
}

static void
gkb_indicator_init (GkbIndicator * gki)
{
	GtkWidget *def_drawing;
	GtkNotebook *notebook;

	if (!g_slist_length (globals.widget_instances))
		gkb_indicator_global_init ();

	gki->priv = g_new0 (GkbIndicatorPrivate, 1);

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

	if (globals.engine == NULL) {
		gkb_indicator_set_tooltips (gki,
					    _("XKB initialization error"));
		return;
	}

	gkb_indicator_set_tooltips (gki, "");

	gkb_indicator_fill (gki);
	gkb_indicator_set_current_page (gki);

	gtk_widget_add_events (GTK_WIDGET (gki), GDK_BUTTON_PRESS_MASK);

	/* append AFTER all initialization work is finished */
	globals.widget_instances =
	    g_slist_append (globals.widget_instances, gki);
}

static void
gkb_indicator_finalize (GObject * obj)
{
	GkbIndicator *gki = GKB_INDICATOR (obj);
	xkl_debug (100,
		   "Starting the gnome-kbd-indicator widget shutdown process for %p\n",
		   gki);

	/* remove BEFORE all termination work is finished */
	globals.widget_instances =
	    g_slist_remove (globals.widget_instances, gki);

	gkb_indicator_cleanup (gki);

	xkl_debug (100,
		   "The instance of gnome-kbd-indicator successfully finalized\n");

	g_free (gki->priv);

	G_OBJECT_CLASS (gkb_indicator_parent_class)->finalize (obj);

	if (!g_slist_length (globals.widget_instances))
		gkb_indicator_global_term ();
}

static void
gkb_indicator_global_term (void)
{
	xkl_debug (100, "*** Last  GkbIndicator instance *** \n");
	gkb_indicator_stop_listen ();

	gkb_desktop_config_stop_listen (&globals.cfg);
	gkb_indicator_config_stop_listen (&globals.ind_cfg);

	gkb_indicator_plugin_manager_term_initialized_plugins (&globals.
							   plugin_manager);
	gkb_indicator_plugin_manager_term (&globals.plugin_manager);

	gkb_indicator_config_term (&globals.ind_cfg);
	gkb_keyboard_config_term (&globals.kbd_cfg);
	gkb_desktop_config_term (&globals.cfg);

	gkb_indicator_plugin_container_term (&globals.plugin_container);

	g_object_unref (G_OBJECT (globals.engine));
	globals.engine = NULL;
	xkl_debug (100, "*** Terminated globals *** \n");
}

static void
gkb_indicator_class_init (GkbIndicatorClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	xkl_debug (100, "*** First GkbIndicator instance *** \n");

	memset (&globals, 0, sizeof (globals));

	/* Initing some global vars */
	globals.tooltips_format = "%s";

	/* Initing vtable */
	object_class->finalize = gkb_indicator_finalize;

	widget_class->scroll_event = gkb_indicator_scroll;
	widget_class->parent_set = gkb_indicator_parent_set;

	/* Signals */
	g_signal_new ("reinit-ui", GKB_TYPE_INDICATOR,
		      G_SIGNAL_RUN_LAST,
		      G_STRUCT_OFFSET (GkbIndicatorClass, reinit_ui),
		      NULL, NULL, gkb_indicator_VOID__VOID,
		      G_TYPE_NONE, 0);
}

static void
gkb_indicator_global_init (void)
{
	GConfClient *gconf_client;

	globals.engine = xkl_engine_get_instance (GDK_DISPLAY ());
	if (globals.engine == NULL) {
		xkl_debug (0, "Libxklavier initialization error");
		return;
	}

	gconf_client = gconf_client_get_default ();

	g_signal_connect (globals.engine, "X-state-changed",
			  G_CALLBACK (gkb_indicator_state_callback), NULL);
	g_signal_connect (globals.engine, "X-config-changed",
			  G_CALLBACK
			  (gkb_indicator_kbd_cfg_callback), NULL);

	gkb_indicator_plugin_container_init (&globals.plugin_container,
					 gconf_client);

	gkb_desktop_config_init (&globals.cfg, gconf_client,
				 globals.engine);
	gkb_keyboard_config_init (&globals.kbd_cfg, gconf_client,
				  globals.engine);
	gkb_indicator_config_init (&globals.ind_cfg, gconf_client,
				   globals.engine);

	g_object_unref (gconf_client);

	gkb_desktop_config_load_from_gconf (&globals.cfg);
	gkb_desktop_config_activate (&globals.cfg);
	gkb_keyboard_config_load_from_x_current (&globals.kbd_cfg);
	gkb_indicator_config_load_from_gconf (&globals.ind_cfg);
	gkb_indicator_config_update_images (&globals.ind_cfg,
					    &globals.kbd_cfg);
	gkb_indicator_config_activate (&globals.ind_cfg);

	gkb_indicator_load_group_names ();

	gkb_indicator_plugin_manager_init (&globals.plugin_manager);
	gkb_indicator_plugin_manager_init_enabled_plugins (&globals.
						       plugin_manager,
						       &globals.
						       plugin_container,
						       globals.
						       ind_cfg.
						       enabled_plugins);
	gkb_desktop_config_start_listen (&globals.cfg,
					 (GConfClientNotifyFunc)
					 gkb_indicator_cfg_changed, NULL);
	gkb_indicator_config_start_listen (&globals.ind_cfg,
					   (GConfClientNotifyFunc)
					   gkb_indicator_ind_cfg_changed,
					   NULL);
	gkb_indicator_start_listen ();

	xkl_debug (100, "*** Inited globals *** \n");
}

GtkWidget *
gkb_indicator_new (void)
{
	return GTK_WIDGET (g_object_new (gkb_indicator_get_type (), NULL));
}

void
gkb_indicator_set_parent_tooltips (GkbIndicator * gki, gboolean spt)
{
	gki->priv->set_parent_tooltips = spt;
	gkb_indicator_update_tooltips (gki);
}

void
gkb_indicator_set_tooltips_format (const gchar format[])
{
	globals.tooltips_format = format;
	ForAllIndicators ()
	    gkb_indicator_update_tooltips (gki);
	NextIndicator ()
}

XklEngine *
gkb_indicator_get_xkl_engine ()
{
	return globals.engine;
}

gchar **
gkb_indicator_get_group_names ()
{
	return globals.full_group_names;
}

gchar *
gkb_indicator_get_image_filename (guint group)
{
	if (!globals.ind_cfg.show_flags)
		return NULL;
	return gkb_indicator_config_get_images_file (&globals.
						     ind_cfg,
						     &globals.
						     kbd_cfg, group);
}

gdouble
gkb_indicator_get_max_width_height_ratio (void)
{
	gdouble rv = 0.0;
	GSList *ip = globals.ind_cfg.images;
	if (!globals.ind_cfg.show_flags)
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
gkb_indicator_set_angle (GkbIndicator * gki, gdouble angle)
{
	gki->priv->angle = angle;
}

/* Plugin support */
/* Preserve the plugin container functions during the linking */
void
gkb_indicator_plugin_container_reinit_ui (GkbIndicatorPluginContainer * pc)
{
	ForAllIndicators () {
		gkb_indicator_reinit_ui (gki);
	} NextIndicator ();
}

gchar **
gkb_indicator_plugin_load_localized_group_names (GkbIndicatorPluginContainer *
					     pc)
{
	return globals.full_group_names;
}
