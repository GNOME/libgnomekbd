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

#ifndef __GKB_INDICATOR_PLUGIN_H__
#define __GKB_INDICATOR_PLUGIN_H__

#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <libgnomekbd/gkb-keyboard-config.h>

#define MAX_LOCAL_NAME_BUF_LENGTH 512

struct _GkbIndicatorPlugin;

typedef struct _GkbIndicatorPluginContainer {
	GConfClient *conf_client;
} GkbIndicatorPluginContainer;

typedef const struct _GkbIndicatorPlugin
*(*GkbIndicatorPluginGetPluginFunc) (void);

typedef gboolean (*GkbIndicatorPluginInitFunc) (GkbIndicatorPluginContainer
						* pc);

typedef void (*GkbIndicatorPluginGroupChangedFunc) (GtkWidget * notebook,
						    int new_group);

typedef void (*GkbIndicatorPluginConfigChangedFunc) (const
						     GkbKeyboardConfig *
						     from,
						     const
						     GkbKeyboardConfig *
						     to);

typedef int (*GkbIndicatorPluginWindowCreatedFunc) (const Window win,
						    const Window parent);

typedef void (*GkbIndicatorPluginTermFunc) (void);

typedef GtkWidget *(*GkbIndicatorPluginCreateWidget) (void);

typedef GtkWidget *(*GkbIndicatorPluginDecorateWidget) (GtkWidget * widget,
							const gint group,
							const char
							*group_description,
							GkbKeyboardConfig *
							config);

typedef
void (*GkbIndicatorPluginConfigureProperties) (GkbIndicatorPluginContainer
					       * pc, GtkWindow * parent);

typedef struct _GkbIndicatorPlugin {
	const char *name;

	const char *description;

// implemented
	GkbIndicatorPluginInitFunc init_callback;

// implemented
	GkbIndicatorPluginTermFunc term_callback;

// implemented
	 GkbIndicatorPluginConfigureProperties
	    configure_properties_callback;

// implemented
	GkbIndicatorPluginGroupChangedFunc group_changed_callback;

// implemented
	GkbIndicatorPluginWindowCreatedFunc window_created_callback;

// implemented
	GkbIndicatorPluginDecorateWidget decorate_widget_callback;

// non implemented
	GkbIndicatorPluginConfigChangedFunc config_changed_callback;

// non implemented
	GkbIndicatorPluginCreateWidget create_widget_callback;

} GkbIndicatorPlugin;

/**
 * Functions accessible for plugins
 */

extern void gkb_indicator_plugin_container_init (GkbIndicatorPluginContainer *
					     pc,
					     GConfClient * conf_client);

extern void gkb_indicator_plugin_container_term (GkbIndicatorPluginContainer *
					     pc);

extern void
 gkb_indicator_plugin_container_reinit_ui (GkbIndicatorPluginContainer * pc);

extern guint gkb_indicator_plugin_get_num_groups (GkbIndicatorPluginContainer *
					      pc);

extern gchar
    **
gkb_indicator_plugin_load_localized_group_names (GkbIndicatorPluginContainer *
					     pc);

#endif
