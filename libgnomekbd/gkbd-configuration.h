/*
 * Copyright (C) 2010 Canonical Ltd.
 * Copyright (C) 2010-2011 Sergey V. Udaltsov <svu@gnome.org>
 * 
 * Authors: Jan Arne Petersen <jpetersen@openismus.com>
 *          Sergey V. Udaltsov <svu@gnome.org>
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

#ifndef __GKBD_CONFIGURATION_H__
#define __GKBD_CONFIGURATION_H__

#include <glib-object.h>

#include <libxklavier/xklavier.h>

#include <libgnomekbd/gkbd-indicator-config.h>

G_BEGIN_DECLS typedef struct _GkbdConfiguration GkbdConfiguration;
typedef struct _GkbdConfigurationPrivate GkbdConfigurationPrivate;
typedef struct _GkbdConfigurationClass GkbdConfigurationClass;

#define GKBD_TYPE_CONFIGURATION           (gkbd_configuration_get_type ())
#define GKBD_CONFIGURATION(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GKBD_TYPE_CONFIGURATION, GkbdConfiguration))
#define GKBD_INDCATOR_CLASS(obj)          (G_TYPE_CHECK_CLASS_CAST ((obj), GKBD_TYPE_CONFIGURATION,  GkbdConfigurationClass))
#define GKBD_IS_CONFIGURATION(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GKBD_TYPE_CONFIGURATION))
#define GKBD_IS_CONFIGURATION_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE ((obj), GKBD_TYPE_CONFIGURATION))
#define GKBD_CONFIGURATION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GKBD_TYPE_CONFIGURATION, GkbdConfigurationClass))

struct _GkbdConfiguration {
	GObject parent;

	GkbdConfigurationPrivate *priv;
};

struct _GkbdConfigurationClass {
	GObjectClass parent_class;
};

extern GType gkbd_configuration_get_type (void);

extern GkbdConfiguration *gkbd_configuration_get (void);

extern XklEngine *gkbd_configuration_get_xkl_engine (GkbdConfiguration *
						     configuration);

extern gchar **gkbd_configuration_get_group_names (GkbdConfiguration *
						   configuration);

extern gchar **gkbd_configuration_get_short_group_names (GkbdConfiguration
							 * configuration);

extern gchar *gkbd_configuration_get_image_filename (GkbdConfiguration *
						     configuration,
						     guint group);

extern gchar *gkbd_configuration_get_current_tooltip (GkbdConfiguration *
						      configuration);

extern gboolean gkbd_configuration_if_flags_shown (GkbdConfiguration *
						   configuration);

extern gchar *gkbd_configuration_extract_layout_name (GkbdConfiguration *
						      configuration,
						      int group);

extern void gkbd_configuration_lock_next_group (GkbdConfiguration *
						configuration);

extern void gkbd_configuration_lock_group (GkbdConfiguration *
					   configuration, guint group);

extern guint gkbd_configuration_get_current_group (GkbdConfiguration *
						   configuration);

extern gchar *gkbd_configuration_get_group_name (GkbdConfiguration *
						 configuration,
						 guint group);

extern void gkbd_configuration_start_listen (GkbdConfiguration *
					     configuration);

extern void gkbd_configuration_stop_listen (GkbdConfiguration *
					    configuration);

extern GkbdIndicatorConfig
    * gkbd_configuration_get_indicator_config (GkbdConfiguration *
					       configuration);

extern GkbdKeyboardConfig
    * gkbd_configuration_get_keyboard_config (GkbdConfiguration *
					      configuration);

extern GSList *gkbd_configuration_get_all_objects (GkbdConfiguration *
						   configuration);

extern gboolean gkbd_configuration_if_any_object_exists (GkbdConfiguration
							 * configuration);

extern void gkbd_configuration_append_object (GkbdConfiguration *
					      configuration,
					      GObject * obj);

extern void gkbd_configuration_remove_object (GkbdConfiguration *
					      configuration,
					      GObject * obj);

#define ForAllObjects(config) \
	{ \
		GSList* cur; \
		for (cur = gkbd_configuration_get_all_objects (config); cur != NULL; cur = cur->next) { \
			GObject* gki = (GObject*)cur->data;
#define NextObject() \
		} \
	}

extern GSList *gkbd_configuration_load_images (GkbdConfiguration *
					       configuration);

extern void gkbd_configuration_free_images (GkbdConfiguration *
					    configuration,
					    GSList * images);

extern gchar *gkbd_configuration_create_label_title (int group,
						     GHashTable **
						     ln2cnt_map,
						     gchar * layout_name);

extern gboolean gkbd_configuration_get_caps_lock_state (GkbdConfiguration *
							configuration);
extern gboolean gkbd_configuration_get_num_lock_state (GkbdConfiguration *
						       configuration);
extern gboolean gkbd_configuration_get_scroll_lock_state (GkbdConfiguration
							  * configuration);

G_END_DECLS
#endif
