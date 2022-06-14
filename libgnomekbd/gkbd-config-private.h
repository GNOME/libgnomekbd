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

#ifndef __GKBD_CONFIG_PRIVATE_H__
#define __GKBD_CONFIG_PRIVATE_H__

#include "libgnomekbd/gkbd-desktop-config.h"
#include "libgnomekbd/gkbd-keyboard-config.h"

#define GKBD_SCHEMA_PREVIEW "org.gnome.libgnomekbd.preview"

/**
 * General config functions (private)
 */

void gkbd_keyboard_config_model_set (GkbdKeyboardConfig *
					    kbd_config,
					    const gchar * model_name);

void gkbd_keyboard_config_options_set (GkbdKeyboardConfig *
					      kbd_config, gint idx,
					      const gchar * group_name,
					      const gchar * option_name);

gboolean gkbd_keyboard_config_options_is_set (GkbdKeyboardConfig *
						     kbd_config,
						     const gchar *
						     group_name,
						     const gchar *
						     option_name);

gboolean gkbd_keyboard_config_dump_settings (GkbdKeyboardConfig *
						    kbd_config,
						    const char *file_name);

void gkbd_keyboard_config_start_listen (GkbdKeyboardConfig *
					       kbd_config,
					       GCallback func,
					       gpointer user_data);

void gkbd_keyboard_config_stop_listen (GkbdKeyboardConfig *
					      kbd_config);

gboolean gkbd_keyboard_config_get_lv_descriptions (XklConfigRegistry
							  *
							  config_registry,
							  const gchar *
							  layout_name,
							  const gchar *
							  variant_name,
							  gchar **
							  layout_short_descr,
							  gchar **
							  layout_descr,
							  gchar **
							  variant_short_descr,
							  gchar **
							  variant_descr);

#endif
