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

#ifndef GKBD_KEYBOARD_DRAWING_PRIVATE_H
#define GKBD_KEYBOARD_DRAWING_PRIVATE_H

#include <glib.h>
#include <cairo.h>
#include <gdk/gdk.h>
#include <pango/pango.h>
#include <X11/extensions/XKBgeom.h>

G_BEGIN_DECLS

typedef enum {
	GKBD_KEYBOARD_DRAWING_ITEM_TYPE_INVALID = 0,
	GKBD_KEYBOARD_DRAWING_ITEM_TYPE_KEY,
	GKBD_KEYBOARD_DRAWING_ITEM_TYPE_KEY_EXTRA,
	GKBD_KEYBOARD_DRAWING_ITEM_TYPE_DOODAD
} GkbdKeyboardDrawingItemType;

typedef enum {
	GKBD_KEYBOARD_DRAWING_POS_TOPLEFT,
	GKBD_KEYBOARD_DRAWING_POS_TOPRIGHT,
	GKBD_KEYBOARD_DRAWING_POS_BOTTOMLEFT,
	GKBD_KEYBOARD_DRAWING_POS_BOTTOMRIGHT,
	GKBD_KEYBOARD_DRAWING_POS_TOTAL,
	GKBD_KEYBOARD_DRAWING_POS_FIRST =
	    GKBD_KEYBOARD_DRAWING_POS_TOPLEFT,
	GKBD_KEYBOARD_DRAWING_POS_LAST =
	    GKBD_KEYBOARD_DRAWING_POS_BOTTOMRIGHT
} GkbdKeyboardDrawingGroupLevelPosition;

/* units are in xkb form */
typedef struct {
	GkbdKeyboardDrawingItemType type;
	gint origin_x;
	gint origin_y;
	gint angle;
	guint priority;
} GkbdKeyboardDrawingItem;

/* units are in xkb form */
typedef struct {
	GkbdKeyboardDrawingItemType type;
	gint origin_x;
	gint origin_y;
	gint angle;
	guint priority;

	XkbKeyRec *xkbkey;
	gboolean pressed;
	guint keycode;
} GkbdKeyboardDrawingKey;

/* units are in xkb form */
typedef struct {
	GkbdKeyboardDrawingItemType type;
	gint origin_x;
	gint origin_y;
	gint angle;
	guint priority;

	XkbDoodadRec *doodad;
	gboolean on;		/* for indicator doodads */
} GkbdKeyboardDrawingDoodad;

typedef struct {
	cairo_t *cr;

	gint angle;		/* current angle pango is set to draw at, in tenths of a degree */
	PangoLayout *layout;
	PangoFontDescription *font_desc;

	gint scale_numerator;
	gint scale_denominator;

	GdkRGBA dark_color;
} GkbdKeyboardDrawingRenderContext;

G_END_DECLS

#endif /* GKBD_KEYBOARD_DRAWING_PRIVATE_H */
