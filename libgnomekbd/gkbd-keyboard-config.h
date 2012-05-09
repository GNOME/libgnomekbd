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

#ifndef __GKBD_KEYBOARD_CONFIG_H__
#define __GKBD_KEYBOARD_CONFIG_H__

#include <X11/Xlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <libxklavier/xklavier.h>

#define GKBD_KEYBOARD_SCHEMA "org.gnome.libgnomekbd.keyboard"

extern const gchar GKBD_KEYBOARD_CONFIG_KEY_MODEL[];
extern const gchar GKBD_KEYBOARD_CONFIG_KEY_LAYOUTS[];
extern const gchar GKBD_KEYBOARD_CONFIG_KEY_OPTIONS[];

/*
 * Keyboard Configuration
 */
typedef struct _GkbdKeyboardConfig GkbdKeyboardConfig;
struct _GkbdKeyboardConfig {
	gchar *model;
	gchar **layouts_variants;
	gchar **options;

	/* private, transient */
	GSettings *settings;
	int config_listener_id;
	XklEngine *engine;
};

/*
 * GkbdKeyboardConfig functions
 */
extern void gkbd_keyboard_config_init (GkbdKeyboardConfig * kbd_config,
				       XklEngine * engine);
extern void gkbd_keyboard_config_term (GkbdKeyboardConfig * kbd_config);

extern void gkbd_keyboard_config_load (GkbdKeyboardConfig * kbd_config,
				       GkbdKeyboardConfig *
				       kbd_config_default);

extern void gkbd_keyboard_config_save (GkbdKeyboardConfig * kbd_config);

extern void gkbd_keyboard_config_load_from_x_initial (GkbdKeyboardConfig *
						      kbd_config,
						      XklConfigRec * buf);

extern void gkbd_keyboard_config_load_from_x_current (GkbdKeyboardConfig *
						      kbd_config,
						      XklConfigRec * buf);

extern void gkbd_keyboard_config_start_listen (GkbdKeyboardConfig *
					       kbd_config,
					       GCallback func,
					       gpointer user_data);

extern void gkbd_keyboard_config_stop_listen (GkbdKeyboardConfig *
					      kbd_config);

extern gboolean gkbd_keyboard_config_equals (GkbdKeyboardConfig *
					     kbd_config1,
					     GkbdKeyboardConfig *
					     kbd_config2);

extern gboolean gkbd_keyboard_config_activate (GkbdKeyboardConfig *
					       kbd_config);

extern const gchar *gkbd_keyboard_config_merge_items (const gchar * parent,
						      const gchar * child);

extern gboolean gkbd_keyboard_config_split_items (const gchar * merged,
						  gchar ** parent,
						  gchar ** child);

extern gboolean gkbd_keyboard_config_get_descriptions (XklConfigRegistry *
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

extern const gchar *gkbd_keyboard_config_format_full_description (const
								  gchar *
								  layout_descr,
								  const
								  gchar *
								  variant_descr);

extern gchar *gkbd_keyboard_config_to_string (const GkbdKeyboardConfig *
					      config);

extern gchar
    **
gkbd_keyboard_config_add_default_switch_option_if_necessary (gchar **
							     layouts_list,
							     gchar **
							     options_list,
							     gboolean *
							     was_appended);

#endif
