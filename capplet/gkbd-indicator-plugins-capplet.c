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

#include <glib/gi18n.h>
#include <gdk/gdkx.h>
#include <glade/glade.h>
#include <libgnomeui/gnome-ui-init.h>
#include <libgnomeui/gnome-help.h>

static GkbdKeyboardConfig initialSysKbdConfig;
static GMainLoop *loop;

extern void
CappletFillActivePluginList (GkbdIndicatorPluginsCapplet * gipc)
{
  GtkWidget *activePlugins = CappletGetGladeWidget (gipc, "activePlugins");
  GtkListStore *activePluginsModel =
    GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (activePlugins)));
  GSList *pluginPathNode = gipc->applet_cfg.enabled_plugins;
  GHashTable *allPluginRecs = gipc->plugin_manager.all_plugin_recs;

  gtk_list_store_clear (activePluginsModel);
  if (allPluginRecs == NULL)
    return;

  while (pluginPathNode != NULL)
    {
      GtkTreeIter iter;
      const char *fullPath = (const char *) pluginPathNode->data;
      const GkbdIndicatorPlugin *plugin =
	gkbd_indicator_plugin_manager_get_plugin (&gipc->plugin_manager,
						  fullPath);
      if (plugin != NULL)
	{
	  gtk_list_store_append (activePluginsModel, &iter);
	  gtk_list_store_set (activePluginsModel, &iter,
			      NAME_COLUMN, plugin->name,
			      FULLPATH_COLUMN, fullPath, -1);
	}

      pluginPathNode = g_slist_next (pluginPathNode);
    }
}

static char *
CappletGetSelectedActivePluginPath (GkbdIndicatorPluginsCapplet * gipc)
{
  GtkTreeView *pluginsList =
    GTK_TREE_VIEW (CappletGetGladeWidget (gipc, "activePlugins"));
  return CappletGetSelectedPluginPath (pluginsList, gipc);
}

char *
CappletGetSelectedPluginPath (GtkTreeView * pluginsList,
			      GkbdIndicatorPluginsCapplet * gipc)
{
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (pluginsList));
  GtkTreeSelection *selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (pluginsList));
  GtkTreeIter selectedIter;

  if (gtk_tree_selection_get_selected (selection, NULL, &selectedIter))
    {
      char *fullPath = NULL;

      gtk_tree_model_get (model, &selectedIter,
			  FULLPATH_COLUMN, &fullPath, -1);
      return fullPath;
    }
  return NULL;
}

static void
CappletActivePluginsSelectionChanged (GtkTreeSelection *
				      selection,
				      GkbdIndicatorPluginsCapplet * gipc)
{
  GtkWidget *activePlugins = CappletGetGladeWidget (gipc, "activePlugins");
  GtkTreeModel *model =
    gtk_tree_view_get_model (GTK_TREE_VIEW (activePlugins));
  GtkTreeIter selectedIter;
  gboolean isAnythingSelected = FALSE;
  gboolean isFirstSelected = FALSE;
  gboolean isLastSelected = FALSE;
  gboolean hasConfigurationUi = FALSE;
  GtkWidget *btnRemove = CappletGetGladeWidget (gipc, "btnRemove");
  GtkWidget *btnUp = CappletGetGladeWidget (gipc, "btnUp");
  GtkWidget *btnDown = CappletGetGladeWidget (gipc, "btnDown");
  GtkWidget *btnProperties = CappletGetGladeWidget (gipc, "btnProperties");
  GtkWidget *lblDescription = CappletGetGladeWidget (gipc, "lblDescription");

  gtk_label_set_text (GTK_LABEL (lblDescription),
		      g_strconcat ("<small><i>",
				   _("No description."),
				   "</i></small>", NULL));
  gtk_label_set_use_markup (GTK_LABEL (lblDescription), TRUE);

  if (gtk_tree_selection_get_selected (selection, NULL, &selectedIter))
    {
      int counter = gtk_tree_model_iter_n_children (model, NULL);
      GtkTreePath *treePath = gtk_tree_model_get_path (model, &selectedIter);
      gint *indices = gtk_tree_path_get_indices (treePath);
      char *fullPath = CappletGetSelectedActivePluginPath (gipc);
      const GkbdIndicatorPlugin *plugin =
	gkbd_indicator_plugin_manager_get_plugin (&gipc->plugin_manager,
						  fullPath);

      isAnythingSelected = TRUE;

      isFirstSelected = indices[0] == 0;
      isLastSelected = indices[0] == counter - 1;

      if (plugin != NULL)
	{
	  hasConfigurationUi =
	    (plugin->configure_properties_callback != NULL);
	  gtk_label_set_text (GTK_LABEL (lblDescription),
			      g_strconcat ("<small><i>",
					   plugin->
					   description,
					   "</i></small>", NULL));
	  gtk_label_set_use_markup (GTK_LABEL (lblDescription), TRUE);
	}
      g_free (fullPath);

      gtk_tree_path_free (treePath);
    }
  gtk_widget_set_sensitive (btnRemove, isAnythingSelected);
  gtk_widget_set_sensitive (btnUp, isAnythingSelected && !isFirstSelected);
  gtk_widget_set_sensitive (btnDown, isAnythingSelected && !isLastSelected);
  gtk_widget_set_sensitive (btnProperties, isAnythingSelected
			    && hasConfigurationUi);
}

