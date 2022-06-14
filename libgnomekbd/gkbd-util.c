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

#include <gkbd-util.h>

#include <time.h>

#include <glib/gi18n-lib.h>

#include <libxklavier/xklavier.h>

#include <gkbd-config-private.h>

static void
gkbd_log_appender (const char file[], const char function[],
		   int level, const char format[], va_list args)
{
	time_t now = time (NULL);
	g_log (NULL, G_LOG_LEVEL_DEBUG, "[%08ld,%03d,%s:%s/] \t",
	       (long) now, level, file, function);
	g_logv (NULL, G_LOG_LEVEL_DEBUG, format, args);
}

void
gkbd_install_glib_log_appender (void)
{
	xkl_set_log_appender (gkbd_log_appender);
}

const gchar GKBD_PREVIEW_CONFIG_KEY_X[] = "x";
const gchar GKBD_PREVIEW_CONFIG_KEY_Y[] = "y";
const gchar GKBD_PREVIEW_CONFIG_KEY_WIDTH[] = "width";
const gchar GKBD_PREVIEW_CONFIG_KEY_HEIGHT[] = "height";

/**
 * gkbd_preview_load_position:
 *
 * Returns: (transfer full): A rectangle to use
 */
GdkRectangle *
gkbd_preview_load_position (void)
{
	GdkRectangle *rv = NULL;
	gint x, y, w, h;
	GSettings *settings = g_settings_new (GKBD_SCHEMA_PREVIEW);

	if (settings == NULL)
		return NULL;

	x = g_settings_get_int (settings, GKBD_PREVIEW_CONFIG_KEY_X);

	y = g_settings_get_int (settings, GKBD_PREVIEW_CONFIG_KEY_Y);

	w = g_settings_get_int (settings, GKBD_PREVIEW_CONFIG_KEY_WIDTH);

	h = g_settings_get_int (settings, GKBD_PREVIEW_CONFIG_KEY_HEIGHT);

	g_object_unref (G_OBJECT (settings));

	rv = g_new (GdkRectangle, 1);
	if (x == -1 || y == -1 || w == -1 || h == -1) {
		/* default values should be treated as 
		 * "0.75 of the screen size" */
		GdkRectangle rect;
		gint w, h;
		gdk_monitor_get_geometry (gdk_display_get_primary_monitor
                                          (gdk_display_get_default ()), &rect);
		w = rect.width; h = rect.height;
		rv->x = w >> 3;
		rv->y = h >> 3;
		rv->width = w - (w >> 2);
		rv->height = h - (h >> 2);
	} else {
		rv->x = x;
		rv->y = y;
		rv->width = w;
		rv->height = h;
	}
	return rv;
}

void
gkbd_preview_save_position (GdkRectangle * rect)
{
	GSettings *settings = g_settings_new (GKBD_SCHEMA_PREVIEW);

	g_settings_delay (settings);

	g_settings_set_int (settings, GKBD_PREVIEW_CONFIG_KEY_X, rect->x);
	g_settings_set_int (settings, GKBD_PREVIEW_CONFIG_KEY_Y, rect->y);
	g_settings_set_int (settings, GKBD_PREVIEW_CONFIG_KEY_WIDTH,
			    rect->width);
	g_settings_set_int (settings, GKBD_PREVIEW_CONFIG_KEY_HEIGHT,
			    rect->height);

	g_settings_apply (settings);
	g_object_unref (G_OBJECT (settings));
}

gboolean
gkbd_strv_remove (gchar ** arr, const gchar * element)
{
	gchar **p = arr;
	if (p != NULL) {
		while (*p != NULL) {
			if (!strcmp (*p, element)) {
				gkbd_strv_behead (p);
				return TRUE;
			}
			p++;
		}
	}
	return FALSE;
}

/**
 * gkbd_strv_append:
 *
 * Returns: (transfer full) (array zero-terminated=1): Append string to strv array
 */
gchar **
gkbd_strv_append (gchar ** arr, gchar * element)
{
	gint old_length = (arr == NULL) ? 0 : g_strv_length (arr);
	gchar **new_arr = g_new0 (gchar *, old_length + 2);
	if (arr != NULL) {
		memcpy (new_arr, arr, old_length * sizeof (gchar *));
		g_free (arr);
	}
	new_arr[old_length] = element;
	return new_arr;
}

void
gkbd_strv_behead (gchar ** arr)
{
	if (arr == NULL || *arr == NULL)
		return;
	g_free (*arr);
	memmove (arr, arr + 1, g_strv_length (arr) * sizeof (gchar *));
}
