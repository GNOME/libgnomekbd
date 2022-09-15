/*
 * Copyright 2022 Corentin Noël <corentin.noel@collabora.com>
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

#ifndef GKBD_GEOMETRY_PARSER_H
#define GKBD_GEOMETRY_PARSER_H 1

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <graphene.h>

G_BEGIN_DECLS

#define GKBD_TYPE_GEOMETRY (gkbd_geometry_get_type())

typedef struct _GkbdGeometry GkbdGeometry;

#define GKBD_TYPE_SHAPE_OUTLINE (gkbd_shape_outline_get_type())

typedef struct _GkbdShapeOutline GkbdShapeOutline;

#define GKBD_TYPE_SOLID (gkbd_solid_get_type())

typedef struct _GkbdSolid GkbdSolid;

#define GKBD_TYPE_SHAPE (gkbd_shape_get_type())

typedef struct _GkbdShape GkbdShape;

#define GKBD_TYPE_COLOR (gkbd_color_get_type())

typedef struct _GkbdColor GkbdColor;

#define GKBD_TYPE_INDICATOR (gkbd_indicator_get_type())

typedef struct _GkbdIndicator GkbdIndicator;

#define GKBD_TYPE_TEXT (gkbd_text_get_type())

typedef struct _GkbdText GkbdText;

#define GKBD_TYPE_SECTION (gkbd_section_get_type())

typedef struct _GkbdSection GkbdSection;

#define GKBD_TYPE_ROW (gkbd_row_get_type())

typedef struct _GkbdRow GkbdRow;

#define GKBD_TYPE_KEY (gkbd_key_get_type())

typedef struct _GkbdKey GkbdKey;

#define GKBD_TYPE_ALIAS (gkbd_alias_get_type())

typedef struct _GkbdAlias GkbdAlias;

#define GKBD_TYPE_GEOMETRY_PARSER (gkbd_geometry_parser_get_type())

G_DECLARE_FINAL_TYPE (GkbdGeometryParser, gkbd_geometry_parser, GKBD, GEOMETRY_PARSER, GObject)

GkbdGeometryParser *gkbd_geometry_parser_new (void);

void gkbd_geometry_parser_parse_file (GkbdGeometryParser  *self,
                                      const char          *file,
                                      GCancellable        *cancellable,
                                      GAsyncReadyCallback  callback,
                                      gpointer             user_data);
GList *gkbd_geometry_parser_parse_file_finish (GkbdGeometryParser  *self,
                                               GAsyncResult        *result,
                                               GError             **error);

GType gkbd_geometry_get_type (void);
GkbdGeometry *gkbd_geometry_ref (GkbdGeometry *geometry);
void gkbd_geometry_unref (GkbdGeometry *geometry);
const char *gkbd_geometry_get_name (GkbdGeometry *geometry);
const char *gkbd_geometry_get_description (GkbdGeometry *geometry);
float gkbd_geometry_get_width_mm (GkbdGeometry *geometry);
float gkbd_geometry_get_height_mm (GkbdGeometry *geometry);
GkbdColor *gkbd_geometry_get_label_color (GkbdGeometry *geometry);
GkbdColor *gkbd_geometry_get_base_color (GkbdGeometry *geometry);
GList *gkbd_geometry_get_aliases (GkbdGeometry *geometry);
GList *gkbd_geometry_get_sections (GkbdGeometry *geometry);
GList *gkbd_geometry_get_solids (GkbdGeometry *geometry);
GList *gkbd_geometry_get_indicators (GkbdGeometry *geometry);
GList *gkbd_geometry_get_texts (GkbdGeometry *geometry);
GList *gkbd_geometry_get_colors (GkbdGeometry *geometry);

GType gkbd_shape_get_type (void);
GkbdShape *gkbd_shape_ref (GkbdShape *shape);
void gkbd_shape_unref (GkbdShape *shape);
GList *gkbd_shape_get_outlines (GkbdShape *shape);
GkbdShapeOutline *gkbd_shape_get_approx (GkbdShape *shape);
GkbdShapeOutline *gkbd_shape_get_primary (GkbdShape *shape);
void gkbd_shape_get_bounds (GkbdShape       *shape,
                            graphene_rect_t *bounds);

GType gkbd_shape_outline_get_type (void);
GkbdShapeOutline *gkbd_shape_outline_ref (GkbdShapeOutline *shape_outline);
void gkbd_shape_outline_unref (GkbdShapeOutline *shape_outline);
GList *gkbd_shape_outline_get_points (GkbdShapeOutline *shape_outline);
float gkbd_shape_outline_get_corner_radius (GkbdShapeOutline *shape_outline);

GType gkbd_solid_get_type (void);
GkbdSolid *gkbd_solid_ref (GkbdSolid *solid);
void gkbd_solid_unref (GkbdSolid *solid);
const char *gkbd_solid_get_name (GkbdSolid *solid);
guint gkbd_solid_get_priority (GkbdSolid *solid);
float gkbd_solid_get_top (GkbdSolid *solid);
float gkbd_solid_get_left (GkbdSolid *solid);
float gkbd_solid_get_angle (GkbdSolid *solid);
GkbdColor *gkbd_solid_get_color (GkbdSolid *solid);
GkbdShape *gkbd_solid_get_shape (GkbdSolid *solid);

GType gkbd_color_get_type (void);
GkbdColor *gkbd_color_ref (GkbdColor *color);
void gkbd_color_unref (GkbdColor *color);
const char *gkbd_color_get_name (GkbdColor *color);

GType gkbd_indicator_get_type (void);
GkbdIndicator *gkbd_indicator_ref (GkbdIndicator *indicator);
void gkbd_indicator_unref (GkbdIndicator *indicator);
const char *gkbd_indicator_get_name (GkbdIndicator *indicator);
guint gkbd_indicator_get_priority (GkbdIndicator *indicator);
float gkbd_indicator_get_top (GkbdIndicator *indicator);
float gkbd_indicator_get_left (GkbdIndicator *indicator);
float gkbd_indicator_get_angle (GkbdIndicator *indicator);
GkbdShape *gkbd_indicator_get_shape (GkbdIndicator *indicator);
GkbdColor *gkbd_indicator_get_on_color (GkbdIndicator *indicator);
GkbdColor *gkbd_indicator_get_off_color (GkbdIndicator *indicator);

GType gkbd_text_get_type (void);
GkbdText *gkbd_text_ref (GkbdText *text);
void gkbd_text_unref (GkbdText *text);
const char *gkbd_text_get_name (GkbdText *text);
guint gkbd_text_get_priority (GkbdText *text);
float gkbd_text_get_top (GkbdText *text);
float gkbd_text_get_left (GkbdText *text);
float gkbd_text_get_angle (GkbdText *text);
float gkbd_text_get_width (GkbdText *text);
float gkbd_text_get_height (GkbdText *text);
GkbdColor *gkbd_text_get_color (GkbdText *text);
const char *gkbd_text_get_text (GkbdText *text);
const char *gkbd_text_get_font (GkbdText *text);

GType gkbd_section_get_type (void);
GkbdSection *gkbd_section_ref (GkbdSection *section);
void gkbd_section_unref (GkbdSection *section);
const char *gkbd_section_get_name (GkbdSection *section);
GList *gkbd_section_get_rows (GkbdSection *section);
float gkbd_section_get_top (GkbdSection *section);
float gkbd_section_get_left (GkbdSection *section);
float gkbd_section_get_angle (GkbdSection *section);
guint gkbd_section_get_priority (GkbdSection *section);

GType gkbd_row_get_type (void);
GkbdRow *gkbd_row_ref (GkbdRow *row);
void gkbd_row_unref (GkbdRow *row);
GList *gkbd_row_get_keys (GkbdRow *row);
float gkbd_row_get_top (GkbdRow *row);
float gkbd_row_get_left (GkbdRow *row);
gboolean gkbd_row_get_vertical (GkbdRow *row);

GType gkbd_key_get_type (void);
GkbdKey *gkbd_key_ref (GkbdKey *key);
void gkbd_key_unref (GkbdKey *key);
const char *gkbd_key_get_name (GkbdKey *key);
float gkbd_key_get_gap (GkbdKey *key);
GkbdShape *gkbd_key_get_shape (GkbdKey *key);

GType gkbd_alias_get_type (void);
GkbdAlias *gkbd_alias_ref (GkbdAlias *alias);
void gkbd_alias_unref (GkbdAlias *alias);

#define GKBD_GEOMETRY_PARSER_ERROR gkbd_geometry_parser_error_quark ()

typedef enum {
  GKBD_GEOMETRY_PARSER_ERROR_FAILED /* generic failure, error->message
                                     * should explain
                                     */
} GkbdGeometryParserError;


G_DEFINE_AUTOPTR_CLEANUP_FUNC (GkbdGeometry, gkbd_geometry_unref);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GkbdShape, gkbd_shape_unref);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GkbdShapeOutline, gkbd_shape_outline_unref);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GkbdSolid, gkbd_solid_unref);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GkbdColor, gkbd_color_unref);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GkbdIndicator, gkbd_indicator_unref);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GkbdText, gkbd_text_unref);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GkbdSection, gkbd_section_unref);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GkbdRow, gkbd_row_unref);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GkbdKey, gkbd_key_unref);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GkbdAlias, gkbd_alias_unref);

G_END_DECLS

#endif				/* #ifndef GKBD_GEOMETRY_PARSER_H */
