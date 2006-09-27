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

#ifndef __GKB_DESKTOP_CONFIG_H__
#define __GKB_DESKTOP_CONFIG_H__

#include <X11/Xlib.h>

#include <glib.h>
#include <glib/gslist.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gconf/gconf-client.h>

#include <libxklavier/xklavier.h>

extern const gchar GKB_DESKTOP_CONFIG_DIR[];
extern const gchar GKB_DESKTOP_CONFIG_KEY_DEFAULT_GROUP[];
extern const gchar GKB_DESKTOP_CONFIG_KEY_GROUP_PER_WINDOW[];
extern const gchar GKB_DESKTOP_CONFIG_KEY_HANDLE_INDICATORS[];
extern const gchar GKB_DESKTOP_CONFIG_KEY_LAYOUT_NAMES_AS_GROUP_NAMES[];

/*
 * General configuration
 */
typedef struct _GkbDesktopConfig {
	gint default_group;
	gboolean group_per_app;
	gboolean handle_indicators;
	gboolean layout_names_as_group_names;

	/* private, transient */
	GConfClient *conf_client;
	int config_listener_id;
	XklEngine *engine;
} GkbDesktopConfig;

/**
 * GkbDesktopConfig functions
 */
extern void gkb_desktop_config_init (GkbDesktopConfig * config,
				     GConfClient * conf_client,
				     XklEngine * engine);
extern void gkb_desktop_config_term (GkbDesktopConfig * config);

extern void gkb_desktop_config_load_from_gconf (GkbDesktopConfig * config);

extern void gkb_desktop_config_save_to_gconf (GkbDesktopConfig * config);

extern gboolean gkb_desktop_config_activate (GkbDesktopConfig * config);

/* Affected by XKB and XKB/GConf configuration */
extern gchar
    ** gkb_desktop_config_load_group_descriptions_utf8 (GkbDesktopConfig *
							config,
							XklConfigRegistry *
							config_registry);


/* Using DBUS */
extern gboolean
gkb_desktop_config_load_remote_group_descriptions_utf8 (GkbDesktopConfig *
							config,
							gchar ***
							short_group_names,
							gchar ***
							full_group_names);

extern void gkb_desktop_config_lock_next_group (GkbDesktopConfig * config);

extern void gkb_desktop_config_lock_prev_group (GkbDesktopConfig * config);

extern void gkb_desktop_config_restore_group (GkbDesktopConfig * config);

extern void gkb_desktop_config_start_listen (GkbDesktopConfig * config,
					     GConfClientNotifyFunc func,
					     gpointer user_data);

extern void gkb_desktop_config_stop_listen (GkbDesktopConfig * config);

#endif
