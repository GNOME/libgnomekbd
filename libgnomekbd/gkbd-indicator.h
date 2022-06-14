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

#ifndef __GKBD_INDICATOR_H__
#define __GKBD_INDICATOR_H__

#include <gtk/gtk.h>

#include <libxklavier/xklavier.h>

G_BEGIN_DECLS typedef struct _GkbdIndicator GkbdIndicator;
typedef struct _GkbdIndicatorPrivate GkbdIndicatorPrivate;
typedef struct _GkbdIndicatorClass GkbdIndicatorClass;

#define GKBD_TYPE_INDICATOR             (gkbd_indicator_get_type ())
#define GKBD_INDICATOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GKBD_TYPE_INDICATOR, GkbdIndicator))
#define GKBD_INDICATOR_CLASS(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), GKBD_TYPE_INDICATOR,  GkbdIndicatorClass))
#define GKBD_IS_INDICATOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GKBD_TYPE_INDICATOR))
#define GKBD_IS_INDICATOR_CLASS(obj)    (G_TYPE_CHECK_CLASS_TYPE ((obj), GKBD_TYPE_INDICATOR))
#define GKBD_INDICATOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GKBD_TYPE_INDICATOR, GkbdIndicatorClass))

struct _GkbdIndicator {
	GtkNotebook parent;
	GkbdIndicatorPrivate *priv;
};

struct _GkbdIndicatorClass {
	GtkNotebookClass parent_class;

	void (*reinit_ui) (GkbdIndicator * gki);
};

GType gkbd_indicator_get_type (void);

GtkWidget *gkbd_indicator_new (void);

void gkbd_indicator_reinit_ui (GkbdIndicator * gki);

void gkbd_indicator_set_angle (GkbdIndicator * gki, gdouble angle);

XklEngine *gkbd_indicator_get_xkl_engine (void);

gchar **gkbd_indicator_get_group_names (void);

gchar *gkbd_indicator_get_image_filename (guint group);

gdouble gkbd_indicator_get_max_width_height_ratio (void);

void
 gkbd_indicator_set_parent_tooltips (GkbdIndicator * gki, gboolean ifset);

G_END_DECLS
#endif
