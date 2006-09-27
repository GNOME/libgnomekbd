#ifndef __GKB_CONFIG_REGISTRY_H__
#define __GKB_CONFIG_REGISTRY_H__

#include <dbus/dbus-glib-bindings.h>
#include <libxklavier/xklavier.h>

typedef struct GkbConfigRegistry GkbConfigRegistry;
typedef struct GkbConfigRegistryClass GkbConfigRegistryClass;

struct GkbConfigRegistry {
	GObject parent;

	XklEngine *engine;
	XklConfigRegistry *registry;
};

struct GkbConfigRegistryClass {
	GObjectClass parent;
	DBusGConnection *connection;
};

#define GKB_CONFIG_TYPE_REGISTRY              (gkb_config_registry_get_type ())
#define GKB_CONFIG_REGISTRY(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), GKB_CONFIG_TYPE_REGISTRY, GkbConfigRegistry))
#define GKB_CONFIG_REGISTRY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GKB_CONFIG_TYPE_REGISTRY, GkbConfigRegistryClass))
#define GKB_IS_CONFIG_REGISTRY(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), GKB_CONFIG_TYPE_REGISTRY))
#define GKB_IS_CONFIG_REGISTRY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GKB_CONFIG_TYPE_REGISTRY))
#define GKB_CONFIG_REGISTRY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GKB_CONFIG_TYPE_REGISTRY, GkbConfigRegistryClass))


/**
 * DBUS server
 */

extern GType gkb_config_registry_get_type (void);

extern gboolean
    gkb_config_registry_get_current_descriptions_as_utf8
    (GkbConfigRegistry * registry,
     gchar *** short_layout_descriptions,
     gchar *** long_layout_descriptions,
     gchar *** short_variant_descriptions,
     gchar *** long_variant_descriptions, GError ** error);

#endif
