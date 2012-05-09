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
#include <string.h>
#include <stdlib.h>
#include <X11/keysym.h>

#include <pango/pango.h>

#include <glib/gi18n-lib.h>
#include <gdk/gdkx.h>

#include <gkbd-keyboard-config.h>
#include <gkbd-indicator-config.h>

#include <gkbd-config-private.h>

/**
 * GkbdIndicatorConfig:
 */

const gchar GKBD_INDICATOR_CONFIG_KEY_SHOW_FLAGS[] = "show-flags";
const gchar GKBD_INDICATOR_CONFIG_KEY_SECONDARIES[] = "secondary";
const gchar GKBD_INDICATOR_CONFIG_KEY_FONT_FAMILY[] = "font-family";
const gchar GKBD_INDICATOR_CONFIG_KEY_FONT_SIZE[] = "font-size";
const gchar GKBD_INDICATOR_CONFIG_KEY_FOREGROUND_COLOR[] =
    "foreground-color";
const gchar GKBD_INDICATOR_CONFIG_KEY_BACKGROUND_COLOR[] =
    "background-color";

/*
 * static applet config functions
 */

static void
gkbd_indicator_config_load_font (GkbdIndicatorConfig * ind_config)
{
	ind_config->font_family =
	    g_settings_get_string (ind_config->settings,
				   GKBD_INDICATOR_CONFIG_KEY_FONT_FAMILY);

	ind_config->font_size =
	    g_settings_get_int (ind_config->settings,
				GKBD_INDICATOR_CONFIG_KEY_FONT_SIZE);

	xkl_debug (150, "font: [%s], size %d\n",
		   ind_config->font_family ? ind_config->
		   font_family : "(null)", ind_config->font_size);

}

static void
gkbd_indicator_config_load_colors (GkbdIndicatorConfig * ind_config)
{
	ind_config->foreground_color =
	    g_settings_get_string (ind_config->settings,
				   GKBD_INDICATOR_CONFIG_KEY_FOREGROUND_COLOR);

	ind_config->background_color =
	    g_settings_get_string (ind_config->settings,
				   GKBD_INDICATOR_CONFIG_KEY_BACKGROUND_COLOR);
}

void
gkbd_indicator_config_get_font_for_widget (GkbdIndicatorConfig *
					   ind_config, GtkWidget * widget,
					   gchar ** font_family,
					   int *font_size)
{
	GtkStyleContext *context;
	const PangoFontDescription *fd = NULL;

	g_return_if_fail (GTK_IS_WIDGET (widget));

	if (ind_config->font_family != NULL &&
	    ind_config->font_family[0] != '\0') {
		if (font_family)
			*font_family = g_strdup (ind_config->font_family);
		if (font_size)
			*font_size = ind_config->font_size;

		return;
	}

	context = gtk_widget_get_style_context (widget);
	fd = gtk_style_context_get_font (context, GTK_STATE_FLAG_NORMAL);

	if (font_family)
		*font_family =
		    g_strdup (pango_font_description_get_family (fd));
	if (font_size)
		*font_size =
		    pango_font_description_get_size (fd) / PANGO_SCALE;
}

gchar *
gkbd_indicator_config_get_fg_color_for_widget (GkbdIndicatorConfig *
					       ind_config,
					       GtkWidget * widget)
{
	GtkStyleContext *context;
	GdkRGBA rgba;

	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	if (ind_config->foreground_color != NULL &&
	    ind_config->foreground_color[0] != '\0')
		return g_strdup (ind_config->foreground_color);

	context = gtk_widget_get_style_context (widget);
	gtk_style_context_get_color (context, GTK_STATE_FLAG_NORMAL,
				     &rgba);

	return g_strdup_printf ("%g %g %g", rgba.red, rgba.green,
				rgba.blue);
}

void
gkbd_indicator_config_refresh_style (GkbdIndicatorConfig * ind_config)
{
	g_free (ind_config->font_family);
	g_free (ind_config->foreground_color);
	g_free (ind_config->background_color);
	gkbd_indicator_config_load_font (ind_config);
	gkbd_indicator_config_load_colors (ind_config);
}