static void
CappletPromotePlugin (GtkWidget * btnUp, GkbdIndicatorPluginsCapplet * gipc)
{
  char *fullPath = CappletGetSelectedActivePluginPath (gipc);
  if (fullPath != NULL)
    {
      gkbd_indicator_plugin_manager_promote_plugin (&gipc->
						    plugin_manager,
						    gipc->applet_cfg.
						    enabled_plugins,
						    fullPath);
      g_free (fullPath);
      CappletFillActivePluginList (gipc);
      gkbd_indicator_config_save_to_gconf (&gipc->applet_cfg);
    }
}

static void
CappletDemotePlugin (GtkWidget * btnUp, GkbdIndicatorPluginsCapplet * gipc)
{
  char *fullPath = CappletGetSelectedActivePluginPath (gipc);
  if (fullPath != NULL)
    {
      gkbd_indicator_plugin_manager_demote_plugin (&gipc->
						   plugin_manager,
						   gipc->applet_cfg.
						   enabled_plugins, fullPath);
      g_free (fullPath);
      CappletFillActivePluginList (gipc);
      gkbd_indicator_config_save_to_gconf (&gipc->applet_cfg);
    }
}

static void
CappletDisablePlugin (GtkWidget * btnRemove,
		      GkbdIndicatorPluginsCapplet * gipc)
{
  char *fullPath = CappletGetSelectedActivePluginPath (gipc);
  if (fullPath != NULL)
    {
      gkbd_indicator_plugin_manager_disable_plugin (&gipc->
						    plugin_manager,
						    &gipc->
						    applet_cfg.
						    enabled_plugins,
						    fullPath);
      g_free (fullPath);
      CappletFillActivePluginList (gipc);
      gkbd_indicator_config_save_to_gconf (&gipc->applet_cfg);
    }
}

static void
CappletConfigurePlugin (GtkWidget * btnRemove,
			GkbdIndicatorPluginsCapplet * gipc)
{
  char *fullPath = CappletGetSelectedActivePluginPath (gipc);
  if (fullPath != NULL)
    {
      gkbd_indicator_plugin_manager_configure_plugin (&gipc->
						      plugin_manager,
						      &gipc->
						      plugin_container,
						      fullPath,
						      GTK_WINDOW
						      (gipc->capplet));
      g_free (fullPath);
    }
}

static void
CappletResponse (GtkDialog * dialog, gint response)
{
  if (response == GTK_RESPONSE_HELP)
    {
      GError *error = NULL;
      gnome_help_display_on_screen ("gkbd", "gkb-indicator-applet-plugins",
				    gtk_widget_get_screen (GTK_WIDGET
							   (dialog)), &error);
      return;
    }

  g_main_loop_quit (loop);
}

