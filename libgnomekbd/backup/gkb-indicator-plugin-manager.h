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

#ifndef __GSWITCHIT_PLUGIN_MANAGER_H__
#define __GSWITCHIT_PLUGIN_MANAGER_H__

#include <gmodule.h>
#include <libgnomekbd/gkb-indicator-plugin.h>

typedef struct _GkbIndicatorPluginManager {
	GHashTable *all_plugin_recs;
	GSList *inited_plugin_recs;
} GkbIndicatorPluginManager;

typedef struct _GkbIndicatorPluginManagerRecord {
	const char *full_path;
	GModule *module;
	const GkbIndicatorPlugin *plugin;
} GkbIndicatorPluginManagerRecord;

extern void
 gkb_indicator_plugin_manager_init (GkbIndicatorPluginManager * manager);

extern void
 gkb_indicator_plugin_manager_term (GkbIndicatorPluginManager * manager);

extern void
 gkb_indicator_plugin_manager_init_enabled_plugins (GkbIndicatorPluginManager * manager,
						GkbIndicatorPluginContainer
						* pc,
						GSList * enabled_plugins);

extern void
 gkb_indicator_plugin_manager_term_initialized_plugins (GkbIndicatorPluginManager * manager);

extern void
 gkb_indicator_plugin_manager_toggle_plugins (GkbIndicatorPluginManager * manager,
					  GkbIndicatorPluginContainer * pc,
					  GSList * enabled_plugins);

extern const GkbIndicatorPlugin
    * gkb_indicator_plugin_manager_get_plugin (GkbIndicatorPluginManager *
					   manager, const char *full_path);

extern void
 gkb_indicator_plugin_manager_promote_plugin (GkbIndicatorPluginManager * manager,
					  GSList * enabled_plugins,
					  const char *full_path);

extern void
 gkb_indicator_plugin_manager_demote_plugin (GkbIndicatorPluginManager * manager,
					 GSList * enabled_plugins,
					 const char *full_path);

extern void
 gkb_indicator_plugin_manager_enable_plugin (GkbIndicatorPluginManager * manager,
					 GSList ** enabled_plugins,
					 const char *full_path);

extern void
 gkb_indicator_plugin_manager_disable_plugin (GkbIndicatorPluginManager * manager,
					  GSList ** enabled_plugins,
					  const char *full_path);

extern void
 gkb_indicator_plugin_manager_configure_plugin (GkbIndicatorPluginManager * manager,
					    GkbIndicatorPluginContainer *
					    pc, const char *full_path,
					    GtkWindow * parent);

// actual calling plugin notification methods

extern void
 gkb_indicator_plugin_manager_group_changed (GkbIndicatorPluginManager * manager,
					 GtkWidget * notebook,
					 int new_group);

extern void
 gkb_indicator_plugin_manager_config_changed (GkbIndicatorPluginManager * manager,
					  GkbKeyboardConfig * from,
					  GkbKeyboardConfig * to);

extern int
 gkb_indicator_plugin_manager_window_created (GkbIndicatorPluginManager * manager,
					  Window win, Window parent);

extern GtkWidget
    * gkb_indicator_plugin_manager_decorate_widget (GkbIndicatorPluginManager *
						manager,
						GtkWidget * widget,
						const gint group,
						const char
						*group_description,
						GkbKeyboardConfig *
						config);

#endif
