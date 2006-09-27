#include <config.h>

#include <stdio.h>
#include <X11/Xlib.h>
#include <libxklavier/xklavier.h>
#include <dbus/dbus-glib-bindings.h>

#ifdef HAVE_SETLOCALE
# include <locale.h>
#endif

#include "libgnomekbd/gkbd-config-registry.h"

static GMainLoop *loop;

int
main ()
{
	g_type_init_with_debug_flags (G_TYPE_DEBUG_OBJECTS |
				      G_TYPE_DEBUG_SIGNALS);

#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL, "");
#endif

	GkbdConfigRegistry *reg =
	    GKBD_CONFIG_REGISTRY (g_object_new
				  (gkbd_config_registry_get_type
				   (), NULL));

	loop = g_main_loop_new (NULL, FALSE);

	g_main_loop_run (loop);

	g_object_unref (reg);

	return 0;
}
