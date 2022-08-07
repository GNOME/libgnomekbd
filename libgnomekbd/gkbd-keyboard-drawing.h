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

#ifndef GKBD_KEYBOARD_DRAWING_H
#define GKBD_KEYBOARD_DRAWING_H 1

#include <gtk/gtk.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBgeom.h>
#include <libxklavier/xklavier.h>

G_BEGIN_DECLS

typedef struct {
	gint group;
	gint level;
} GkbdKeyboardDrawingGroupLevel;

#define GKBD_TYPE_KEYBOARD_DRAWING gkbd_keyboard_drawing_get_type ()
G_DECLARE_DERIVABLE_TYPE (GkbdKeyboardDrawing, gkbd_keyboard_drawing, GKBD, KEYBOARD_DRAWING, GtkDrawingArea)

struct _GkbdKeyboardDrawingClass {
	GtkDrawingAreaClass parent_class;

	/* we send this signal when the user presses a key that "doesn't exist"
	 * according to the keyboard geometry; it probably means their xkb
	 * configuration is incorrect */
	void (*bad_keycode) (GkbdKeyboardDrawing * drawing, guint keycode);
};

GtkWidget *gkbd_keyboard_drawing_new (void);

gboolean gkbd_keyboard_drawing_render (GkbdKeyboardDrawing * kbdrawing,
				       cairo_t * cr,
				       PangoLayout * layout,
				       double x, double y,
				       double width, double height,
				       gdouble dpi_x, gdouble dpi_y);
gboolean gkbd_keyboard_drawing_set_keyboard (GkbdKeyboardDrawing *
					     kbdrawing,
					     XkbComponentNamesRec * names);

void gkbd_keyboard_drawing_set_layout (GkbdKeyboardDrawing * kbdrawing,
				       const gchar * id);

const gchar
    * gkbd_keyboard_drawing_get_keycodes (GkbdKeyboardDrawing * kbdrawing);
const gchar
    * gkbd_keyboard_drawing_get_geometry (GkbdKeyboardDrawing * kbdrawing);
const gchar
    * gkbd_keyboard_drawing_get_symbols (GkbdKeyboardDrawing * kbdrawing);
const gchar *gkbd_keyboard_drawing_get_types (GkbdKeyboardDrawing
						       * kbdrawing);
const gchar *gkbd_keyboard_drawing_get_compat (GkbdKeyboardDrawing
							* kbdrawing);

void gkbd_keyboard_drawing_set_track_modifiers (GkbdKeyboardDrawing *
						kbdrawing,
						gboolean enable);
void gkbd_keyboard_drawing_set_track_config (GkbdKeyboardDrawing *
					     kbdrawing, gboolean enable);

void gkbd_keyboard_drawing_set_groups_levels (GkbdKeyboardDrawing *
					      kbdrawing,
					      GkbdKeyboardDrawingGroupLevel
					      * groupLevels[]);


void gkbd_keyboard_drawing_print (GkbdKeyboardDrawing * drawing,
				  GtkWindow * parent_window,
				  const gchar * description);

G_END_DECLS
#endif				/* #ifndef GKBD_KEYBOARD_DRAWING_H */
