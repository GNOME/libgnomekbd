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

#ifndef __GKB_INDICATOR_CONFIG_H__
#define __GKB_INDICATOR_CONFIG_H__

#include "libgnomekbd/gkb-keyboard-config.h"

/*
 * Indicator configuration
 */
typedef struct _GkbIndicatorConfig {
	int secondary_groups_mask;
	gboolean show_flags;

	GSList *enabled_plugins;

	/* private, transient */
	GConfClient *conf_client;
	GSList *images;
	GtkIconTheme *icon_theme;
	int config_listener_id;
	XklEngine *engine;
} GkbIndicatorConfig;

/**
 * GkbIndicatorConfig functions - 
 * some of them require GkbKeyboardConfig as well - 
 * for loading approptiate images
 */
extern void gkb_indicator_config_init (GkbIndicatorConfig *
				       applet_config,
				       GConfClient * conf_client,
				       XklEngine * engine);
extern void gkb_indicator_config_term (GkbIndicatorConfig * applet_config);

extern void gkb_indicator_config_load_from_gconf (GkbIndicatorConfig
						  * applet_config);
extern void gkb_indicator_config_save_to_gconf (GkbIndicatorConfig *
						applet_config);

extern gchar
    * gkb_indicator_config_get_images_file (GkbIndicatorConfig *
					    applet_config,
					    GkbKeyboardConfig *
					    kbd_config, int group);

extern void gkb_indicator_config_load_images (GkbIndicatorConfig *
					      applet_config,
					      GkbKeyboardConfig *
					      kbd_config);
extern void gkb_indicator_config_free_images (GkbIndicatorConfig *
					      applet_config);

/* Should be updated on Indicator/GConf and Kbd/GConf configuration change */
extern void gkb_indicator_config_update_images (GkbIndicatorConfig *
						applet_config,
						GkbKeyboardConfig *
						kbd_config);

/* Should be updated on Indicator/GConf configuration change */
extern void gkb_indicator_config_activate (GkbIndicatorConfig *
					   applet_config);

extern void gkb_indicator_config_start_listen (GkbIndicatorConfig *
					       applet_config,
					       GConfClientNotifyFunc
					       func, gpointer user_data);

extern void gkb_indicator_config_stop_listen (GkbIndicatorConfig *
					      applet_config);

#endif