static void
CappletSetup (GkbdIndicatorPluginsCapplet * gipc)
{
  GladeXML *data;
  GtkWidget *capplet;
  GtkWidget *activePlugins;
  GtkTreeModel *activePluginsModel;
  GtkCellRenderer *renderer =
    GTK_CELL_RENDERER (gtk_cell_renderer_text_new ());
  GtkTreeViewColumn *column =
    gtk_tree_view_column_new_with_attributes (NULL, renderer,
					      "text", 0,
					      NULL);
  GtkTreeSelection *selection;
  glade_gnome_init ();

  gtk_window_set_default_icon_name ("gkbd-indicator_properties-capplet");

  /* default domain! */
  data =
    glade_xml_new (GLADEDIR "/gkbd-indicator-plugins.glade",
		   "gkbd_indicator_plugins", NULL);
  gipc->capplet = capplet =
    glade_xml_get_widget (data, "gkbd_indicator_plugins");

  gtk_object_set_data (GTK_OBJECT (capplet), "gladeData", data);
  g_signal_connect_swapped (GTK_OBJECT (capplet),
			    "destroy", G_CALLBACK (g_object_unref), data);
  g_signal_connect_swapped (G_OBJECT (capplet), "unrealize",
			    G_CALLBACK (g_main_loop_quit), loop);

  g_signal_connect (GTK_OBJECT (capplet),
		    "response", G_CALLBACK (CappletResponse), NULL);

  glade_xml_signal_connect_data (data, "on_btnUp_clicked",
				 GTK_SIGNAL_FUNC
				 (CappletPromotePlugin), gipc);
  glade_xml_signal_connect_data (data,
				 "on_btnDown_clicked",
				 GTK_SIGNAL_FUNC (CappletDemotePlugin), gipc);
  glade_xml_signal_connect_data (data,
				 "on_btnAdd_clicked",
				 GTK_SIGNAL_FUNC (CappletEnablePlugin), gipc);
  glade_xml_signal_connect_data (data,
				 "on_btnRemove_clicked",
				 GTK_SIGNAL_FUNC
				 (CappletDisablePlugin), gipc);
  glade_xml_signal_connect_data (data,
				 "on_btnProperties_clicked",
				 GTK_SIGNAL_FUNC
				 (CappletConfigurePlugin), gipc);

  activePlugins = CappletGetGladeWidget (gipc, "activePlugins");
  activePluginsModel =
    GTK_TREE_MODEL (gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING));
  gtk_tree_view_set_model (GTK_TREE_VIEW (activePlugins), activePluginsModel);
  gtk_tree_view_append_column (GTK_TREE_VIEW (activePlugins), column);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (activePlugins));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (CappletActivePluginsSelectionChanged), gipc);
  CappletFillActivePluginList (gipc);
  CappletActivePluginsSelectionChanged (selection, gipc);
  gtk_widget_show_all (capplet);
}

int
main (int argc, char **argv)
{
  GkbdIndicatorPluginsCapplet gipc;

  GError *gconf_error = NULL;
  GConfClient *confClient;

  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
  memset (&gipc, 0, sizeof (gipc));
  gnome_program_init ("gkbd", VERSION,
		      LIBGNOMEUI_MODULE, argc, argv,
		      GNOME_PROGRAM_STANDARD_PROPERTIES, NULL);
  if (!gconf_init (argc, argv, &gconf_error))
    {
      g_warning (_("Failed to init GConf: %s\n"), gconf_error->message);
      g_error_free (gconf_error);
      return 1;
    }
  gconf_error = NULL;
  /*GkbdIndicatorInstallGlibLogAppender(  ); */
  gipc.engine = xkl_engine_get_instance (GDK_DISPLAY ());
  gipc.config_registry = xkl_config_registry_get_instance (gipc.engine);

  confClient = gconf_client_get_default ();
  gkbd_indicator_plugin_container_init (&gipc.plugin_container, confClient);
  g_object_unref (confClient);

  gkbd_keyboard_config_init (&gipc.kbd_cfg, confClient, gipc.engine);
  gkbd_keyboard_config_init (&initialSysKbdConfig, confClient, gipc.engine);

  gkbd_indicator_config_init (&gipc.applet_cfg, confClient, gipc.engine);

  gkbd_indicator_plugin_manager_init (&gipc.plugin_manager);

  gkbd_keyboard_config_load_from_x_initial (&initialSysKbdConfig);
  gkbd_keyboard_config_load_from_gconf (&gipc.kbd_cfg, &initialSysKbdConfig);

  gkbd_indicator_config_load_from_gconf (&gipc.applet_cfg);

  loop = g_main_loop_new (NULL, TRUE);

  CappletSetup (&gipc);

  g_main_loop_run (loop);

  gkbd_indicator_plugin_manager_term (&gipc.plugin_manager);

  gkbd_indicator_config_term (&gipc.applet_cfg);

  gkbd_keyboard_config_term (&gipc.kbd_cfg);
  gkbd_keyboard_config_term (&initialSysKbdConfig);

  gkbd_indicator_plugin_container_term (&gipc.plugin_container);
  g_object_unref (G_OBJECT (gipc.config_registry));
  g_object_unref (G_OBJECT (gipc.engine));
  return 0;
}

/* functions just for plugins - otherwise ldd is not happy */
void
gkbd_indicator_plugin_container_reinit_ui (GkbdIndicatorPluginContainer * pc)
{
}

gchar **
gkbd_indicator_plugin_load_localized_group_names (GkbdIndicatorPluginContainer
						  * pc)
{
  return
    gkbd_desktop_config_load_group_descriptions_utf8 (&
						      (((GkbdIndicatorPluginsCapplet *) pc)->cfg), (((GkbdIndicatorPluginsCapplet *) pc)->config_registry));
}
