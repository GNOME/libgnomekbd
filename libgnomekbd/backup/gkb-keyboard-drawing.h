/* $Id$ */
/*
 * keyboard-drawing.h: header file for a gtk+ widget that is a drawing of
 * the keyboard of the default display
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

#ifndef GKB_KEYBOARD_DRAWING_H
#define GKB_KEYBOARD_DRAWING_H 1

#include <gtk/gtk.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBgeom.h>

G_BEGIN_DECLS
#define GKB_KEYBOARD_DRAWING(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), gkb_keyboard_drawing_get_type (), \
                               GkbKeyboardDrawing))
#define GKB_KEYBOARD_DRAWING_CLASS(clazz) (G_TYPE_CHECK_CLASS_CAST ((clazz), gkb_keyboard_drawing_get_type () \
                                       GkbKeyboardDrawingClass))
#define GKB_IS_KEYBOARD_DRAWING(obj) G_TYPE_CHECK_INSTANCE_TYPE ((obj), gkb_keyboard_drawing_get_type ())
typedef struct _GkbKeyboardDrawing GkbKeyboardDrawing;
typedef struct _GkbKeyboardDrawingClass GkbKeyboardDrawingClass;

typedef struct _GkbKeyboardDrawingItem GkbKeyboardDrawingItem;
typedef struct _GkbKeyboardDrawingKey GkbKeyboardDrawingKey;
typedef struct _GkbKeyboardDrawingDoodad GkbKeyboardDrawingDoodad;
typedef struct _GkbKeyboardDrawingGroupLevel GkbKeyboardDrawingGroupLevel;

typedef enum {
	GKB_KEYBOARD_DRAWING_ITEM_TYPE_KEY,
	GKB_KEYBOARD_DRAWING_ITEM_TYPE_DOODAD
} GkbKeyboardDrawingItemType;

typedef enum {
	GKB_KEYBOARD_DRAWING_POS_TOPLEFT,
	GKB_KEYBOARD_DRAWING_POS_TOPRIGHT,
	GKB_KEYBOARD_DRAWING_POS_BOTTOMLEFT,
	GKB_KEYBOARD_DRAWING_POS_BOTTOMRIGHT,
	GKB_KEYBOARD_DRAWING_POS_TOTAL,
	GKB_KEYBOARD_DRAWING_POS_FIRST = GKB_KEYBOARD_DRAWING_POS_TOPLEFT,
	GKB_KEYBOARD_DRAWING_POS_LAST =
	    GKB_KEYBOARD_DRAWING_POS_BOTTOMRIGHT,
} GkbKeyboardDrawingGroupLevelPosition;

/* units are in xkb form */
struct _GkbKeyboardDrawingItem {
	/*< private > */

	GkbKeyboardDrawingItemType type;
	gint origin_x;
	gint origin_y;
	gint angle;
	guint priority;
};

/* units are in xkb form */
struct _GkbKeyboardDrawingKey {
	/*< private > */

	GkbKeyboardDrawingItemType type;
	gint origin_x;
	gint origin_y;
	gint angle;
	guint priority;

	XkbKeyRec *xkbkey;
	gboolean pressed;
	guint keycode;
};

/* units are in xkb form */
struct _GkbKeyboardDrawingDoodad {
	/*< private > */

	GkbKeyboardDrawingItemType type;
	gint origin_x;
	gint origin_y;
	gint angle;
	guint priority;

	XkbDoodadRec *doodad;
	gboolean on;		/* for indicator doodads */
};

struct _GkbKeyboardDrawingGroupLevel {
	gint group;
	gint level;
};

struct _GkbKeyboardDrawing {
	/*< private > */

	GtkDrawingArea parent;

	GdkPixmap *pixmap;
	XkbDescRec *xkb;
	gboolean xkbOnDisplay;

	gint angle;		/* current angle pango is set to draw at, in tenths of a degree */
	PangoLayout *layout;
	PangoFontDescription *font_desc;

	gint scale_numerator;
	gint scale_denominator;

	GkbKeyboardDrawingKey *keys;

	/* list of stuff to draw in priority order */
	GList *keyboard_items;

	GdkColor *colors;

	guint timeout;

	GkbKeyboardDrawingGroupLevel **groupLevels;

	guint mods;

	Display *display;
	gint screen_num;

	gint xkb_event_type;

	GkbKeyboardDrawingDoodad **physical_indicators;
	gint physical_indicators_size;

	guint track_config:1;
	guint track_modifiers:1;
};

struct _GkbKeyboardDrawingClass {
	GtkDrawingAreaClass parent_class;

	/* we send this signal when the user presses a key that "doesn't exist"
	 * according to the keyboard geometry; it probably means their xkb
	 * configuration is incorrect */
	void (*bad_keycode) (GkbKeyboardDrawing * drawing, guint keycode);
};

GType gkb_keyboard_drawing_get_type (void);
GtkWidget *gkb_keyboard_drawing_new (void);

GdkPixbuf *gkb_keyboard_drawing_get_pixbuf (GkbKeyboardDrawing *
					    kbdrawing);
gboolean gkb_keyboard_drawing_set_keyboard (GkbKeyboardDrawing * kbdrawing,
					    XkbComponentNamesRec * names);

G_CONST_RETURN gchar *gkb_keyboard_drawing_get_keycodes (GkbKeyboardDrawing
							 * kbdrawing);
G_CONST_RETURN gchar *gkb_keyboard_drawing_get_geometry (GkbKeyboardDrawing
							 * kbdrawing);
G_CONST_RETURN gchar *gkb_keyboard_drawing_get_symbols (GkbKeyboardDrawing
							* kbdrawing);
G_CONST_RETURN gchar *gkb_keyboard_drawing_get_types (GkbKeyboardDrawing *
						      kbdrawing);
G_CONST_RETURN gchar *gkb_keyboard_drawing_get_compat (GkbKeyboardDrawing *
						       kbdrawing);

void gkb_keyboard_drawing_set_track_modifiers (GkbKeyboardDrawing *
					       kbdrawing, gboolean enable);
void gkb_keyboard_drawing_set_track_config (GkbKeyboardDrawing * kbdrawing,
					    gboolean enable);

void gkb_keyboard_drawing_set_groups_levels (GkbKeyboardDrawing *
					     kbdrawing,
					     GkbKeyboardDrawingGroupLevel *
					     groupLevels[]);

G_END_DECLS
#endif				/* #ifndef GKB_KEYBOARD_DRAWING_H */
