/* $Id$ */
/*
 * kbdraw.c: main program file for kbdraw
 *
 * Copyright (c) 2003 Noah Levitt
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>
#include <popt.h>
#include <stdlib.h>
#include <string.h>
#include "libgnomekbd/gkbd-keyboard-drawing.h"


static gchar *groups = NULL;
static gchar *levels = NULL;
static gchar *symbols = NULL;
static gchar *keycodes = NULL;
static gchar *geometry = NULL;
static struct poptOption options[] = {
	{"groups", '\0', POPT_ARG_STRING, &groups, 0,
	 "Keyboard groups to display, from 1-4. Up to four groups only may be displayed. Examples: --groups=3 or --groups=1,2,1,2",
	 "group1[,group2[,group3[,group4]]]"},
	{"levels", '\0', POPT_ARG_STRING, &levels, 0,
	 "Keyboard shift levels to display, from 1-64. Up to four shift levels only may be displayed. Examples: --levels=3 or --levels=1,2,1,2",
	 "level1[,level2[,level3[,level4]]]"},
	{"symbols", '\0', POPT_ARG_STRING, &symbols, 0,
	 "Symbols component of the keyboard. If you omit this option, it is obtained from the X server; that is, the keyboard that is currently configured is drawn. Examples: --symbols=us or --symbols=us(pc104)+iso9995-3+group(switch)+ctrl(nocaps)",
	 NULL},
	{"keycodes", '\0', POPT_ARG_STRING, &keycodes, 0,
	 "Keycodes component of the keyboard. If you omit this option, it is obtained from the X server; that is, the keyboard that is currently configured is drawn. Examples: --keycodes=xfree86+aliases(qwerty)",
	 NULL},
	{"geometry", '\0', POPT_ARG_STRING, &geometry, 0,
	 "Geometry xkb component. If you omit this option, it is obtained from the X server; that is, the keyboard that is currently configured is drawn. Example: --geometry=kinesis",
	 NULL},
	{"track-modifiers", '\0', POPT_ARG_NONE, NULL, 3,
	 "Track the current modifiers", NULL},
	{"track-config", '\0', POPT_ARG_NONE, NULL, 4,
	 "Track the server XKB configuration", NULL},
	{"version", 'v', POPT_ARG_NONE, NULL, 1, "Show current version",
	 NULL},
	POPT_AUTOHELP {NULL, '\0', 0, NULL, 0}
};


static gboolean
set_groups (gchar * groups_option,
	    GkbdKeyboardDrawingGroupLevel * groupLevels)
{
	GkbdKeyboardDrawingGroupLevel *pgl = groupLevels;
	gint cntr, g;

	groupLevels[0].group =
	    groupLevels[1].group =
	    groupLevels[2].group = groupLevels[3].group = -1;

	if (groups_option == NULL)
		return TRUE;

	for (cntr = 4; --cntr >= 0;) {
		if (*groups_option == '\0')
			return FALSE;

		g = *groups_option - '1';
		if (g < 0 || g >= 4)
			return FALSE;

		pgl->group = g;
		/* printf ("group %d\n", pgl->group); */

		groups_option++;
		if (*groups_option == '\0')
			return TRUE;
		if (*groups_option != ',')
			return FALSE;

		groups_option++;
		pgl++;
	}

	return TRUE;
}

static gboolean
set_levels (gchar * levels_option,
	    GkbdKeyboardDrawingGroupLevel * groupLevels)
{
	GkbdKeyboardDrawingGroupLevel *pgl = groupLevels;
	gint cntr, l;
	gchar *p;

	groupLevels[0].level =
	    groupLevels[1].level =
	    groupLevels[2].level = groupLevels[3].level = -1;

	if (levels_option == NULL)
		return TRUE;

	for (cntr = 4; --cntr >= 0;) {
		if (*levels_option == '\0')
			return FALSE;

		l = (gint) strtol (levels_option, &p, 10) - 1;
		if (l < 0 || l >= 64)
			return FALSE;

		pgl->level = l;
		/* printf ("level %d\n", pgl->level); */

		levels_option = p;
		if (*levels_option == '\0')
			return TRUE;
		if (*levels_option != ',')
			return FALSE;

		levels_option++;
		pgl++;
	}

	return TRUE;
}

static void
bad_keycode (GkbdKeyboardDrawing * drawing, guint keycode)
{
	g_warning
	    ("got keycode %u, which is not on your keyboard according to your configuration",
	     keycode);
}