gchar *
gkbd_indicator_config_get_images_file (GkbdIndicatorConfig *
				       ind_config,
				       GkbdKeyboardConfig *
				       kbd_config, int group)
{
	char *image_file = NULL;
	GtkIconInfo *icon_info = NULL;

	if (!ind_config->show_flags)
		return NULL;

	if ((kbd_config->layouts_variants != NULL) &&
	    (g_strv_length (kbd_config->layouts_variants) > group)) {
		char *full_layout_name =
		    kbd_config->layouts_variants[group];

		if (full_layout_name != NULL) {
			char *l, *v;
			gkbd_keyboard_config_split_items (full_layout_name,
							  &l, &v);
			if (l != NULL) {
				/* probably there is something in theme? */
				icon_info = gtk_icon_theme_lookup_icon
				    (ind_config->icon_theme, l, 48, 0);

				/* Unbelievable but happens */
				if (icon_info != NULL &&
				    gtk_icon_info_get_filename (icon_info)
				    == NULL) {
					gtk_icon_info_free (icon_info);
					icon_info = NULL;
				}
			}
		}
	}
	/* fallback to the default value */
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
gkbd_indicator_config_load_image_filenames (GkbdIndicatorConfig *
					    ind_config,
					    GkbdKeyboardConfig *
					    kbd_config)
{
	int i;
	ind_config->image_filenames = NULL;

	if (!ind_config->show_flags)
		return;

	for (i = xkl_engine_get_max_num_groups (ind_config->engine);
	     --i >= 0;) {
		gchar *image_file =
		    gkbd_indicator_config_get_images_file (ind_config,
							   kbd_config,
							   i);
		ind_config->image_filenames =
		    g_slist_prepend (ind_config->image_filenames,
				     image_file);
	}
}

void
gkbd_indicator_config_free_image_filenames (GkbdIndicatorConfig *
					    ind_config)
{
	while (ind_config->image_filenames) {
		if (ind_config->image_filenames->data)
			g_free (ind_config->image_filenames->data);
		ind_config->image_filenames =
		    g_slist_delete_link (ind_config->image_filenames,
					 ind_config->image_filenames);
	}
}

void
gkbd_indicator_config_init (GkbdIndicatorConfig * ind_config,
			    XklEngine * engine)
{
	gchar *sp;

	memset (ind_config, 0, sizeof (*ind_config));
	ind_config->settings =
	    g_settings_new ("org.gnome.libgnomekbd.indicator");
	ind_config->engine = engine;

	ind_config->icon_theme = gtk_icon_theme_get_default ();

	gtk_icon_theme_append_search_path (ind_config->icon_theme, sp =
					   g_build_filename
					   (g_get_user_data_dir (),
					    "icons/flags", NULL));
	g_free (sp);

	gtk_icon_theme_append_search_path (ind_config->icon_theme,
					   sp =
					   g_build_filename (DATADIR,
							     "pixmaps/flags",
							     NULL));
	g_free (sp);

	gtk_icon_theme_append_search_path (ind_config->icon_theme,
					   sp =
					   g_build_filename (DATADIR,
							     "icons/flags",
							     NULL));
	g_free (sp);

	gtk_icon_theme_append_search_path (ind_config->icon_theme,
					   ICONSDIR);
}

void
gkbd_indicator_config_term (GkbdIndicatorConfig * ind_config)
{
	g_free (ind_config->font_family);
	ind_config->font_family = NULL;

	g_free (ind_config->foreground_color);
	ind_config->foreground_color = NULL;

	g_free (ind_config->background_color);
	ind_config->background_color = NULL;

	ind_config->icon_theme = NULL;

	gkbd_indicator_config_free_image_filenames (ind_config);

	g_object_unref (ind_config->settings);
	ind_config->settings = NULL;
}

void
gkbd_indicator_config_load (GkbdIndicatorConfig * ind_config)
{
	ind_config->secondary_groups_mask =
	    g_settings_get_int (ind_config->settings,
				GKBD_INDICATOR_CONFIG_KEY_SECONDARIES);

	ind_config->show_flags =
	    g_settings_get_boolean (ind_config->settings,
				    GKBD_INDICATOR_CONFIG_KEY_SHOW_FLAGS);

	gkbd_indicator_config_load_font (ind_config);
	gkbd_indicator_config_load_colors (ind_config);
}

void
gkbd_indicator_config_save (GkbdIndicatorConfig * ind_config)
{
	g_settings_delay (ind_config->settings);

	g_settings_set_int (ind_config->settings,
			    GKBD_INDICATOR_CONFIG_KEY_SECONDARIES,
			    ind_config->secondary_groups_mask);
	g_settings_set_boolean (ind_config->settings,
				GKBD_INDICATOR_CONFIG_KEY_SHOW_FLAGS,
				ind_config->show_flags);

	g_settings_apply (ind_config->settings);
}

void
gkbd_indicator_config_activate (GkbdIndicatorConfig * ind_config)
{
	xkl_engine_set_secondary_groups_mask (ind_config->engine,
					      ind_config->secondary_groups_mask);
}

/**
 * gkbd_indicator_config_start_listen:
 * @func: (scope notified): a function to call when settings are changed
 */
void
gkbd_indicator_config_start_listen (GkbdIndicatorConfig *
				    ind_config,
				    GCallback func, gpointer user_data)
{
	ind_config->config_listener_id =
	    g_signal_connect (ind_config->settings, "changed", func,
			      user_data);
}

void
gkbd_indicator_config_stop_listen (GkbdIndicatorConfig * ind_config)
{
	g_signal_handler_disconnect (ind_config->settings,
				     ind_config->config_listener_id);
	ind_config->config_listener_id = 0;
}
