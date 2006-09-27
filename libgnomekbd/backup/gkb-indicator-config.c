/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/keysym.h>

#include <glib/gi18n.h>
#include <gdk/gdkx.h>
#include <libgnome/gnome-program.h>

#include <gkb-keyboard-config.h>
#include <gkb-indicator-config.h>

#include <gkb-config-private.h>

#include <gkb-config-registry-client.h>

/**
 * GkbIndicatorConfig
 */
#define GKB_INDICATOR_CONFIG_KEY_PREFIX  GKB_CONFIG_KEY_PREFIX "/indicator"

const gchar GKB_INDICATOR_CONFIG_DIR[] = GKB_INDICATOR_CONFIG_KEY_PREFIX;
const gchar GKB_INDICATOR_CONFIG_KEY_SHOW_FLAGS[] =
    GKB_INDICATOR_CONFIG_KEY_PREFIX "/showFlags";
const gchar GKB_INDICATOR_CONFIG_KEY_ENABLED_PLUGINS[] =
    GKB_INDICATOR_CONFIG_KEY_PREFIX "/enabledPlugins";
const gchar GKB_INDICATOR_CONFIG_KEY_SECONDARIES[] =
    GKB_INDICATOR_CONFIG_KEY_PREFIX "/secondary";

/**
 * static applet config functions
 */
static void
gkb_indicator_config_free_enabled_plugins (GkbIndicatorConfig * ind_config)
{
	GSList *plugin_node = ind_config->enabled_plugins;
	if (plugin_node != NULL) {
		do {
			if (plugin_node->data != NULL) {
				g_free (plugin_node->data);
				plugin_node->data = NULL;
			}
			plugin_node = g_slist_next (plugin_node);
		} while (plugin_node != NULL);
		g_slist_free (ind_config->enabled_plugins);
		ind_config->enabled_plugins = NULL;
	}
}

/**
 * extern applet kbdConfig functions
 */
void
gkb_indicator_config_free_images (GkbIndicatorConfig * ind_config)
{
	GdkPixbuf *pi;
	GSList *img_node;
	while ((img_node = ind_config->images) != NULL) {
		pi = GDK_PIXBUF (img_node->data);
		/* It can be NULL - some images may be missing */
		if (pi != NULL) {
			gdk_pixbuf_unref (pi);
		}
		ind_config->images =
		    g_slist_remove_link (ind_config->images, img_node);
		g_slist_free_1 (img_node);
	}
}

char *
gkb_indicator_config_get_images_file (GkbIndicatorConfig *
				      ind_config,
				      GkbKeyboardConfig *
				      kbd_config, int group)
{
	char *image_file = NULL;
	GtkIconInfo *icon_info = NULL;

	if (!ind_config->show_flags)
		return NULL;

	if ((kbd_config->layouts != NULL) &&
	    (g_slist_length (kbd_config->layouts) > group)) {
		char *full_layout_name =
		    (char *) g_slist_nth_data (kbd_config->layouts, group);

		if (full_layout_name != NULL) {
			char *l, *v;
			gkb_keyboard_config_split_items (full_layout_name,
							 &l, &v);
			if (l != NULL) {
				// probably there is something in theme?
				icon_info = gtk_icon_theme_lookup_icon
				    (ind_config->icon_theme, l, 48, 0);
			}
		}
	}
	// fallback to the default value
	if (icon_info == NULL) {
		icon_info = gtk_icon_theme_lookup_icon
		    (ind_config->icon_theme, "stock_dialog-error", 48, 0);
	}
	if (icon_info != NULL) {
		image_file =
		    g_strdup (gtk_icon_info_get_filename (icon_info));
		gtk_icon_info_free (icon_info);
	}

	return image_file;
}

void
gkb_indicator_config_load_images (GkbIndicatorConfig * ind_config,
				  GkbKeyboardConfig * kbd_config)
{
	int i;
	ind_config->images = NULL;

	if (!ind_config->show_flags)
		return;

	for (i = xkl_engine_get_max_num_groups (ind_config->engine);
	     --i >= 0;) {
		GdkPixbuf *image = NULL;
		char *image_file =
		    gkb_indicator_config_get_images_file (ind_config,
							  kbd_config,
							  i);

		if (image_file != NULL) {
			GError *gerror = NULL;
			image =
			    gdk_pixbuf_new_from_file (image_file, &gerror);
			if (image == NULL) {
				GtkWidget *dialog =
				    gtk_message_dialog_new (NULL,
							    GTK_DIALOG_DESTROY_WITH_PARENT,
							    GTK_MESSAGE_ERROR,
							    GTK_BUTTONS_OK,
							    _
							    ("There was an error loading an image: %s"),
							    gerror->
							    message);
				g_signal_connect (G_OBJECT (dialog),
						  "response",
						  G_CALLBACK
						  (gtk_widget_destroy),
						  NULL);

				gtk_window_set_resizable (GTK_WINDOW
							  (dialog), FALSE);

				gtk_widget_show (dialog);
				g_error_free (gerror);
			}
			xkl_debug (150,
				   "Image %d[%s] loaded -> %p[%dx%d]\n",
				   i, image_file, image,
				   gdk_pixbuf_get_width (image),
				   gdk_pixbuf_get_height (image));
			g_free (image_file);
		}
		/* We append the image anyway - even if it is NULL! */
		ind_config->images =
		    g_slist_prepend (ind_config->images, image);
	}
}

