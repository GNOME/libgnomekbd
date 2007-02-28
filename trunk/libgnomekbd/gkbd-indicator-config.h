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
typedef struct _GkbdIndicatorConfig {
	int secondary_groups_mask;
	gboolean show_flags;

	GSList *enabled_plugins;

	/* private, transient */
	GConfClient *conf_client;
	GSList *images;
	GtkIconTheme *icon_theme;
	int config_listener_id;
	XklEngine *engine;
} GkbdIndicatorConfig;

/**
 * GkbdIndicatorConfig functions - 
 * some of them require GkbdKeyboardConfig as well - 
 * for loading approptiate images
 */
extern void gkbd_indicator_config_init (GkbdIndicatorConfig *
					applet_config,
					GConfClient * conf_client,
					XklEngine * engine);
extern void gkbd_indicator_config_term (GkbdIndicatorConfig *
					applet_config);

extern void gkbd_indicator_config_load_from_gconf (GkbdIndicatorConfig
						   * applet_config);
extern void gkbd_indicator_config_save_to_gconf (GkbdIndicatorConfig *
						 applet_config);

extern gchar
    * gkbd_indicator_config_get_images_file (GkbdIndicatorConfig *
					     applet_config,
					     GkbdKeyboardConfig *
					     kbd_config, int group);

extern void gkbd_indicator_config_load_images (GkbdIndicatorConfig *
					       applet_config,
					       GkbdKeyboardConfig *
					       kbd_config);
extern void gkbd_indicator_config_free_images (GkbdIndicatorConfig *
					       applet_config);

/* Should be updated on Indicator/GConf and Kbd/GConf configuration change */
extern void gkbd_indicator_config_update_images (GkbdIndicatorConfig *
						 applet_config,
						 GkbdKeyboardConfig *
						 kbd_config);

/* Should be updated on Indicator/GConf configuration change */
extern void gkbd_indicator_config_activate (GkbdIndicatorConfig *
					    applet_config);

extern void gkbd_indicator_config_start_listen (GkbdIndicatorConfig *
						applet_config,
						GConfClientNotifyFunc
						func, gpointer user_data);

extern void gkbd_indicator_config_stop_listen (GkbdIndicatorConfig *
					       applet_config);

#endif
