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

#ifndef __GKBD_INDICATOR_CONFIG_H__
#define __GKBD_INDICATOR_CONFIG_H__

#include <gtk/gtk.h>

#include "libgnomekbd/gkbd-keyboard-config.h"

/*
 * Indicator configuration
 */
typedef struct _GkbdIndicatorConfig GkbdIndicatorConfig;
struct _GkbdIndicatorConfig {
	int secondary_groups_mask;
	gboolean show_flags;

	gchar *font_family;
	int font_size;
	gchar *foreground_color;
	gchar *background_color;

	/* private, transient */
	GSettings *settings;
	GSList *image_filenames;
	GtkIconTheme *icon_theme;
	int config_listener_id;
	XklEngine *engine;
};

/*
 * GkbdIndicatorConfig functions - 
 * some of them require GkbdKeyboardConfig as well - 
 * for loading approptiate images
 */
void gkbd_indicator_config_init (GkbdIndicatorConfig *
					applet_config, XklEngine * engine);
void gkbd_indicator_config_term (GkbdIndicatorConfig *
					applet_config);

void gkbd_indicator_config_load (GkbdIndicatorConfig
					* applet_config);
void gkbd_indicator_config_save (GkbdIndicatorConfig *
					applet_config);

void
gkbd_indicator_config_get_font_for_widget (GkbdIndicatorConfig * ind_config,
					   GtkWidget           * widget,
					   gchar               ** font_family,
					   int                  * font_size);

gchar *
gkbd_indicator_config_get_fg_color_for_widget (GkbdIndicatorConfig * ind_config,
					       GtkWidget           * widget);

void gkbd_indicator_config_refresh_style (GkbdIndicatorConfig *
						 applet_config);

gchar
    * gkbd_indicator_config_get_images_file (GkbdIndicatorConfig *
					     applet_config,
					     GkbdKeyboardConfig *
					     kbd_config, int group);

void gkbd_indicator_config_load_image_filenames (GkbdIndicatorConfig
							* applet_config,
							GkbdKeyboardConfig
							* kbd_config);
void gkbd_indicator_config_free_image_filenames (GkbdIndicatorConfig
							* applet_config);

/* Should be updated on Indicator/GConf configuration change */
void gkbd_indicator_config_activate (GkbdIndicatorConfig *
					    applet_config);

void gkbd_indicator_config_start_listen (GkbdIndicatorConfig *
						applet_config,
						GCallback func,
						gpointer user_data);

void gkbd_indicator_config_stop_listen (GkbdIndicatorConfig *
					       applet_config);

#endif
