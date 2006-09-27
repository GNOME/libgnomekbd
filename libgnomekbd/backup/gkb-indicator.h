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

#ifndef __GKB_INDICATOR_H__
#define __GKB_INDICATOR_H__

#include <gtk/gtk.h>

#include <libxklavier/xklavier.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct _GkbIndicator GkbIndicator;
	typedef struct _GkbIndicatorPrivate GkbIndicatorPrivate;
	typedef struct _GkbIndicatorClass GkbIndicatorClass;

#define GKB_TYPE_INDICATOR             (gkb_indicator_get_type ())
#define GKB_INDICATOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GKB_TYPE_INDICATOR, GkbIndicator))
#define GKB_INDCATOR_CLASS(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), GKB_TYPE_INDICATOR,  GkbIndicatorClass))
#define GKB_IS_INDICATOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GKB_TYPE_INDICATOR))
#define GKB_IS_INDICATOR_CLASS(obj)    (G_TYPE_CHECK_CLASS_TYPE ((obj), GKB_TYPE_INDICATOR))
#define GKB_INDICATOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GKB_TYPE_INDICATOR, GkbIndicatorClass))

	struct _GkbIndicator {
		GtkNotebook parent;
		GkbIndicatorPrivate *priv;
	};

	struct _GkbIndicatorClass {
		GtkNotebookClass parent_class;

		void (*reinit_ui) (GkbIndicator * gki);
	};

	extern GType gkb_indicator_get_type (void);

	extern GtkWidget *gkb_indicator_new (void);

	extern void gkb_indicator_reinit_ui (GkbIndicator * gki);

	extern void gkb_indicator_set_angle (GkbIndicator * gki,
					     gdouble angle);

	extern XklEngine *gkb_indicator_get_xkl_engine (void);

	extern gchar **gkb_indicator_get_group_names (void);

	extern gchar *gkb_indicator_get_image_filename (guint group);

	extern gdouble gkb_indicator_get_max_width_height_ratio (void);

	extern void
	 gkb_indicator_set_parent_tooltips (GkbIndicator *
					    gki, gboolean ifset);

	extern void
	 gkb_indicator_set_tooltips_format (const gchar str[]);

#ifdef __cplusplus
}
#endif
#endif
