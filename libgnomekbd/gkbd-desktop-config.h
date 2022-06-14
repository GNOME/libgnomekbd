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

#ifndef __GKBD_DESKTOP_CONFIG_H__
#define __GKBD_DESKTOP_CONFIG_H__

#include <X11/Xlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <libxklavier/xklavier.h>

#define GKBD_DESKTOP_SCHEMA "org.gnome.libgnomekbd.desktop"

/*
 * General configuration
 */
typedef struct _GkbdDesktopConfig GkbdDesktopConfig;
struct _GkbdDesktopConfig {
	gint default_group;
	gboolean group_per_app;
	gboolean handle_indicators;
	gboolean layout_names_as_group_names;
	gboolean load_extra_items;

	/* private, transient */
	GSettings *settings;
	int config_listener_id;
	XklEngine *engine;
};

/*
 * GkbdDesktopConfig functions
 */
void gkbd_desktop_config_init (GkbdDesktopConfig * config,
				      XklEngine * engine);
void gkbd_desktop_config_term (GkbdDesktopConfig * config);

void gkbd_desktop_config_load (GkbdDesktopConfig * config);

void gkbd_desktop_config_save (GkbdDesktopConfig * config);

gboolean gkbd_desktop_config_activate (GkbdDesktopConfig * config);

gboolean
gkbd_desktop_config_load_group_descriptions (GkbdDesktopConfig
					     * config,
					     XklConfigRegistry *
					     registry,
					     const gchar **
					     layout_ids,
					     const gchar **
					     variant_ids,
					     gchar ***
					     short_group_names,
					     gchar *** full_group_names);

void gkbd_desktop_config_lock_next_group (GkbdDesktopConfig *
						 config);

void gkbd_desktop_config_lock_prev_group (GkbdDesktopConfig *
						 config);

void gkbd_desktop_config_restore_group (GkbdDesktopConfig * config);

void gkbd_desktop_config_start_listen (GkbdDesktopConfig * config,
					      GCallback func,
					      gpointer user_data);

void gkbd_desktop_config_stop_listen (GkbdDesktopConfig * config);

#endif
