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

#include "gkbd-indicator-plugins-capplet.h"

#include <string.h>
#include <sys/stat.h>

#include <gdk/gdkx.h>
#include <glade/glade.h>
#include <libbonobo.h>

#include <libxklavier/xklavier.h>

static void
CappletAddAvailablePluginFunc (const char *fullPath,
			       GkbdIndicatorPluginManagerRecord * rec,
			       GkbdIndicatorPluginsCapplet * gipc)
{
  GtkListStore *availablePluginsModel;
  GtkTreeIter iter;
  const GkbdIndicatorPlugin *plugin = rec->plugin;

  if (NULL !=
      g_slist_find_custom (gipc->applet_cfg.enabled_plugins,
			   fullPath, (GCompareFunc) strcmp))
    return;

  availablePluginsModel =
    GTK_LIST_STORE (g_object_get_data (G_OBJECT (gipc->capplet),
				       "gkbd_indicator_plugins_add.availablePluginsModel"));
  if (availablePluginsModel == NULL)
    return;

  if (plugin != NULL)
    {
      gtk_list_store_append (availablePluginsModel, &iter);
      gtk_list_store_set (availablePluginsModel, &iter,
			  NAME_COLUMN, plugin->name,
			  FULLPATH_COLUMN, fullPath, -1);
    }
}

static void
CappletFillAvailablePluginList (GtkTreeView *
				availablePluginsList,
				GkbdIndicatorPluginsCapplet * gipc)
{
  GtkListStore *availablePluginsModel =
    GTK_LIST_STORE (gtk_tree_view_get_model
		    (GTK_TREE_VIEW (availablePluginsList)));
  GSList *pluginPathNode = gipc->applet_cfg.enabled_plugins;
  GHashTable *allPluginRecs = gipc->plugin_manager.all_plugin_recs;

  gtk_list_store_clear (availablePluginsModel);
  if (allPluginRecs == NULL)
    return;

  g_object_set_data (G_OBJECT (gipc->capplet),
		     "gkbd_indicator_plugins_add.availablePluginsModel",
		     availablePluginsModel);
  g_hash_table_foreach (allPluginRecs,
			(GHFunc) CappletAddAvailablePluginFunc, gipc);
  g_object_set_data (G_OBJECT (gipc->capplet),
		     "gkbd_indicator_plugins_add.availablePluginsModel",
		     NULL);
  pluginPathNode = g_slist_next (pluginPathNode);
}

static void
CappletAvailablePluginsSelectionChanged (GtkTreeSelection *
					 selection,
					 GkbdIndicatorPluginsCapplet * gipc)
{
  GtkWidget *availablePluginsList =
    GTK_WIDGET (gtk_tree_selection_get_tree_view (selection));
  gboolean isAnythingSelected = FALSE;
  GtkWidget *lblDescription =
    GTK_WIDGET (g_object_get_data (G_OBJECT (gipc->capplet),
				   "gkbd_indicator_plugins_add.lblDescription"));

  char *fullPath =
    CappletGetSelectedPluginPath (GTK_TREE_VIEW (availablePluginsList),
				  gipc);
  isAnythingSelected = fullPath != NULL;
  gtk_label_set_text (GTK_LABEL (lblDescription),
		      g_strconcat ("<small><i>",
				   _("No description."),
				   "</i></small>", NULL));
  gtk_label_set_use_markup (GTK_LABEL (lblDescription), TRUE);

  if (fullPath != NULL)
    {
      const GkbdIndicatorPlugin *plugin =
	gkbd_indicator_plugin_manager_get_plugin (&gipc->plugin_manager,
						  fullPath);
      if (plugin != NULL && plugin->description != NULL)
	gtk_label_set_text (GTK_LABEL (lblDescription),
			    g_strconcat ("<small><i>",
					 plugin->
					 description, "</i></small>", NULL));
      gtk_label_set_use_markup (GTK_LABEL (lblDescription), TRUE);
    }
  gtk_widget_set_sensitive (GTK_WIDGET
			    (g_object_get_data
			     (G_OBJECT (gipc->capplet),
			      "gkbd_indicator_plugins_add.btnOK")),
			    isAnythingSelected);
}

void
CappletEnablePlugin (GtkWidget * btnAdd, GkbdIndicatorPluginsCapplet * gipc)
{
  /* default domain! */
  GladeXML *data = glade_xml_new (GLADEDIR "/gkbd-indicator-plugins.glade",
				  "gkbd_indicator_plugins_add", NULL);
  GtkWidget *popup =
    glade_xml_get_widget (data, "gkbd_indicator_plugins_add");
  GtkWidget *availablePluginsList;
  GtkTreeModel *availablePluginsModel;
  GtkCellRenderer *renderer =
    GTK_CELL_RENDERER (gtk_cell_renderer_text_new ());
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (NULL,
									renderer,
									"text",
									0,
									NULL);
  GtkTreeSelection *selection;
  gint response;
  availablePluginsList = glade_xml_get_widget (data, "allPlugins");
  availablePluginsModel =
    GTK_TREE_MODEL (gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING));
  gtk_tree_view_set_model (GTK_TREE_VIEW (availablePluginsList),
			   availablePluginsModel);
  gtk_tree_view_append_column (GTK_TREE_VIEW (availablePluginsList), column);
  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (availablePluginsList));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  CappletFillAvailablePluginList (GTK_TREE_VIEW (availablePluginsList), gipc);
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK
		    (CappletAvailablePluginsSelectionChanged), gipc);
  g_object_set_data (G_OBJECT (gipc->capplet),
		     "gkbd_indicator_plugins_add.btnOK",
		     glade_xml_get_widget (data, "btnOK"));
  g_object_set_data (G_OBJECT (gipc->capplet),
		     "gkbd_indicator_plugins_add.lblDescription",
		     glade_xml_get_widget (data, "lblDescription"));
  CappletAvailablePluginsSelectionChanged (selection, gipc);
  response = gtk_dialog_run (GTK_DIALOG (popup));
  g_object_set_data (G_OBJECT (gipc->capplet),
		     "gkbd_indicator_plugins_add.lblDescription", NULL);
  g_object_set_data (G_OBJECT (gipc->capplet),
		     "gkbd_indicator_plugins_add.btnOK", NULL);
  gtk_widget_hide_all (popup);
  if (response == GTK_RESPONSE_OK)
    {
      char *fullPath =
	CappletGetSelectedPluginPath (GTK_TREE_VIEW (availablePluginsList),
				      gipc);
      if (fullPath != NULL)
	{
	  gkbd_indicator_plugin_manager_enable_plugin (&gipc->
						       plugin_manager,
						       &gipc->
						       applet_cfg.
						       enabled_plugins,
						       fullPath);
	  CappletFillActivePluginList (gipc);
	  g_free (fullPath);
	  gkbd_indicator_config_save_to_gconf (&gipc->applet_cfg);
	}
    }
  gtk_widget_destroy (popup);
}
