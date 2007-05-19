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

#include <X11/Xlib.h>

#include <gkbd-config-registry.h>
#include <gkbd-config-registry-server.h>
#include <gkbd-keyboard-config.h>

static GObjectClass *parent_class = NULL;

gboolean
    gkbd_config_registry_get_descriptions_as_utf8
    (GkbdConfigRegistry * registry,
     gchar ** layout_ids,
     gchar ** variant_ids,
     gchar *** short_layout_descriptions,
     gchar *** long_layout_descriptions,
     gchar *** short_variant_descriptions,
     gchar *** long_variant_descriptions, GError ** error) {
	char **pl, **pv;
	guint total_layouts;
	gchar **sld, **lld, **svd, **lvd;
	XklConfigItem *item = xkl_config_item_new ();

	if (!registry->registry) {
		registry->registry =
		    xkl_config_registry_get_instance (registry->engine);

		xkl_config_registry_load (registry->registry);
	}

	if (!
	    (xkl_engine_get_features (registry->engine) &
	     XKLF_MULTIPLE_LAYOUTS_SUPPORTED))
		return FALSE;

	pl = layout_ids;
	pv = variant_ids;
	total_layouts = g_strv_length (layout_ids);
	sld = *short_layout_descriptions =
	    g_new0 (char *, total_layouts + 1);
	lld = *long_layout_descriptions =
	    g_new0 (char *, total_layouts + 1);
	svd = *short_variant_descriptions =
	    g_new0 (char *, total_layouts + 1);
	lvd = *long_variant_descriptions =
	    g_new0 (char *, total_layouts + 1);

	while (pl != NULL && *pl != NULL) {

		xkl_debug (100, "ids: [%s][%s]\n", *pl,
			   pv == NULL ? NULL : *pv);

		g_snprintf (item->name, sizeof item->name, "%s", *pl);
		if (xkl_config_registry_find_layout
		    (registry->registry, item)) {
			*sld = g_strdup (item->short_description);
			*lld = g_strdup (item->description);
		} else {
			*sld = g_strdup ("");
			*lld = g_strdup ("");
		}

		if (*pv != NULL) {
			g_snprintf (item->name, sizeof item->name, "%s",
				    *pv);
			if (xkl_config_registry_find_variant
			    (registry->registry, *pl, item)) {
				*svd = g_strdup (item->short_description);
				*lvd = g_strdup (item->description);
			} else {
				*svd = g_strdup ("");
				*lvd = g_strdup ("");
			}
		} else {
			*svd = g_strdup ("");
			*lvd = g_strdup ("");
		}

		xkl_debug (100, "description: [%s][%s][%s][%s]\n",
			   *sld, *lld, *svd, *lvd);
		sld++;
		lld++;
		svd++;
		lvd++;

		pl++;

		if (*pv != NULL)
			pv++;
	}

	g_object_unref (item);
	return TRUE;
}

G_DEFINE_TYPE (GkbdConfigRegistry, gkbd_config_registry, G_TYPE_OBJECT)
static void
finalize (GObject * object)
{
	GkbdConfigRegistry *registry;

	registry = GKBD_CONFIG_REGISTRY (object);
	if (registry->registry == NULL) {
		return;
	}

	g_object_unref (registry->registry);
	registry->registry = NULL;

	g_object_unref (registry->engine);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gkbd_config_registry_class_init (GkbdConfigRegistryClass * klass)
{
	GError *error = NULL;
	GObjectClass *object_class;

	/* Init the DBus connection, per-klass */
	klass->connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (klass->connection == NULL) {
		g_warning ("Unable to connect to dbus: %s",
			   error->message);
		g_error_free (error);
		return;
	}

	dbus_g_object_type_install_info (GKBD_CONFIG_TYPE_REGISTRY,
					 &dbus_glib_gkbd_config_registry_object_info);

	object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = finalize;

	parent_class = g_type_class_peek_parent (klass);
}

static void
gkbd_config_registry_init (GkbdConfigRegistry * registry)
{
	GError *error = NULL;
	DBusGProxy *driver_proxy;
	GkbdConfigRegistryClass *klass =
	    GKBD_CONFIG_REGISTRY_GET_CLASS (registry);
	unsigned request_ret;

	/* Register DBUS path */
	dbus_g_connection_register_g_object (klass->connection,
					     "/org/gnome/GkbdConfigRegistry",
					     G_OBJECT (registry));

	/* Register the service name, the constant here are defined in dbus-glib-bindings.h */
	driver_proxy = dbus_g_proxy_new_for_name (klass->connection,
						  DBUS_SERVICE_DBUS,
						  DBUS_PATH_DBUS,
						  DBUS_INTERFACE_DBUS);

	if (driver_proxy != NULL) {
		if (!org_freedesktop_DBus_request_name
		    (driver_proxy, "org.gnome.GkbdConfigRegistry", 0,
		     &request_ret, &error)) {
			g_warning ("Unable to register service: %s",
				   error->message);
			g_error_free (error);
		}
		g_object_unref (driver_proxy);
	} else
		g_critical ("Could not create DBUS proxy");

	/* Init libxklavier stuff */
	registry->engine = xkl_engine_get_instance (XOpenDisplay (NULL));
	/* Lazy initialization */
	registry->registry = NULL;
}
