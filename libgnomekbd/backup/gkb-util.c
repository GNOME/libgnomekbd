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

#include <config.h>

#include <gkb-util.h>

#include <time.h>

#include <glib/gi18n.h>
#include <gtk/gtkmessagedialog.h>

#include <libxklavier/xklavier.h>

#include <gconf/gconf-client.h>

#include <gkb-config-private.h>

static void
gkb_log_appender (const char file[], const char function[],
		  int level, const char format[], va_list args)
{
	time_t now = time (NULL);
	g_log (NULL, G_LOG_LEVEL_DEBUG, "[%08ld,%03d,%s:%s/] \t",
	       (long) now, level, file, function);
	g_logv (NULL, G_LOG_LEVEL_DEBUG, format, args);
}

void
gkb_install_glib_log_appender (void)
{
	xkl_set_log_appender (gkb_log_appender);
}

#define GKB_PREVIEW_CONFIG_KEY_PREFIX  GKB_CONFIG_KEY_PREFIX "/preview"

const gchar GKB_PREVIEW_CONFIG_DIR[] = GKB_PREVIEW_CONFIG_KEY_PREFIX;
const gchar GKB_PREVIEW_CONFIG_KEY_X[] =
    GKB_PREVIEW_CONFIG_KEY_PREFIX "/x";
const gchar GKB_PREVIEW_CONFIG_KEY_Y[] =
    GKB_PREVIEW_CONFIG_KEY_PREFIX "/y";
const gchar GKB_PREVIEW_CONFIG_KEY_WIDTH[] =
    GKB_PREVIEW_CONFIG_KEY_PREFIX "/width";
const gchar GKB_PREVIEW_CONFIG_KEY_HEIGHT[] =
    GKB_PREVIEW_CONFIG_KEY_PREFIX "/height";

GdkRectangle *
gkb_preview_load_position (void)
{
	GError *gerror = NULL;
	GdkRectangle *rv = NULL;
	gint x, y, w, h;
	GConfClient *conf_client = gconf_client_get_default ();

	if (conf_client == NULL)
		return NULL;

	x = gconf_client_get_int (conf_client,
				  GKB_PREVIEW_CONFIG_KEY_X, &gerror);
	if (gerror != NULL) {
		xkl_debug (0, "Error getting the preview x: %s\n",
			   gerror->message);
		g_error_free (gerror);
		g_object_unref (G_OBJECT (conf_client));
		return NULL;
	}

	y = gconf_client_get_int (conf_client,
				  GKB_PREVIEW_CONFIG_KEY_Y, &gerror);
	if (gerror != NULL) {
		xkl_debug (0, "Error getting the preview y: %s\n",
			   gerror->message);
		g_error_free (gerror);
		g_object_unref (G_OBJECT (conf_client));
		return NULL;
	}

	w = gconf_client_get_int (conf_client,
				  GKB_PREVIEW_CONFIG_KEY_WIDTH, &gerror);
	if (gerror != NULL) {
		xkl_debug (0, "Error getting the preview width: %s\n",
			   gerror->message);
		g_error_free (gerror);
		g_object_unref (G_OBJECT (conf_client));
		return NULL;
	}

	h = gconf_client_get_int (conf_client,
				  GKB_PREVIEW_CONFIG_KEY_HEIGHT, &gerror);
	if (gerror != NULL) {
		xkl_debug (0, "Error getting the preview height: %s\n",
			   gerror->message);
		g_error_free (gerror);
		g_object_unref (G_OBJECT (conf_client));
		return NULL;
	}

	g_object_unref (G_OBJECT (conf_client));

	// default values should be just ignored
	if (x == -1 || y == -1 || w == -1 || h == -1)
		return NULL;

	rv = g_new (GdkRectangle, 1);
	rv->x = x;
	rv->y = y;
	rv->width = w;
	rv->height = h;
	return rv;
}

void
gkb_preview_save_position (GdkRectangle * rect)
{
	GConfClient *conf_client = gconf_client_get_default ();
	GConfChangeSet *cs;
	GError *gerror = NULL;

	cs = gconf_change_set_new ();

	gconf_change_set_set_int (cs, GKB_PREVIEW_CONFIG_KEY_X, rect->x);
	gconf_change_set_set_int (cs, GKB_PREVIEW_CONFIG_KEY_Y, rect->y);
	gconf_change_set_set_int (cs, GKB_PREVIEW_CONFIG_KEY_WIDTH,
				  rect->width);
	gconf_change_set_set_int (cs, GKB_PREVIEW_CONFIG_KEY_HEIGHT,
				  rect->height);

	gconf_client_commit_change_set (conf_client, cs, TRUE, &gerror);
	if (gerror != NULL) {
		g_warning ("Error saving preview configuration: %s\n",
			   gerror->message);
		g_error_free (gerror);
	}
	gconf_change_set_unref (cs);
	g_object_unref (G_OBJECT (conf_client));
}
