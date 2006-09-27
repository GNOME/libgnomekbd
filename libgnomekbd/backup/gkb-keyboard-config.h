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

#ifndef __GKB_KEYBOARD_CONFIG_H__
#define __GKB_KEYBOARD_CONFIG_H__

#include <X11/Xlib.h>

#include <glib.h>
#include <glib/gslist.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gconf/gconf-client.h>

#include <libxklavier/xklavier.h>

extern const gchar GKB_KEYBOARD_CONFIG_DIR[];
extern const gchar GKB_KEYBOARD_CONFIG_KEY_MODEL[];
extern const gchar GKB_KEYBOARD_CONFIG_KEY_LAYOUTS[];
extern const gchar GKB_KEYBOARD_CONFIG_KEY_OPTIONS[];

/*
 * Keyboard Configuration
 */
typedef struct _GkbKeyboardConfig {
	gchar *model;
	GSList *layouts;
	GSList *options;

	/* private, transient */
	GConfClient *conf_client;
	int config_listener_id;
	XklEngine *engine;
} GkbKeyboardConfig;

/**
 * GkbKeyboardConfig functions
 */
extern void gkb_keyboard_config_init (GkbKeyboardConfig * kbd_config,
				      GConfClient * conf_client,
				      XklEngine * engine);
extern void gkb_keyboard_config_term (GkbKeyboardConfig * kbd_config);

extern void gkb_keyboard_config_load_from_gconf (GkbKeyboardConfig *
						 kbd_config,
						 GkbKeyboardConfig *
						 kbd_config_default);

extern void gkb_keyboard_config_save_to_gconf (GkbKeyboardConfig *
					       kbd_config);

extern void gkb_keyboard_config_load_from_gconf_backup (GkbKeyboardConfig
							* kbd_config);

extern void gkb_keyboard_config_save_to_gconf_backup (GkbKeyboardConfig *
						      kbd_config);

extern void gkb_keyboard_config_load_from_x_initial (GkbKeyboardConfig *
						     kbd_config);

extern void gkb_keyboard_config_load_from_x_current (GkbKeyboardConfig *
						     kbd_config);

extern void gkb_keyboard_config_start_listen (GkbKeyboardConfig *
					      kbd_config,
					      GConfClientNotifyFunc func,
					      gpointer user_data);

extern void gkb_keyboard_config_stop_listen (GkbKeyboardConfig *
					     kbd_config);

extern gboolean gkb_keyboard_config_equals (GkbKeyboardConfig *
					    kbd_config1,
					    GkbKeyboardConfig *
					    kbd_config2);

extern gboolean gkb_keyboard_config_activate (GkbKeyboardConfig *
					      kbd_config);

extern const gchar *gkb_keyboard_config_merge_items (const gchar * parent,
						     const gchar * child);

extern gboolean gkb_keyboard_config_split_items (const gchar * merged,
						 gchar ** parent,
						 gchar ** child);

extern gboolean gkb_keyboard_config_get_descriptions (XklConfigRegistry *
						      config_registry,
						      const gchar * name,
						      gchar **
						      layout_short_descr,
						      gchar **
						      layout_descr,
						      gchar **
						      variant_short_descr,
						      gchar **
						      variant_descr);

extern const gchar *gkb_keyboard_config_format_full_layout (const gchar
							    *
							    layout_descr,
							    const gchar *
							    variant_descr);

extern gchar *gkb_keyboard_config_to_string (const GkbKeyboardConfig *
					     config);

#endif