gint
main (gint argc, gchar ** argv)
{
	GtkWidget *window;
	GtkWidget *gkbd_keyboard_drawing;
	GdkScreen *screen;
	gint monitor;
	GdkRectangle rect;
	poptContext popt_context;
	gint rc;
	GkbdKeyboardDrawingGroupLevel groupLevels[4] =
	    { {0, 0}, {1, 0}, {0, 1}, {1, 1} };
	GkbdKeyboardDrawingGroupLevel *pgroupLevels[4] =
	    { &groupLevels[0], &groupLevels[1], &groupLevels[2],
		&groupLevels[3]
	};
	gboolean track_config = False, track_modifiers = False;

	gtk_init (&argc, &argv);

	popt_context =
	    poptGetContext ("kbdraw", argc, (const gchar **) argv, options,
			    0);

	for (rc = poptGetNextOpt (popt_context); rc > 0;
	     rc = poptGetNextOpt (popt_context))
		switch (rc) {
		case 1:
			g_print ("kbdraw %s\n", VERSION);
			exit (0);
		case 3:
			track_modifiers = True;
			break;
		case 4:
			track_config = True;
			break;
		}

	if (rc != -1) {
		g_printerr ("%s: %s\n",
			    poptBadOption (popt_context,
					   POPT_BADOPTION_NOALIAS),
			    poptStrerror (rc));
		exit (1);
	}

	if (!set_groups (groups, groupLevels)) {
		g_printerr ("--groups: invalid argument\n");
		exit (1);
	}

	if (!set_levels (levels, groupLevels)) {
		g_printerr ("--levels: invalid argument\n");
		exit (1);
	}

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (G_OBJECT (window), "destroy",
			  G_CALLBACK (gtk_main_quit), NULL);

	screen = gtk_window_get_screen (GTK_WINDOW (window));
	monitor = gdk_screen_get_monitor_at_point (screen, 0, 0);
	gdk_screen_get_monitor_geometry (screen, monitor, &rect);
	gtk_window_set_default_size (GTK_WINDOW (window),
				     rect.width * 4 / 5,
				     rect.height * 1 / 2);

	gtk_widget_show (window);

	gkbd_keyboard_drawing = gkbd_keyboard_drawing_new ();
	gtk_widget_show (gkbd_keyboard_drawing);
	gtk_container_add (GTK_CONTAINER (window), gkbd_keyboard_drawing);

	gkbd_keyboard_drawing_set_groups_levels (GKBD_KEYBOARD_DRAWING
						 (gkbd_keyboard_drawing),
						 pgroupLevels);

	if (track_modifiers)
		gkbd_keyboard_drawing_set_track_modifiers
		    (GKBD_KEYBOARD_DRAWING (gkbd_keyboard_drawing), TRUE);
	if (track_config)
		gkbd_keyboard_drawing_set_track_config
		    (GKBD_KEYBOARD_DRAWING (gkbd_keyboard_drawing), TRUE);
	g_signal_connect (G_OBJECT (gkbd_keyboard_drawing), "bad-keycode",
			  G_CALLBACK (bad_keycode), NULL);

	if (symbols || geometry || keycodes) {
		XkbComponentNamesRec names;
		gint success;

		memset (&names, '\0', sizeof (names));

		if (symbols)
			names.symbols = symbols;
		else
			names.symbols = (gchar *)
			    gkbd_keyboard_drawing_get_symbols
			    (GKBD_KEYBOARD_DRAWING
			     (gkbd_keyboard_drawing));

		if (keycodes)
			names.keycodes = keycodes;
		else
			names.keycodes = (gchar *)
			    gkbd_keyboard_drawing_get_keycodes
			    (GKBD_KEYBOARD_DRAWING
			     (gkbd_keyboard_drawing));

		if (geometry)
			names.geometry = geometry;
		else
			names.geometry = (gchar *)
			    gkbd_keyboard_drawing_get_geometry
			    (GKBD_KEYBOARD_DRAWING
			     (gkbd_keyboard_drawing));

		success =
		    gkbd_keyboard_drawing_set_keyboard
		    (GKBD_KEYBOARD_DRAWING (gkbd_keyboard_drawing),
		     &names);
		if (!success) {
			g_printerr
			    ("\nError loading new keyboard description with components:\n\n"
			     "  keycodes:  %s\n" "  types:     %s\n"
			     "  compat:    %s\n" "  symbols:   %s\n"
			     "  geometry:  %s\n\n", names.keycodes,
			     names.types, names.compat, names.symbols,
			     names.geometry);
			exit (1);
		}
	}

	gtk_widget_grab_focus (gkbd_keyboard_drawing);

	gtk_main ();

	return 0;
}
