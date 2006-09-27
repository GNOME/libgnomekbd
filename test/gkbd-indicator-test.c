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

#include "config.h"

#include "libxklavier/xklavier.h"
#include "libgnomekbd/gkbd-indicator.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <gdk/gdkscreen.h>
#include <gdk/gdkx.h>
#include <gnome.h>
#include <glade/glade.h>

#include "X11/XKBlib.h"

int
main (int argc, char **argv)
{
	GtkWidget *gki;
	GtkWidget *mainwin;
	GtkWidget *vbox;

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* Different data dir defs in g-a and g-c-c */
	gnome_program_init ("gkbd-indicator-test", VERSION,
			    LIBGNOMEUI_MODULE, argc, argv,
			    GNOME_PARAM_APP_DATADIR, DATADIR, NULL);

	glade_gnome_init ();

	mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gki = gkbd_indicator_new ();
	gkbd_indicator_set_tooltips_format (_
					    ("Keyboard Indicator Test (%s)"));
	gkbd_indicator_set_parent_tooltips (GKBD_INDICATOR (gki), TRUE);

	gtk_window_resize (GTK_WINDOW (mainwin), 250, 250);
	vbox = gtk_vbox_new (TRUE, 6);

	gtk_container_add (GTK_CONTAINER (mainwin), vbox);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
	gtk_container_add (GTK_CONTAINER (vbox),
			   gtk_label_new (_("Indicator:")));
	gtk_container_add (GTK_CONTAINER (vbox), gki);

	gtk_widget_show_all (mainwin);

	g_signal_connect (G_OBJECT (mainwin),
			  "destroy", G_CALLBACK (gtk_main_quit), NULL);


	gtk_main ();

	return 0;
}
