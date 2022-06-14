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

#ifndef __GKBD_UTIL_H__
#define __GKBD_UTIL_H__

#include <glib.h>
#include <gdk/gdk.h>

void gkbd_install_glib_log_appender (void);

GdkRectangle *gkbd_preview_load_position (void);

void gkbd_preview_save_position (GdkRectangle * rect);

/* Missing in glib */
gboolean gkbd_strv_remove (gchar ** arr, const gchar * element);

gchar **gkbd_strv_append (gchar ** arr, gchar * element);

void gkbd_strv_behead (gchar ** arr);

#endif
