/*
 * Copyright (C) 2009 Sergey V. Udaltsov <svu@gnome.org>
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

#ifndef __GKBD_STATUS_H__
#define __GKBD_STATUS_H__

#include <gtk/gtk.h>

#include <libxklavier/xklavier.h>

G_BEGIN_DECLS typedef struct _GkbdStatus GkbdStatus;
typedef struct _GkbdStatusPrivate GkbdStatusPrivate;
typedef struct _GkbdStatusClass GkbdStatusClass;

#define GKBD_TYPE_STATUS             (gkbd_status_get_type ())
#define GKBD_STATUS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GKBD_TYPE_STATUS, GkbdStatus))
#define GKBD_STATUS_CLASS(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), GKBD_TYPE_STATUS,  GkbdStatusClass))
#define GKBD_IS_STATUS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GKBD_TYPE_STATUS))
#define GKBD_IS_STATUS_CLASS(obj)    (G_TYPE_CHECK_CLASS_TYPE ((obj), GKBD_TYPE_STATUS))
#define GKBD_STATUS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GKBD_TYPE_STATUS, GkbdStatusClass))

struct _GkbdStatus {
	GtkStatusIcon parent;
	GkbdStatusPrivate *priv;
};

struct _GkbdStatusClass {
	GtkStatusIconClass parent_class;
};

GType gkbd_status_get_type (void);

GtkStatusIcon *gkbd_status_new (void);

void gkbd_status_reinit_ui (GkbdStatus * gki);

XklEngine *gkbd_status_get_xkl_engine (void);

gchar **gkbd_status_get_group_names (void);

gchar *gkbd_status_get_image_filename (guint group);

G_END_DECLS
#endif