void
gkb_indicator_config_init (GkbIndicatorConfig * ind_config,
			   GConfClient * conf_client, XklEngine * engine)
{
	GError *gerror = NULL;
	gchar *sp, *datadir;

	memset (ind_config, 0, sizeof (*ind_config));
	ind_config->conf_client = conf_client;
	ind_config->engine = engine;
	g_object_ref (ind_config->conf_client);

	gconf_client_add_dir (ind_config->conf_client,
			      GKB_INDICATOR_CONFIG_DIR,
			      GCONF_CLIENT_PRELOAD_NONE, &gerror);
	if (gerror != NULL) {
		g_warning ("err1:%s\n", gerror->message);
		g_error_free (gerror);
		gerror = NULL;
	}

	ind_config->icon_theme = gtk_icon_theme_get_default ();

	datadir =
	    gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_DATADIR,
				       "", FALSE, NULL);
	gtk_icon_theme_append_search_path (ind_config->icon_theme, sp =
					   g_build_filename (g_get_home_dir
							     (),
							     ".icons/flags",
							     NULL));
	g_free (sp);

	gtk_icon_theme_append_search_path (ind_config->icon_theme,
					   sp =
					   g_build_filename (datadir,
							     "pixmaps/flags",
							     NULL));
	g_free (sp);

	gtk_icon_theme_append_search_path (ind_config->icon_theme,
					   sp =
					   g_build_filename (datadir,
							     "icons/flags",
							     NULL));
	g_free (sp);
	g_free (datadir);
}

void
gkb_indicator_config_term (GkbIndicatorConfig * ind_config)
{
#if 0
	g_object_unref (G_OBJECT (ind_config->icon_theme));
#endif
	ind_config->icon_theme = NULL;

	gkb_indicator_config_free_images (ind_config);

	gkb_indicator_config_free_enabled_plugins (ind_config);
	g_object_unref (ind_config->conf_client);
	ind_config->conf_client = NULL;
}

void
gkb_indicator_config_update_images (GkbIndicatorConfig *
				    ind_config,
				    GkbKeyboardConfig * kbd_config)
{
	gkb_indicator_config_free_images (ind_config);
	gkb_indicator_config_load_images (ind_config, kbd_config);
}

void
gkb_indicator_config_load_from_gconf (GkbIndicatorConfig * ind_config)
{
	GError *gerror = NULL;

	ind_config->secondary_groups_mask =
	    gconf_client_get_int (ind_config->conf_client,
				  GKB_INDICATOR_CONFIG_KEY_SECONDARIES,
				  &gerror);
	if (gerror != NULL) {
		g_warning ("Error reading configuration:%s\n",
			   gerror->message);
		ind_config->secondary_groups_mask = 0;
		g_error_free (gerror);
		gerror = NULL;
	}

	ind_config->show_flags =
	    gconf_client_get_bool (ind_config->conf_client,
				   GKB_INDICATOR_CONFIG_KEY_SHOW_FLAGS,
				   &gerror);
	if (gerror != NULL) {
		g_warning ("Error reading kbdConfiguration:%s\n",
			   gerror->message);
		ind_config->show_flags = FALSE;
		g_error_free (gerror);
		gerror = NULL;
	}

	gkb_indicator_config_free_enabled_plugins (ind_config);
	ind_config->enabled_plugins =
	    gconf_client_get_list (ind_config->conf_client,
				   GKB_INDICATOR_CONFIG_KEY_ENABLED_PLUGINS,
				   GCONF_VALUE_STRING, &gerror);

	if (gerror != NULL) {
		g_warning ("Error reading kbd_configuration:%s\n",
			   gerror->message);
		ind_config->enabled_plugins = NULL;
		g_error_free (gerror);
		gerror = NULL;
	}
}

void
gkb_indicator_config_save_to_gconf (GkbIndicatorConfig * ind_config)
{
	GConfChangeSet *cs;
	GError *gerror = NULL;

	cs = gconf_change_set_new ();

	gconf_change_set_set_int (cs,
				  GKB_INDICATOR_CONFIG_KEY_SECONDARIES,
				  ind_config->secondary_groups_mask);
	gconf_change_set_set_bool (cs,
				   GKB_INDICATOR_CONFIG_KEY_SHOW_FLAGS,
				   ind_config->show_flags);
	gconf_change_set_set_list (cs,
				   GKB_INDICATOR_CONFIG_KEY_ENABLED_PLUGINS,
				   GCONF_VALUE_STRING,
				   ind_config->enabled_plugins);

	gconf_client_commit_change_set (ind_config->conf_client, cs,
					TRUE, &gerror);
	if (gerror != NULL) {
		g_warning ("Error saving configuration: %s\n",
			   gerror->message);
		g_error_free (gerror);
		gerror = NULL;
	}
	gconf_change_set_unref (cs);
}

void
gkb_indicator_config_activate (GkbIndicatorConfig * ind_config)
{
	xkl_engine_set_secondary_groups_mask (ind_config->engine,
					      ind_config->
					      secondary_groups_mask);
}

void
gkb_indicator_config_start_listen (GkbIndicatorConfig *
				   ind_config,
				   GConfClientNotifyFunc func,
				   gpointer user_data)
{
	gkb_desktop_config_add_listener (ind_config->conf_client,
					 GKB_INDICATOR_CONFIG_DIR, func,
					 user_data,
					 &ind_config->config_listener_id);
}

void
gkb_indicator_config_stop_listen (GkbIndicatorConfig * ind_config)
{
	gkb_desktop_config_remove_listener (ind_config->conf_client,
					    &ind_config->
					    config_listener_id);
}
