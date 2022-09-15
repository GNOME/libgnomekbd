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

#include <glib/gstdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <graphene.h>

#include "gkbd-geometry-parser.h"


struct _GkbdGeometry {
    gatomicrefcount   ref_count;
    char *            name;            /* keyboard name */
    char *            description;     /* keyboard description */
    gushort           width_mm;        /* keyboard width in mm/10 */
    gushort           height_mm;       /* keyboard height in mm/10 */
    char *            label_font;      /* font for key labels */
    GkbdColor        *label_color;     /* color for key labels */
    GkbdColor        *base_color;      /* color for basic keyboard */
    GList            *colors;          /* colors array */
    GList            *shapes;          /* shapes array */
    GList            *indicators;      /* indicators array */
    GList            *texts;           /* texts array */
    GList            *sections;        /* sections array */
    GList            *solids;          /* solids array */
    GList            *outlines;        /* solids array */
    GList            *aliases;         /* key aliases array */
    //XkbDoodadPtr      doodads;         /* doodads array */
    //XkbKeyAliasPtr    key_aliases;     /* key aliases array */
};

struct _GkbdShapeOutline {
    gatomicrefcount  ref_count;
    float            corner_radius; /* draw corners as circles with this radius */
    GList           *points;        /* array of points defining the outline */
};

struct _GkbdSolid {
    gatomicrefcount  ref_count;
    gboolean         is_outline;
    char            *name;         /* doodad name */
    unsigned char    priority;     /* drawing priority,
                                       0⇒highest, 255⇒lowest */
    float            top;          /* top coordinate, in mm/10 */
    float            left;         /* left coordinate, in mm/10 */
    float            angle;        /* angle of rotation, clockwise,
                                      in 1/10 degrees */
    GkbdColor       *color;        /* doodad color */
    GkbdShape       *shape;        /* doodad shape */
};

struct _GkbdShape {
    gatomicrefcount   ref_count;
    char             *name;             /* shape’s name */
    GList            *outlines;         /* array of outlines for the shape */
    GkbdShapeOutline *approx;           /* pointer into the array to the
                                           approximating outline */
    GkbdShapeOutline *primary;          /* pointer into the array to the
                                           primary outline */
    graphene_rect_t   bounds;           /* bounding box for the shape;
                                           encompasses all outlines */
};

struct _GkbdColor {
    gatomicrefcount  ref_count;
    char            *name;         /* doodad name */
};

struct _GkbdIndicator {
    gatomicrefcount  ref_count;
    char            *name;         /* doodad name */
    unsigned char    priority;     /* drawing priority, 0⇒highest, 255⇒lowest */
    float            top;          /* top coordinate, in mm/10 */
    float            left;         /* left coordinate, in mm/10 */
    float            angle;        /* angle of rotation, clockwise,
                                      in 1/10 degrees */
    GkbdShape       *shape;        /* doodad shape */
    GkbdColor       *on_color;     /* color for doodad if indicator is on */
    GkbdColor       *off_color;    /* color for doodad if indicator is off */
};

struct _GkbdText {
    gatomicrefcount  ref_count;
    char            *name;         /* doodad name */
    unsigned char    priority;     /* drawing priority, 0⇒highest, 255⇒lowest */
    float            top;          /* top coordinate, in mm/10 */
    float            left;         /* left coordinate, in mm/10 */
    float            angle;        /* angle of rotation, clockwise,
                                      in 1/10 degrees */
    short            width;        /* width in mm/10 */
    short            height;       /* height in mm/10 */
    GkbdColor       *color;        /* doodad color */
    char *           text;         /* doodad text */
    char *           font;         /* arbitrary font name for doodad text */
};

struct _GkbdSection {
    gatomicrefcount  ref_count;
    char            *name;         /* doodad name */
    unsigned char    priority;     /* drawing priority, 0⇒highest, 255⇒lowest */
    float            top;          /* top coordinate of section origin */
    float            left;         /* left coordinate of row origin */
    unsigned short   width;        /* section width, in mm/10 */
    unsigned short   height;       /* section height, in mm/10 */
    float            angle;        /* angle of section rotation,
                                      counterclockwise */
    GList           *rows;         /* section rows array */
    //XkbDoodadPtr    doodads;       /* section doodads array */
    //XkbBoundsRec    bounds;        /* bounding box for the section,
    //                                  before rotation */
    //XkbOverlayPtr   overlays;      /* section overlays array */
};

struct _GkbdRow {
    gatomicrefcount  ref_count;
    float            top;       /* top coordinate of row origin,
                                   relative to section’s origin */
    float            left;      /* left coordinate of row origin,
                                   relative to section’s origin */
    int              vertical;  /* True ⇒vertical row,
                                   False ⇒horizontal row */
    GList           *keys;      /* keys array */
    //XkbBoundsRec     bounds;    /* bounding box for the row */
};

struct _GkbdKey {
    gatomicrefcount  ref_count;
    char            *name;         /* doodad name */
    float            gap;          /* gap in mm/10 from previous key in row */
    GkbdShape       *shape;        /* shape for key */
    GkbdColor       *color;        /* color for key body */
};

struct _GkbdAlias {
    gatomicrefcount  ref_count;
    char            *name;         /* alias name */
    char            *real;         /* real key name */
};

G_DEFINE_BOXED_TYPE (GkbdGeometry, gkbd_geometry, gkbd_geometry_ref, gkbd_geometry_unref)
G_DEFINE_BOXED_TYPE (GkbdShapeOutline, gkbd_shape_outline, gkbd_shape_outline_ref, gkbd_shape_outline_unref)
G_DEFINE_BOXED_TYPE (GkbdSolid, gkbd_solid, gkbd_solid_ref, gkbd_solid_unref)
G_DEFINE_BOXED_TYPE (GkbdShape, gkbd_shape, gkbd_shape_ref, gkbd_shape_unref)
G_DEFINE_BOXED_TYPE (GkbdColor, gkbd_color, gkbd_color_ref, gkbd_color_unref)
G_DEFINE_BOXED_TYPE (GkbdIndicator, gkbd_indicator, gkbd_indicator_ref, gkbd_indicator_unref)
G_DEFINE_BOXED_TYPE (GkbdText, gkbd_text, gkbd_text_ref, gkbd_text_unref)
G_DEFINE_BOXED_TYPE (GkbdSection, gkbd_section, gkbd_section_ref, gkbd_section_unref)
G_DEFINE_BOXED_TYPE (GkbdRow, gkbd_row, gkbd_row_ref, gkbd_row_unref)
G_DEFINE_BOXED_TYPE (GkbdKey, gkbd_key, gkbd_key_ref, gkbd_key_unref)
G_DEFINE_BOXED_TYPE (GkbdAlias, gkbd_alias, gkbd_alias_ref, gkbd_alias_unref)

G_DEFINE_QUARK (gkbd-geometry-parser-error-quark, gkbd_geometry_parser_error)

struct _GkbdGeometryParser
{
	GObject parent_instance;

	GScanner *scanner;
};

G_DEFINE_FINAL_TYPE (GkbdGeometryParser, gkbd_geometry_parser, G_TYPE_OBJECT)

enum {
	PROP_0,
	N_PROPS
};

static GParamSpec *properties [N_PROPS];

static GkbdGeometry *
gkbd_geometry_new (void)
{
	GkbdGeometry *self = g_new0 (GkbdGeometry, 1);
	g_atomic_ref_count_init (&self->ref_count);
	return self;
}

static GkbdShape *
gkbd_shape_new (void)
{
	GkbdShape *self = g_new0 (GkbdShape, 1);
	g_atomic_ref_count_init (&self->ref_count);
	graphene_rect_init (&self->bounds, G_MAXFLOAT, G_MAXFLOAT, 0.f, 0.f);
	return self;
}

static GkbdShapeOutline *
gkbd_shape_outline_new (void)
{
	GkbdShapeOutline *self = g_new0 (GkbdShapeOutline, 1);
	g_atomic_ref_count_init (&self->ref_count);
	return self;
}

static GkbdSolid *
gkbd_solid_new (void)
{
	GkbdSolid *self = g_new0 (GkbdSolid, 1);
	g_atomic_ref_count_init (&self->ref_count);
	return self;
}

static GkbdColor *
gkbd_color_new (void)
{
	GkbdColor *self = g_new0 (GkbdColor, 1);
	g_atomic_ref_count_init (&self->ref_count);
	return self;
}

static GkbdIndicator *
gkbd_indicator_new (void)
{
	GkbdIndicator *self = g_new0 (GkbdIndicator, 1);
	g_atomic_ref_count_init (&self->ref_count);
	return self;
}

static GkbdText *
gkbd_text_new (void)
{
	GkbdText *self = g_new0 (GkbdText, 1);
	g_atomic_ref_count_init (&self->ref_count);
	return self;
}

static GkbdSection *
gkbd_section_new (void)
{
	GkbdSection *self = g_new0 (GkbdSection, 1);
	g_atomic_ref_count_init (&self->ref_count);
	return self;
}

static GkbdRow *
gkbd_row_new (void)
{
	GkbdRow *self = g_new0 (GkbdRow, 1);
	g_atomic_ref_count_init (&self->ref_count);
	return self;
}

static GkbdKey *
gkbd_key_new (void)
{
	GkbdKey *self = g_new0 (GkbdKey, 1);
	g_atomic_ref_count_init (&self->ref_count);
	return self;
}

static GkbdAlias *
gkbd_alias_new (void)
{
	GkbdAlias *self = g_new0 (GkbdAlias, 1);
	g_atomic_ref_count_init (&self->ref_count);
	return self;
}

GkbdGeometryParser *
gkbd_geometry_parser_new (void)
{
	return g_object_new (GKBD_TYPE_GEOMETRY_PARSER, NULL);
}

static void
gkbd_geometry_parser_finalize (GObject *object)
{
	GkbdGeometryParser *self = (GkbdGeometryParser *)object;

	g_clear_pointer (&self->scanner, g_scanner_destroy);

	G_OBJECT_CLASS (gkbd_geometry_parser_parent_class)->finalize (object);
}

static void
gkbd_geometry_parser_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
	GkbdGeometryParser *self = GKBD_GEOMETRY_PARSER (object);

	switch (prop_id)
	  {
	  default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  }
}

static void
gkbd_geometry_parser_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
	GkbdGeometryParser *self = GKBD_GEOMETRY_PARSER (object);

	switch (prop_id)
	  {
	  default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	  }
}

static void
gkbd_geometry_parser_class_init (GkbdGeometryParserClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gkbd_geometry_parser_finalize;
	object_class->get_property = gkbd_geometry_parser_get_property;
	object_class->set_property = gkbd_geometry_parser_set_property;
}

static GScannerConfig scanner_config =
{
  .cset_skip_characters = ( " \t\r\n;" ),
  .cset_identifier_first = "_" G_CSET_a_2_z G_CSET_A_2_Z,
  .cset_identifier_nth = G_CSET_DIGITS "-_" G_CSET_a_2_z G_CSET_A_2_Z,
  .cpair_comment_single = "/\n",
  .case_sensitive = TRUE,
  .skip_comment_multi = TRUE,
  .skip_comment_single = TRUE,
  .scan_comment_multi = FALSE,
  .scan_identifier = TRUE,
  .scan_identifier_1char = TRUE,
  .scan_identifier_NULL = FALSE,
  .scan_symbols = TRUE,
  .scan_binary = FALSE,
  .scan_octal = FALSE,
  .scan_float = FALSE,
  .scan_hex = FALSE,
  .scan_hex_dollar = FALSE,
  .scan_string_sq = TRUE,
  .scan_string_dq = TRUE,
  .numbers_2_int = TRUE,
  .int_2_float = FALSE,
  .identifier_2_string = FALSE,
  .char_2_token = TRUE,
  .symbol_2_token = TRUE,
  .scope_0_fallback = FALSE,
  .store_int64 = FALSE
};

typedef enum {
	GKBD_GEOMETRY_SCOPE_ROOT,
} GkbdGeometryScope;

static void
gkbd_geometry_parser_init (GkbdGeometryParser *self)
{
	self->scanner = g_scanner_new (&scanner_config);
}

static void
gkbd_value_free (gpointer val)
{
	g_value_unset ((GValue *)val);
	g_free (val);
}

static void
clone_keyval (gpointer key,
              gpointer value,
              gpointer user_data)
{
	g_hash_table_insert (user_data, g_strdup (key), g_boxed_copy (G_TYPE_VALUE, value));
}

static GHashTable *
clone_properties_table (GHashTable *table)
{
	GHashTable *clone = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, gkbd_value_free);
	g_hash_table_foreach (table, clone_keyval, clone);
	return clone;
}

static GkbdShape *
find_shape (GkbdGeometry *geometry,
            const char   *name)
{
	for (GList *l = geometry->shapes; l != NULL; l = l->next) {
		GkbdShape *shape = l->data;
		if (!g_strcmp0(shape->name, name))
			return shape;
	}

	return NULL;
}

static GkbdColor *
find_color (GkbdGeometry *geometry,
            const char   *name)
{
	for (GList *l = geometry->colors; l != NULL; l = l->next) {
		GkbdColor *color = l->data;
		if (!g_strcmp0(color->name, name))
			return color;
	}

	GkbdColor *color = gkbd_color_new ();
	color->name = g_strdup (name);
	geometry->colors = g_list_append (geometry->colors, color);
	return color;
}

#define gkbd_error_val_if_wrong_token(scanner, next_token, val, error) \
  G_STMT_START { \
    if (G_LIKELY (g_scanner_get_next_token (scanner) == next_token)) \
      { } \
    else \
      { \
        g_set_error (error, \
		     GKBD_GEOMETRY_PARSER_ERROR, \
		     GKBD_GEOMETRY_PARSER_ERROR_FAILED, \
		     "Unexpected token at %d:%d, a %s was expected", \
		     g_scanner_cur_line (scanner), \
		     g_scanner_cur_position (scanner), \
		     #next_token); \
        return (val); \
      } \
  } G_STMT_END

static gboolean
parse_number (GkbdGeometryParser  *self,
	      float               *parsed_number,
	      GError             **error)
{
	GTokenValue cur_val = g_scanner_cur_value (self->scanner);
	*parsed_number = 0.f;

	if (g_scanner_get_next_token (self->scanner) == '-') {
		cur_val = g_scanner_cur_value (self->scanner);
		gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_INT, FALSE, error);
		*parsed_number = -cur_val.v_int;
	} else {
		cur_val = g_scanner_cur_value (self->scanner);
		*parsed_number = cur_val.v_int;
	}

	if (g_scanner_peek_next_token (self->scanner) == '.') {
		g_scanner_get_next_token (self->scanner);
		gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_INT, FALSE, error);
		cur_val = g_scanner_cur_value (self->scanner);
		float decimal_part = (float) cur_val.v_int;
		while (decimal_part > 1.f) {
			decimal_part /= 10;
		}

		*parsed_number += decimal_part;
	}

	while (g_scanner_peek_next_token (self->scanner) == '+') {
		g_scanner_get_next_token (self->scanner);
		gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_INT, FALSE, error);
		cur_val = g_scanner_cur_value (self->scanner);
		*parsed_number += cur_val.v_int;
		if (g_scanner_peek_next_token (self->scanner) == '.') {
			g_scanner_get_next_token (self->scanner);
			gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_INT, FALSE, error);
			cur_val = g_scanner_cur_value (self->scanner);
			float decimal_part = (float) cur_val.v_int;
			while (decimal_part > 1.f) {
				decimal_part /= 10;
			}

			*parsed_number += decimal_part;
		}
	}

	return TRUE;
}

static gboolean
parse_positive_number (GkbdGeometryParser  *self,
                       float               *parsed_number,
                       GError             **error)
{
	GTokenValue cur_val;
	*parsed_number = 0.f;

	cur_val = g_scanner_cur_value (self->scanner);
	*parsed_number = cur_val.v_int;
	if (g_scanner_peek_next_token (self->scanner) == '.') {
		g_scanner_get_next_token (self->scanner);
		gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_INT, FALSE, error);
		cur_val = g_scanner_cur_value (self->scanner);
		float decimal_part = (float) cur_val.v_int;
		while (decimal_part > 1.f) {
			decimal_part /= 10;
		}

		*parsed_number += decimal_part;
	}
	while (g_scanner_peek_next_token (self->scanner) == '+') {
		g_scanner_get_next_token (self->scanner);
		gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_INT, FALSE, error);
		cur_val = g_scanner_cur_value (self->scanner);
		*parsed_number += cur_val.v_int;
		if (g_scanner_peek_next_token (self->scanner) == '.') {
			g_scanner_get_next_token (self->scanner);
			gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_INT, FALSE, error);
			cur_val = g_scanner_cur_value (self->scanner);
			float decimal_part = (float) cur_val.v_int;
			while (decimal_part > 1.f) {
				decimal_part /= 10;
			}

			*parsed_number += decimal_part;
		}
	}

	return TRUE;
}

static GList *
parse_point_list (GkbdGeometryParser  *self,
                  GError             **error)
{
	GList *points = NULL;
	GTokenType token;

	do {
		graphene_point_t *point = graphene_point_alloc ();
		gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_LEFT_BRACE, NULL, error);
		if (!parse_number (self, &point->x, error))
			return NULL;
		point->x *= 10;
		gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_COMMA, NULL, error);
		if (!parse_number (self, &point->y, error))
			return NULL;
		point->y *= 10;
		gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_RIGHT_BRACE, NULL, error);
		points = g_list_prepend (points, point);
	} while ((token = g_scanner_get_next_token (self->scanner)) == G_TOKEN_COMMA);

	return g_list_reverse (points);
}

static void
calculate_bound (GkbdShape *shape)
{
	for (GList *listoutlines = shape->outlines; listoutlines != NULL; listoutlines = listoutlines->next) {
		GkbdShapeOutline *outline = listoutlines->data;
		for (GList *listpoints = outline->points; listpoints != NULL; listpoints = listpoints->next) {
			graphene_point_t *point = listpoints->data;
			if (point->x < shape->bounds.origin.x)
				shape->bounds.origin.x = point->x;
			if (point->x > shape->bounds.size.width)
				shape->bounds.size.width = point->x;
			if (point->y < shape->bounds.origin.y)
				shape->bounds.origin.y = point->y;
			if (point->y > shape->bounds.size.height)
				shape->bounds.size.height = point->y;
		}
	}
}

static GkbdShape *
parse_shape (GkbdGeometryParser  *self,
             GHashTable          *properties,
             GCancellable        *cancellable,
             GError             **error)
{
	g_autoptr(GkbdShape) shape = gkbd_shape_new ();
	GTokenType token;
	GTokenValue cur_val;

	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
	cur_val = g_scanner_cur_value (self->scanner);
	shape->name = g_strdup (cur_val.v_string);
	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_LEFT_CURLY, NULL, error);

	GValue *val = g_hash_table_lookup (properties, "shape.cornerRadius");
	float corner_radius = 0;
	if (val)
		corner_radius = g_value_get_float (val);

	while ((token = g_scanner_get_next_token (self->scanner)) != G_TOKEN_EOF) {
		if (g_cancellable_is_cancelled (cancellable)) {
			return NULL;
		}

		cur_val = g_scanner_cur_value (self->scanner);

		switch (token) {
		case G_TOKEN_RIGHT_CURLY:
			calculate_bound (shape);
			shape->outlines = g_list_reverse (shape->outlines);
			return g_steal_pointer (&shape);
		case G_TOKEN_LEFT_CURLY:
			{
				g_autoptr(GkbdShapeOutline) shape_outline = gkbd_shape_outline_new ();
				shape_outline->corner_radius = corner_radius;
				shape_outline->points = parse_point_list (self, error);
				if (!shape_outline->points && error && *error)
					return NULL;

				shape->outlines = g_list_prepend (shape->outlines, g_steal_pointer (&shape_outline));
			}
			break;
		case G_TOKEN_IDENTIFIER:
			if (!g_strcmp0 (cur_val.v_identifier, "cornerRadius")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &corner_radius, error))
					return NULL;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "approx")) {
				g_autoptr(GkbdShapeOutline) shape_outline = gkbd_shape_outline_new ();
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_LEFT_CURLY, NULL, error);
				shape_outline->corner_radius = corner_radius;
				shape_outline->points = parse_point_list (self, error);
				if (!shape_outline->points && error && *error)
					return NULL;

				shape->outlines = g_list_prepend (shape->outlines, g_steal_pointer (&shape_outline));
				shape->approx = shape_outline;
				break;
			} else {
				g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
				             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
				             "Unexpected identifier at %d:%d: %s",
				             g_scanner_cur_line (self->scanner),
				             g_scanner_cur_position (self->scanner),
				             cur_val.v_identifier);
				return NULL;
			}
			break;
		case G_TOKEN_COMMA:
			continue;
		default:
			g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
			             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
			             "Unexpected token at %d:%d: '%c'",
			             g_scanner_cur_line (self->scanner),
			             g_scanner_cur_position (self->scanner),
			             token);
			return NULL;
		}
	}

	calculate_bound (shape);
	shape->outlines = g_list_reverse (shape->outlines);
	return g_steal_pointer (&shape);
}

static GkbdIndicator *
parse_indicator (GkbdGeometryParser  *self,
                 GkbdGeometry        *geometry,
                 GHashTable          *properties,
                 GCancellable        *cancellable,
                 GError             **error)
{
	g_autoptr(GkbdIndicator) indicator = gkbd_indicator_new ();
	GTokenType token;
	GTokenValue cur_val;
	GValue *val;

	val = g_hash_table_lookup (properties, "indicator.left");
	if (val)
		indicator->left = g_value_get_float (val) * 10;

	val = g_hash_table_lookup (properties, "indicator.top");
	if (val)
		indicator->top = g_value_get_float (val) * 10;

	val = g_hash_table_lookup (properties, "indicator.angle");
	if (val)
		indicator->angle = g_value_get_float (val) * 10;

	val = g_hash_table_lookup (properties, "indicator.onColor");
	if (val)
		indicator->on_color = find_color (geometry, g_value_get_string (val));

	val = g_hash_table_lookup (properties, "indicator.offColor");
	if (val)
		indicator->off_color = find_color (geometry, g_value_get_string (val));

	val = g_hash_table_lookup (properties, "indicator.shape");
	if (val)
		indicator->shape = find_shape (geometry, g_value_get_string (val));

	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
	cur_val = g_scanner_cur_value (self->scanner);
	indicator->name = g_strdup (cur_val.v_string);
	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_LEFT_CURLY, NULL, error);

	while ((token = g_scanner_get_next_token (self->scanner)) != G_TOKEN_EOF) {
		if (g_cancellable_is_cancelled (cancellable)) {
			return NULL;
		}

		cur_val = g_scanner_cur_value (self->scanner);

		switch (token) {
		case G_TOKEN_RIGHT_CURLY:
			return g_steal_pointer (&indicator);
		case G_TOKEN_IDENTIFIER:
			if (!g_strcmp0 (cur_val.v_identifier, "priority")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				float fpriority = 0.f;
				if (!parse_number (self, &fpriority, error))
					return NULL;
				indicator->priority = (int) fpriority;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "top")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &indicator->top, error))
					return NULL;
				indicator->top *= 10;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "left")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &indicator->left, error))
					return NULL;
				indicator->left *= 10;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "angle")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &indicator->angle, error))
					return NULL;
				indicator->angle *= 10;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "onColor")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				indicator->on_color = find_color (geometry, cur_val.v_string);
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "offColor")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				indicator->off_color = find_color (geometry, cur_val.v_string);
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "shape")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				indicator->shape = find_shape (geometry, cur_val.v_string);
				break;
			} else {
				g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
				             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
				             "Unexpected identifier at %d:%d: %s",
				             g_scanner_cur_line (self->scanner),
				             g_scanner_cur_position (self->scanner),
				             cur_val.v_identifier);
				return NULL;
			}
			break;
		case G_TOKEN_COMMA:
			continue;
		default:
			g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
			             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
			             "Unexpected token at %d:%d: '%c'",
			             g_scanner_cur_line (self->scanner),
			             g_scanner_cur_position (self->scanner),
			             token);
			return NULL;
		}
	}

	return g_steal_pointer (&indicator);
}

static GkbdText *
parse_text (GkbdGeometryParser  *self,
            GkbdGeometry        *geometry,
            GHashTable          *properties,
            GCancellable        *cancellable,
            GError             **error)
{
	g_autoptr(GkbdText) text = gkbd_text_new ();
	GTokenType token;
	GTokenValue cur_val;
	GValue *val;

	val = g_hash_table_lookup (properties, "text.priority");
	if (val)
		text->priority = g_value_get_float (val);

	val = g_hash_table_lookup (properties, "text.left");
	if (val)
		text->left = g_value_get_float (val);

	val = g_hash_table_lookup (properties, "text.top");
	if (val)
		text->top = g_value_get_float (val);

	val = g_hash_table_lookup (properties, "text.angle");
	if (val)
		text->angle = g_value_get_float (val);

	val = g_hash_table_lookup (properties, "text.color");
	if (val)
		text->color = find_color (geometry, g_value_get_string (val));

	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
	cur_val = g_scanner_cur_value (self->scanner);
	text->name = g_strdup (cur_val.v_string);
	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_LEFT_CURLY, NULL, error);

	while ((token = g_scanner_get_next_token (self->scanner)) != G_TOKEN_EOF) {
		if (g_cancellable_is_cancelled (cancellable)) {
			return NULL;
		}

		cur_val = g_scanner_cur_value (self->scanner);

		switch (token) {
		case G_TOKEN_RIGHT_CURLY:
			return g_steal_pointer (&text);
		case G_TOKEN_IDENTIFIER:
			if (!g_strcmp0 (cur_val.v_identifier, "priority")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				float fpriority = 0.f;
				if (!parse_number (self, &fpriority, error))
					return NULL;
				text->priority = (int) fpriority;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "top")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &text->top, error))
					return NULL;
				text->top *= 10;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "left")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &text->left, error))
					return NULL;
				text->left *= 10;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "angle")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &text->angle, error))
					return NULL;
				text->angle *= 10;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "color")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				text->color = find_color (geometry, cur_val.v_string);
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "text")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				text->text = g_strdup (cur_val.v_string);
				break;
			} else {
				g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
				             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
				             "Unexpected identifier at %d:%d: %s",
				             g_scanner_cur_line (self->scanner),
				             g_scanner_cur_position (self->scanner),
				             cur_val.v_identifier);
				return NULL;
			}
			break;
		case G_TOKEN_COMMA:
			continue;
		default:
			g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
			             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
			             "Unexpected token at %d:%d: '%c'",
			             g_scanner_cur_line (self->scanner),
			             g_scanner_cur_position (self->scanner),
			             token);
			return NULL;
		}
	}

	return g_steal_pointer (&text);
}

static GkbdSolid *
parse_solid (GkbdGeometryParser  *self,
             GkbdGeometry        *geometry,
             gboolean             is_outline,
             GHashTable          *properties,
             GCancellable        *cancellable,
             GError             **error)
{
	g_autoptr(GkbdSolid) solid = gkbd_solid_new ();
	GTokenType token;
	GTokenValue cur_val;
	GValue *val;

	if (is_outline)
		val = g_hash_table_lookup (properties, "outline.left");
	else
		val = g_hash_table_lookup (properties, "solid.left");

	if (val)
		solid->left = g_value_get_float (val) * 10;

	if (is_outline)
		val = g_hash_table_lookup (properties, "outline.top");
	else
		val = g_hash_table_lookup (properties, "solid.top");

	if (val)
		solid->top = g_value_get_float (val) * 10;

	if (is_outline)
		val = g_hash_table_lookup (properties, "outline.angle");
	else
		val = g_hash_table_lookup (properties, "solid.angle");

	if (val)
		solid->angle = g_value_get_float (val) * 10;

	if (is_outline)
		val = g_hash_table_lookup (properties, "outline.color");
	else
		val = g_hash_table_lookup (properties, "solid.color");

	if (val)
		solid->color = find_color (geometry, g_value_get_string (val));

	if (is_outline)
		val = g_hash_table_lookup (properties, "outline.shape");
	else
		val = g_hash_table_lookup (properties, "solid.shape");

	if (val)
		solid->shape = find_shape (geometry, g_value_get_string (val));

	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
	cur_val = g_scanner_cur_value (self->scanner);
	solid->name = g_strdup (cur_val.v_string);
	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_LEFT_CURLY, NULL, error);

	while ((token = g_scanner_get_next_token (self->scanner)) != G_TOKEN_EOF) {
		if (g_cancellable_is_cancelled (cancellable)) {
			return NULL;
		}

		cur_val = g_scanner_cur_value (self->scanner);

		switch (token) {
		case G_TOKEN_RIGHT_CURLY:
			return g_steal_pointer (&solid);
		case G_TOKEN_IDENTIFIER:
			if (!g_strcmp0 (cur_val.v_identifier, "shape")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				solid->shape = find_shape (geometry, cur_val.v_string);
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "top")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &solid->top, error))
					return NULL;
				solid->top *= 10;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "left")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &solid->left, error))
					return NULL;
				solid->left *= 10;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "color")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				solid->color = find_color (geometry, cur_val.v_string);
				break;
			} else {
				g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
				             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
				             "Unexpected identifier at %d:%d: %s",
				             g_scanner_cur_line (self->scanner),
				             g_scanner_cur_position (self->scanner),
				             cur_val.v_identifier);
				return NULL;
			}
			break;
		default:
			g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
			             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
			             "Unexpected token at %d:%d: '%c'",
			             g_scanner_cur_line (self->scanner),
			             g_scanner_cur_position (self->scanner),
			             token);
			return NULL;
		}
	}

	return g_steal_pointer (&solid);
}

static GList *
parse_key_list (GkbdGeometryParser  *self,
                GkbdGeometry        *geometry,
                GHashTable          *properties,
                GError             **error)
{
	GList *keys = NULL;
	GTokenValue cur_val;
	GTokenType token;

	do {
		GkbdKey *key = gkbd_key_new ();
		GValue *val;
		val = g_hash_table_lookup (properties, "key.gap");
		if (val)
			key->gap = g_value_get_float (val) * 10;

		val = g_hash_table_lookup (properties, "key.color");
		if (val)
			key->color = find_color (geometry, g_value_get_string (val));

		val = g_hash_table_lookup (properties, "key.shape");
		if (val)
			key->shape = find_shape (geometry, g_value_get_string (val));

		if (g_scanner_get_next_token (self->scanner) == G_TOKEN_LEFT_CURLY) {
			gkbd_error_val_if_wrong_token (self->scanner, '<', NULL, error);
			gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_IDENTIFIER, NULL, error);
			cur_val = g_scanner_cur_value (self->scanner);
			key->name = g_strdup (cur_val.v_identifier);
			gkbd_error_val_if_wrong_token (self->scanner, '>', NULL, error);
			while ((token = g_scanner_get_next_token (self->scanner)) == G_TOKEN_COMMA) {
				token = g_scanner_get_next_token (self->scanner);
				cur_val = g_scanner_cur_value (self->scanner);
				if (token == G_TOKEN_IDENTIFIER) {
					if (!g_strcmp0 (cur_val.v_identifier, "color")) {
						gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
						gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
						cur_val = g_scanner_cur_value (self->scanner);
						key->color = find_color (geometry, cur_val.v_string);
					} else if (!g_strcmp0 (cur_val.v_identifier, "shape")) {
						gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
						gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
						cur_val = g_scanner_cur_value (self->scanner);
						key->shape = find_shape (geometry, cur_val.v_string);
					} else {
						g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
							     GKBD_GEOMETRY_PARSER_ERROR_FAILED,
							     "Unexpected identifier at %d:%d: %s",
							     g_scanner_cur_line (self->scanner),
							     g_scanner_cur_position (self->scanner),
							     cur_val.v_identifier);
						return NULL;
					}
				} else if (token == G_TOKEN_STRING) {
					key->shape = find_shape (geometry, cur_val.v_string);
				} else if (token == G_TOKEN_INT) {
					if (!parse_positive_number (self, &key->gap, error))
						return NULL;
					key->gap *= 10;
				} else {
					g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
						     GKBD_GEOMETRY_PARSER_ERROR_FAILED,
						     "Unexpected token at %d:%d: '%c'",
						     g_scanner_cur_line (self->scanner),
						     g_scanner_cur_position (self->scanner),
						     token);
					return NULL;
				}
			}
		} else {
			gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_IDENTIFIER, NULL, error);
			cur_val = g_scanner_cur_value (self->scanner);
			key->name = g_strdup (cur_val.v_string);
			gkbd_error_val_if_wrong_token (self->scanner, '>', NULL, error);
		}

		keys = g_list_prepend (keys, key);
	} while ((token = g_scanner_get_next_token (self->scanner)) == G_TOKEN_COMMA);

	return g_list_reverse (keys);
}

static bool
parse_property (GkbdGeometryParser  *self,
                const char          *prefix,
                GHashTable          *properties,
                GError             **error)
{
	g_autofree char *prop_name = NULL;
	GTokenType token;
	GTokenValue cur_val;

	gkbd_error_val_if_wrong_token (self->scanner, '.', NULL, error);
	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_IDENTIFIER, NULL, error);
	cur_val = g_scanner_cur_value (self->scanner);
	if (prefix)
		prop_name = g_strdup_printf ("%s.%s", prefix, cur_val.v_identifier);
	else
		prop_name = g_strdup (cur_val.v_identifier);
	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
	token = g_scanner_get_next_token (self->scanner);
	switch (token) {
	case G_TOKEN_INT:
		{
			g_autofree GValue *value = g_new0 (GValue, 1);
			g_value_init (value, G_TYPE_FLOAT);
			float num = 0.f;
			if (!parse_positive_number (self, &num, error)) {
				g_value_unset (value);
				return FALSE;
			}

			g_value_set_float (value, num);
			g_hash_table_insert (properties,
				             g_steal_pointer (&prop_name),
				             g_steal_pointer (&value));
		}

		return TRUE;
	case G_TOKEN_STRING:
		{
			GValue *value = g_new0(GValue, 1);
			g_value_init (value, G_TYPE_STRING);
			cur_val = g_scanner_cur_value (self->scanner);
			g_value_set_string (value, cur_val.v_string);
			g_hash_table_insert (properties,
				             g_steal_pointer (&prop_name),
				             value);
		}

		return TRUE;
	case G_TOKEN_IDENTIFIER:
		cur_val = g_scanner_cur_value (self->scanner);
		if (!g_strcmp0 (cur_val.v_identifier, "True")) {
			GValue *value = g_new0(GValue, 1);
			g_value_init (value, G_TYPE_BOOLEAN);
			g_value_set_boolean (value, TRUE);
			g_hash_table_insert (properties,
			                     g_steal_pointer (&prop_name),
			                     value);
			return TRUE;
		} else if (!g_strcmp0 (cur_val.v_identifier, "False")) {
			GValue *value = g_new0(GValue, 1);
			g_value_init (value, G_TYPE_BOOLEAN);
			g_value_set_boolean (value, FALSE);
			g_hash_table_insert (properties,
			                     g_steal_pointer (&prop_name),
			                     value);
			return TRUE;
		}
		G_GNUC_FALLTHROUGH;
	default:
		g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
		             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
		             "Unexpected token value at %d:%d for %s: '%c'",
		             g_scanner_cur_line (self->scanner),
		             g_scanner_cur_position (self->scanner),
			     prop_name,
		             token);
		return FALSE;
	}
}

static GkbdRow *
parse_row (GkbdGeometryParser  *self,
           GkbdGeometry        *geometry,
           GHashTable          *properties,
           GCancellable        *cancellable,
           GError             **error)
{
	g_autoptr(GkbdRow) row = gkbd_row_new ();
	GTokenType token;
	GTokenValue cur_val;
	GValue *val;
	val = g_hash_table_lookup (properties, "row.top");
	if (val)
		row->top = g_value_get_float (val) * 10;

	val = g_hash_table_lookup (properties, "row.left");
	if (val)
		row->left = g_value_get_float (val) * 10;

	val = g_hash_table_lookup (properties, "row.vertical");
	if (val)
		row->vertical = g_value_get_boolean (val);

	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_LEFT_CURLY, NULL, error);

	while ((token = g_scanner_get_next_token (self->scanner)) != G_TOKEN_EOF) {
		if (g_cancellable_is_cancelled (cancellable)) {
			return NULL;
		}

		cur_val = g_scanner_cur_value (self->scanner);

		switch (token) {
		case G_TOKEN_RIGHT_CURLY:
			return g_steal_pointer (&row);
		case G_TOKEN_IDENTIFIER:
			if (!g_strcmp0 (cur_val.v_identifier, "keys")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_LEFT_CURLY, NULL, error);
				row->keys = parse_key_list (self, geometry, properties, error);
				if (!row->keys && error && *error)
					return NULL;
			} else if (!g_strcmp0 (cur_val.v_identifier, "top")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &row->top, error))
					return NULL;
				row->top *= 10;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "left")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &row->left, error))
					return NULL;
				row->left *= 10;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "vertical")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_IDENTIFIER, NULL, error);
				if (!g_strcmp0 (cur_val.v_identifier, "True")) {
					row->vertical = TRUE;
				} else if (!g_strcmp0 (cur_val.v_identifier, "False")) {
					row->vertical = FALSE;
				} else {
					g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
						     GKBD_GEOMETRY_PARSER_ERROR_FAILED,
						     "Unexpected identifier at %d:%d: %s (expected True or False)",
						     g_scanner_cur_line (self->scanner),
						     g_scanner_cur_position (self->scanner),
						     cur_val.v_identifier);
				}

				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "key")) {
				if (!parse_property (self, "key", properties, error))
					return NULL;
			} else {
				g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
				             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
				             "Unexpected identifier at %d:%d: %s",
				             g_scanner_cur_line (self->scanner),
				             g_scanner_cur_position (self->scanner),
				             cur_val.v_identifier);
				return NULL;
			}
			break;
		default:
			g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
			             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
			             "Unexpected token at %d:%d: '%c'",
			             g_scanner_cur_line (self->scanner),
			             g_scanner_cur_position (self->scanner),
			             token);
			return NULL;
		}
	}

	return g_steal_pointer (&row);
}

static GkbdSection *
parse_section (GkbdGeometryParser  *self,
               GkbdGeometry        *geometry,
               GHashTable          *properties,
               GCancellable        *cancellable,
               GError             **error)
{
	g_autoptr(GkbdSection) section = gkbd_section_new ();
	GTokenType token;
	GTokenValue cur_val;

	GValue *val;
	val = g_hash_table_lookup (properties, "section.top");
	if (val)
		section->top = g_value_get_float (val) * 10;

	val = g_hash_table_lookup (properties, "section.left");
	if (val)
		section->left = g_value_get_float (val) * 10;

	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
	cur_val = g_scanner_cur_value (self->scanner);
	section->name = g_strdup (cur_val.v_string);
	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_LEFT_CURLY, NULL, error);

	while ((token = g_scanner_get_next_token (self->scanner)) != G_TOKEN_EOF) {
		if (g_cancellable_is_cancelled (cancellable)) {
			return NULL;
		}

		cur_val = g_scanner_cur_value (self->scanner);

		switch (token) {
		case G_TOKEN_RIGHT_CURLY:
			section->rows = g_list_reverse (section->rows);
			return g_steal_pointer (&section);
		case G_TOKEN_IDENTIFIER:
			if (!g_strcmp0 (cur_val.v_identifier, "row")) {
				if (g_scanner_peek_next_token (self->scanner) == '.') {
					if (!parse_property (self, "row", properties, error))
						return NULL;
				} else {
					g_autoptr(GHashTable) cloned_table = clone_properties_table (properties);
					GkbdRow *row = parse_row (self, geometry, cloned_table, cancellable, error);
					if (!row)
						return NULL;

					section->rows = g_list_prepend (section->rows, row);
					break;
				}
			} else if (!g_strcmp0 (cur_val.v_identifier, "top")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &section->top, error))
					return NULL;
				section->top *= 10;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "left")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				if (!parse_number (self, &section->left, error))
					return NULL;
				section->left *= 10;
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "key")) {
				if (!parse_property (self, "key", properties, error))
					return NULL;
			} else {
				g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
				             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
				             "Unexpected identifier at %d:%d: %s",
				             g_scanner_cur_line (self->scanner),
				             g_scanner_cur_position (self->scanner),
				             cur_val.v_identifier);
				return NULL;
			}
			break;
		default:
			g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
			             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
			             "Unexpected token at %d:%d: '%c'",
			             g_scanner_cur_line (self->scanner),
			             g_scanner_cur_position (self->scanner),
			             token);
			return NULL;
		}
	}

	section->rows = g_list_reverse (section->rows);
	return g_steal_pointer (&section);
}

static GkbdGeometry *
parse_geometry (GkbdGeometryParser  *self,
                GCancellable        *cancellable,
                GError             **error)
{
	g_autoptr(GkbdGeometry) geometry = gkbd_geometry_new ();
	g_autoptr(GHashTable) properties = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, gkbd_value_free);
	GTokenType token;
	GTokenValue cur_val;

	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
	cur_val = g_scanner_cur_value (self->scanner);
	geometry->name = g_strdup (cur_val.v_string);
	gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_LEFT_CURLY, NULL, error);

	while ((token = g_scanner_get_next_token (self->scanner)) != G_TOKEN_EOF) {
		if (g_cancellable_is_cancelled (cancellable)) {
			return NULL;
		}

		cur_val = g_scanner_cur_value (self->scanner);

		switch (token) {
		case G_TOKEN_RIGHT_CURLY:
			geometry->shapes = g_list_reverse (geometry->shapes);
			geometry->solids = g_list_reverse (geometry->solids);
			geometry->indicators = g_list_reverse (geometry->indicators);
			geometry->sections = g_list_reverse (geometry->sections);
			geometry->aliases = g_list_reverse (geometry->aliases);
			geometry->texts = g_list_reverse (geometry->texts);
			geometry->outlines = g_list_reverse (geometry->outlines);
			return g_steal_pointer (&geometry);
		case G_TOKEN_IDENTIFIER:
			if (!g_strcmp0 (cur_val.v_identifier, "shape")) {
				if (g_scanner_peek_next_token (self->scanner) == '.') {
					if (!parse_property (self, "shape", properties, error))
						return NULL;
				} else {
					g_autoptr(GHashTable) cloned_table = clone_properties_table (properties);
					GkbdShape *shape = parse_shape (self, cloned_table, cancellable, error);
					if (!shape)
						return NULL;

					geometry->shapes = g_list_prepend (geometry->shapes, shape);
					break;
				}
			} else if (!g_strcmp0 (cur_val.v_identifier, "solid")) {
				g_autoptr(GHashTable) cloned_table = clone_properties_table (properties);
				GkbdSolid *solid = parse_solid (self, geometry, FALSE, cloned_table, cancellable, error);
				if (!solid)
					return NULL;

				geometry->solids = g_list_prepend (geometry->solids, solid);
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "indicator")) {
				if (g_scanner_peek_next_token (self->scanner) == '.') {
					if (!parse_property (self, "indicator", properties, error))
						return NULL;
				} else {
					g_autoptr(GHashTable) cloned_table = clone_properties_table (properties);
					GkbdIndicator *indicator = parse_indicator (self, geometry, cloned_table, cancellable, error);
					if (!indicator)
						return NULL;

					geometry->indicators = g_list_prepend (geometry->indicators, indicator);
					break;
				}
			} else if (!g_strcmp0 (cur_val.v_identifier, "row")) {
				if (g_scanner_peek_next_token (self->scanner) == '.') {
					if (!parse_property (self, "row", properties, error))
						return NULL;
				} else {
					g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
						     GKBD_GEOMETRY_PARSER_ERROR_FAILED,
						     "Unexpected identifier at %d:%d: %s",
						     g_scanner_cur_line (self->scanner),
						     g_scanner_cur_position (self->scanner),
						     cur_val.v_identifier);
					return NULL;
				}
			} else if (!g_strcmp0 (cur_val.v_identifier, "section")) {
				if (g_scanner_peek_next_token (self->scanner) == '.') {
					if (!parse_property (self, "section", properties, error))
						return NULL;
				} else {
					g_autoptr(GHashTable) cloned_table = clone_properties_table (properties);
					GkbdSection *section = parse_section (self, geometry, cloned_table, cancellable, error);
					if (!section)
						return NULL;

					geometry->sections = g_list_prepend (geometry->sections, section);
					break;
				}
			} else if (!g_strcmp0 (cur_val.v_identifier, "key")) {
				if (g_scanner_peek_next_token (self->scanner) == '.') {
					if (!parse_property (self, "key", properties, error))
						return NULL;
				} else {
					g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
						     GKBD_GEOMETRY_PARSER_ERROR_FAILED,
						     "Unexpected identifier at %d:%d: %s",
						     g_scanner_cur_line (self->scanner),
						     g_scanner_cur_position (self->scanner),
						     cur_val.v_identifier);
					return NULL;
				}
			} else if (!g_strcmp0 (cur_val.v_identifier, "alias")) {
				g_autoptr(GkbdAlias) alias = gkbd_alias_new ();
				gkbd_error_val_if_wrong_token (self->scanner, '<', NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_IDENTIFIER, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				alias->name = g_strdup (cur_val.v_identifier);
				gkbd_error_val_if_wrong_token (self->scanner, '>', NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, '<', NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_IDENTIFIER, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				alias->real = g_strdup (cur_val.v_identifier);
				gkbd_error_val_if_wrong_token (self->scanner, '>', NULL, error);
				geometry->aliases = g_list_prepend (geometry->aliases, g_steal_pointer (&alias));
			} else if (!g_strcmp0 (cur_val.v_identifier, "text")) {
				if (g_scanner_peek_next_token (self->scanner) == '.') {
					if (!parse_property (self, "text", properties, error))
						return NULL;
				} else {
					g_autoptr(GHashTable) cloned_table = clone_properties_table (properties);
					GkbdText *text = parse_text (self, geometry, cloned_table, cancellable, error);
					if (!text)
						return NULL;

					geometry->texts = g_list_prepend (geometry->texts, text);
					break;
				}
			} else if (!g_strcmp0 (cur_val.v_identifier, "outline")) {
				g_autoptr(GHashTable) cloned_table = clone_properties_table (properties);
				GkbdSolid *outline = parse_solid (self, geometry, TRUE, cloned_table, cancellable, error);
				if (!outline)
					return NULL;

				outline->is_outline = TRUE;
				geometry->outlines = g_list_prepend (geometry->outlines, outline);
				break;
			} else if (!g_strcmp0 (cur_val.v_identifier, "description")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				geometry->description = g_strdup (cur_val.v_string);
			} else if (!g_strcmp0 (cur_val.v_identifier, "width")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_INT, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				geometry->width_mm = cur_val.v_int * 10;
			} else if (!g_strcmp0 (cur_val.v_identifier, "height")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_INT, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				geometry->height_mm = cur_val.v_int * 10;
			} else if (!g_strcmp0 (cur_val.v_identifier, "baseColor")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				geometry->base_color = find_color (geometry, cur_val.v_string);
			} else if (!g_strcmp0 (cur_val.v_identifier, "labelColor")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				geometry->label_color = find_color (geometry, cur_val.v_string);
			} else if (!g_strcmp0 (cur_val.v_identifier, "xfont")) {
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_EQUAL_SIGN, NULL, error);
				gkbd_error_val_if_wrong_token (self->scanner, G_TOKEN_STRING, NULL, error);
				cur_val = g_scanner_cur_value (self->scanner);
				geometry->label_font = g_strdup (cur_val.v_string);
			} else {
				g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
				             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
				             "Unexpected identifier at %d:%d: %s",
				             g_scanner_cur_line (self->scanner),
				             g_scanner_cur_position (self->scanner),
				             cur_val.v_identifier);
				return NULL;
			}
			break;
		default:
			g_set_error (error, GKBD_GEOMETRY_PARSER_ERROR,
			             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
			             "Unexpected token at %d:%d: '%c'",
			             g_scanner_cur_line (self->scanner),
			             g_scanner_cur_position (self->scanner),
			             token);
			return NULL;
		}
	}

	geometry->shapes = g_list_reverse (geometry->shapes);
	geometry->solids = g_list_reverse (geometry->solids);
	geometry->indicators = g_list_reverse (geometry->indicators);
	geometry->sections = g_list_reverse (geometry->sections);
	geometry->aliases = g_list_reverse (geometry->aliases);
	geometry->texts = g_list_reverse (geometry->texts);
	geometry->outlines = g_list_reverse (geometry->outlines);
	return g_steal_pointer (&geometry);
}

static void
parse_file_thread (GTask *task,
                   gpointer source_object,
                   gpointer task_data,
                   GCancellable *cancellable)
{
	GkbdGeometryParser *self = source_object;
	GList *geometry_list = NULL;
	const char *file = task_data;
	GTokenType token;
	int fd;

	fd = g_open (file, O_RDONLY, 0);
	if (fd == -1) {
		int fderror = errno;
		g_task_return_new_error (task, G_FILE_ERROR, g_file_error_from_errno (fderror), "Error opening %s: %s", file, strerror (fderror));
		return;
	}

	g_scanner_input_file (self->scanner, fd);
	g_scanner_set_scope (self->scanner, GKBD_GEOMETRY_SCOPE_ROOT);
	while ((token = g_scanner_get_next_token (self->scanner)) != G_TOKEN_EOF) {
		if (g_cancellable_is_cancelled (cancellable)) {
			g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_CANCELLED, "Scan cancelled");
			return;
		}

		GTokenValue cur_val = g_scanner_cur_value (self->scanner);

		switch (token) {
		case G_TOKEN_IDENTIFIER:
			if (!g_strcmp0 (cur_val.v_identifier, "default")) {
				//
			} else if (!g_strcmp0 (cur_val.v_identifier, "xkb_geometry")) {
				GError *error = NULL;
				GkbdGeometry *geometry = parse_geometry (self, cancellable, &error);
				if (!geometry) {
					if (!g_task_return_error_if_cancelled (task))
						g_task_return_error (task, error);
					return;
				}

				geometry_list = g_list_prepend (geometry_list, geometry);
			} else {
				g_task_return_new_error (task, GKBD_GEOMETRY_PARSER_ERROR,
				             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
				             "Unexpected identifier at %d:%d: %s",
				             g_scanner_cur_line (self->scanner),
				             g_scanner_cur_position (self->scanner),
				             cur_val.v_identifier);
				return;
			}
			break;
		default:
			g_task_return_new_error (task, GKBD_GEOMETRY_PARSER_ERROR,
			             GKBD_GEOMETRY_PARSER_ERROR_FAILED,
			             "Unexpected token at %d:%d: '%c'",
			             g_scanner_cur_line (self->scanner),
			             g_scanner_cur_position (self->scanner),
			             token);
			return;
		}
	}

	g_task_return_pointer (task, g_list_reverse (g_steal_pointer (&geometry_list)), (GDestroyNotify) gkbd_geometry_unref);
}

void
gkbd_geometry_parser_parse_file (GkbdGeometryParser  *self,
                                 const char          *file,
                                 GCancellable        *cancellable,
                                 GAsyncReadyCallback  callback,
                                 gpointer             user_data)
{
	g_autoptr(GTask) task = NULL;

	g_return_if_fail (GKBD_GEOMETRY_PARSER (self));

	task = g_task_new (self, cancellable, callback, user_data);
	g_task_set_source_tag (task, gkbd_geometry_parser_parse_file);
	g_task_set_task_data (task, g_strdup (file), g_free);
	g_task_run_in_thread (task, parse_file_thread);

}

GList *
gkbd_geometry_parser_parse_file_finish (GkbdGeometryParser  *self,
                                        GAsyncResult        *result,
                                        GError             **error)
{
	g_return_val_if_fail (g_task_is_valid (result, self), NULL);

	return g_task_propagate_pointer (G_TASK (result), error);
}

GkbdGeometry *
gkbd_geometry_ref (GkbdGeometry *geometry)
{
	if (!geometry)
		return NULL;

	g_atomic_ref_count_inc (&geometry->ref_count);
	return geometry;
}

void
gkbd_geometry_unref (GkbdGeometry *geometry)
{
	if (!geometry)
		return;

	if (g_atomic_ref_count_dec (&geometry->ref_count)) {
		g_clear_pointer (&geometry->name, g_free);
		g_clear_pointer (&geometry->description, g_free);
		g_clear_pointer (&geometry->label_font, g_free);
		g_list_free_full (g_steal_pointer (&geometry->shapes), (GDestroyNotify) gkbd_shape_unref);
		g_list_free_full (g_steal_pointer (&geometry->colors), (GDestroyNotify) gkbd_color_unref);
		g_list_free_full (g_steal_pointer (&geometry->indicators), (GDestroyNotify) gkbd_indicator_unref);
		g_list_free_full (g_steal_pointer (&geometry->texts), (GDestroyNotify) gkbd_text_unref);
		g_list_free_full (g_steal_pointer (&geometry->sections), (GDestroyNotify) gkbd_section_unref);
		g_list_free_full (g_steal_pointer (&geometry->solids), (GDestroyNotify) gkbd_solid_unref);
		g_list_free_full (g_steal_pointer (&geometry->outlines), (GDestroyNotify) gkbd_solid_unref);
		g_list_free_full (g_steal_pointer (&geometry->aliases), (GDestroyNotify) gkbd_alias_unref);
		g_free (geometry);
	}
}

const char *
gkbd_geometry_get_name (GkbdGeometry *geometry)
{
	g_return_val_if_fail (geometry != NULL, NULL);

	return geometry->name;
}

const char *
gkbd_geometry_get_description (GkbdGeometry *geometry)
{
	g_return_val_if_fail (geometry != NULL, NULL);

	return geometry->description;
}

float
gkbd_geometry_get_width_mm (GkbdGeometry *geometry)
{
	g_return_val_if_fail (geometry != NULL, 0.f);

	return geometry->width_mm;
}

float
gkbd_geometry_get_height_mm (GkbdGeometry *geometry)
{
	g_return_val_if_fail (geometry != NULL, 0.f);

	return geometry->height_mm;
}

GkbdColor *
gkbd_geometry_get_label_color (GkbdGeometry *geometry)
{
	g_return_val_if_fail (geometry != NULL, NULL);

	return geometry->label_color;
}

GkbdColor *
gkbd_geometry_get_base_color (GkbdGeometry *geometry)
{
	g_return_val_if_fail (geometry != NULL, NULL);

	if (!geometry->base_color)
		return NULL;

	return geometry->base_color;
}

GList *
gkbd_geometry_get_aliases (GkbdGeometry *geometry)
{
	g_return_val_if_fail (geometry != NULL, NULL);

	return geometry->aliases;
}

GList *
gkbd_geometry_get_sections (GkbdGeometry *geometry)
{
	g_return_val_if_fail (geometry != NULL, NULL);

	return geometry->sections;
}

GList *
gkbd_geometry_get_solids (GkbdGeometry *geometry)
{
	g_return_val_if_fail (geometry != NULL, NULL);

	return geometry->solids;
}

GList *
gkbd_geometry_get_indicators (GkbdGeometry *geometry)
{
	g_return_val_if_fail (geometry != NULL, NULL);

	return geometry->indicators;
}

GList *
gkbd_geometry_get_texts (GkbdGeometry *geometry)
{
	g_return_val_if_fail (geometry != NULL, NULL);

	return geometry->texts;
}

GList *
gkbd_geometry_get_colors (GkbdGeometry *geometry)
{
	g_return_val_if_fail (geometry != NULL, NULL);

	return geometry->colors;
}

GkbdShape *
gkbd_shape_ref (GkbdShape *shape)
{
	if (!shape)
		return NULL;

	g_atomic_ref_count_inc (&shape->ref_count);
	return shape;
}

void
gkbd_shape_unref (GkbdShape *shape)
{
	if (!shape)
		return;

	if (g_atomic_ref_count_dec (&shape->ref_count)) {
		g_list_free_full (g_steal_pointer (&shape->outlines), (GDestroyNotify) gkbd_shape_outline_unref);
		g_clear_pointer (&shape->name, g_free);
		g_free (shape);
	}
}

GList *
gkbd_shape_get_outlines (GkbdShape *shape)
{
	g_return_val_if_fail (shape != NULL, NULL);

	return shape->outlines;
}

GkbdShapeOutline *
gkbd_shape_get_approx (GkbdShape *shape)
{
	g_return_val_if_fail (shape != NULL, NULL);

	return shape->approx;
}

GkbdShapeOutline *
gkbd_shape_get_primary (GkbdShape *shape)
{
	g_return_val_if_fail (shape != NULL, NULL);

	return shape->primary;
}

void
gkbd_shape_get_bounds (GkbdShape       *shape,
                       graphene_rect_t *bounds)
{
	g_return_if_fail (shape != NULL);
	graphene_rect_init_from_rect (bounds, &shape->bounds);
}

GkbdShapeOutline *
gkbd_shape_outline_ref (GkbdShapeOutline *shape_outline)
{
	if (!shape_outline)
		return NULL;

	g_atomic_ref_count_inc (&shape_outline->ref_count);
	return shape_outline;
}

void
gkbd_shape_outline_unref (GkbdShapeOutline *shape_outline)
{
	if (!shape_outline)
		return;

	if (g_atomic_ref_count_dec (&shape_outline->ref_count)) {
		g_list_free_full (g_steal_pointer (&shape_outline->points), (GDestroyNotify) graphene_point_free);
		g_free (shape_outline);
	}
}

GList *
gkbd_shape_outline_get_points (GkbdShapeOutline *shape_outline)
{
	g_return_val_if_fail (shape_outline != NULL, NULL);

	return shape_outline->points;
}

float
gkbd_shape_outline_get_corner_radius (GkbdShapeOutline *shape_outline)
{
	g_return_val_if_fail (shape_outline != NULL, 0.f);

	return shape_outline->corner_radius;
}

GkbdSolid *
gkbd_solid_ref (GkbdSolid *solid)
{
	if (!solid)
		return NULL;

	g_atomic_ref_count_inc (&solid->ref_count);
	return solid;
}

void
gkbd_solid_unref (GkbdSolid *solid)
{
	if (!solid)
		return;

	if (g_atomic_ref_count_dec (&solid->ref_count)) {
		g_clear_pointer (&solid->name, g_free);
		g_free (solid);
	}
}

const char *
gkbd_solid_get_name (GkbdSolid *solid)
{
	g_return_val_if_fail (solid != NULL, NULL);

	return solid->name;
}

guint
gkbd_solid_get_priority (GkbdSolid *solid)
{
	g_return_val_if_fail (solid != NULL, 0);

	return solid->priority;
}

float
gkbd_solid_get_top (GkbdSolid *solid)
{
	g_return_val_if_fail (solid != NULL, 0.f);

	return solid->top;
}

float
gkbd_solid_get_left (GkbdSolid *solid)
{
	g_return_val_if_fail (solid != NULL, 0.f);

	return solid->left;
}

float
gkbd_solid_get_angle (GkbdSolid *solid)
{
	g_return_val_if_fail (solid != NULL, 0.f);

	return solid->angle;
}

GkbdColor *
gkbd_solid_get_color (GkbdSolid *solid)
{
	g_return_val_if_fail (solid != NULL, NULL);

	return solid->color;
}

GkbdShape *
gkbd_solid_get_shape (GkbdSolid *solid)
{
	g_return_val_if_fail (solid != NULL, NULL);

	return solid->shape;
}

GkbdColor *
gkbd_color_ref (GkbdColor *color)
{
	if (!color)
		return NULL;

	g_atomic_ref_count_inc (&color->ref_count);
	return color;
}

void
gkbd_color_unref (GkbdColor *color)
{
	if (!color)
		return;

	if (g_atomic_ref_count_dec (&color->ref_count)) {
		g_clear_pointer (&color->name, g_free);
		g_free (color);
	}
}

const char *
gkbd_color_get_name (GkbdColor *color)
{
	g_return_val_if_fail (color != NULL, NULL);

	return color->name;
}

GkbdIndicator *
gkbd_indicator_ref (GkbdIndicator *indicator)
{
	if (!indicator)
		return NULL;

	g_atomic_ref_count_inc (&indicator->ref_count);
	return indicator;
}

void
gkbd_indicator_unref (GkbdIndicator *indicator)
{
	if (!indicator)
		return;

	if (g_atomic_ref_count_dec (&indicator->ref_count)) {
		g_clear_pointer (&indicator->name, g_free);
		g_free (indicator);
	}
}

const char *
gkbd_indicator_get_name (GkbdIndicator *indicator)
{
	g_return_val_if_fail (indicator != NULL, NULL);

	return indicator->name;
}

guint
gkbd_indicator_get_priority (GkbdIndicator *indicator)
{
	g_return_val_if_fail (indicator != NULL, 0);

	return indicator->priority;
}

float
gkbd_indicator_get_top (GkbdIndicator *indicator)
{
	g_return_val_if_fail (indicator != NULL, 0.f);

	return indicator->top;
}

float
gkbd_indicator_get_left (GkbdIndicator *indicator)
{
	g_return_val_if_fail (indicator != NULL, 0.f);

	return indicator->left;
}

float
gkbd_indicator_get_angle (GkbdIndicator *indicator)
{
	g_return_val_if_fail (indicator != NULL, 0.f);

	return indicator->angle;
}

GkbdShape *
gkbd_indicator_get_shape (GkbdIndicator *indicator)
{
	g_return_val_if_fail (indicator != NULL, NULL);

	return indicator->shape;
}

GkbdColor *
gkbd_indicator_get_on_color (GkbdIndicator *indicator)
{
	g_return_val_if_fail (indicator != NULL, NULL);

	return indicator->on_color;
}

GkbdColor *
gkbd_indicator_get_off_color (GkbdIndicator *indicator)
{
	g_return_val_if_fail (indicator != NULL, NULL);

	return indicator->off_color;
}

GkbdText *
gkbd_text_ref (GkbdText *text)
{
	if (!text)
		return NULL;

	g_atomic_ref_count_inc (&text->ref_count);
	return text;
}

void
gkbd_text_unref (GkbdText *text)
{
	if (!text)
		return;

	if (g_atomic_ref_count_dec (&text->ref_count)) {
		g_clear_pointer (&text->text, g_free);
		g_clear_pointer (&text->name, g_free);
		g_free (text);
	}
}

const char *
gkbd_text_get_name (GkbdText *text)
{
	g_return_val_if_fail (text != NULL, NULL);

	return text->name;
}

guint
gkbd_text_get_priority (GkbdText *text)
{
	g_return_val_if_fail (text != NULL, 0);

	return text->priority;
}

float
gkbd_text_get_top (GkbdText *text)
{
	g_return_val_if_fail (text != NULL, 0.f);

	return text->top;
}

float
gkbd_text_get_left (GkbdText *text)
{
	g_return_val_if_fail (text != NULL, 0.f);

	return text->left;
}

float
gkbd_text_get_angle (GkbdText *text)
{
	g_return_val_if_fail (text != NULL, 0.f);

	return text->angle;
}

float
gkbd_text_get_width (GkbdText *text)
{
	g_return_val_if_fail (text != NULL, 0.f);

	return text->width;
}

float
gkbd_text_get_height (GkbdText *text)
{
	g_return_val_if_fail (text != NULL, 0.f);

	return text->height;
}

GkbdColor *
gkbd_text_get_color (GkbdText *text)
{
	g_return_val_if_fail (text != NULL, NULL);

	return text->color;
}

const char *
gkbd_text_get_text (GkbdText *text)
{
	g_return_val_if_fail (text != NULL, NULL);

	return text->text;
}

const char *
gkbd_text_get_font (GkbdText *text)
{
	g_return_val_if_fail (text != NULL, NULL);

	return text->font;
}

GkbdSection *
gkbd_section_ref (GkbdSection *section)
{
	if (!section)
		return NULL;

	g_atomic_ref_count_inc (&section->ref_count);
	return section;
}

void
gkbd_section_unref (GkbdSection *section)
{
	if (!section)
		return;

	if (g_atomic_ref_count_dec (&section->ref_count)) {
		g_clear_pointer (&section->name, g_free);
		g_list_free_full (g_steal_pointer (&section->rows), (GDestroyNotify) gkbd_row_unref);
		g_free (section);
	}
}

const char *
gkbd_section_get_name (GkbdSection *section)
{
	g_return_val_if_fail (section != NULL, NULL);

	return section->name;
}

GList *
gkbd_section_get_rows (GkbdSection *section)
{
	g_return_val_if_fail (section != NULL, NULL);

	return section->rows;
}

float
gkbd_section_get_top (GkbdSection *section)
{
	g_return_val_if_fail (section != NULL, 0.f);

	return section->top;
}

float
gkbd_section_get_left (GkbdSection *section)
{
	g_return_val_if_fail (section != NULL, 0.f);

	return section->left;
}

float
gkbd_section_get_angle (GkbdSection *section)
{
	g_return_val_if_fail (section != NULL, 0.f);

	return section->angle;
}

guint
gkbd_section_get_priority (GkbdSection *section)
{
	g_return_val_if_fail (section != NULL, 0);

	return section->priority;
}

GkbdRow *
gkbd_row_ref (GkbdRow *row)
{
	if (!row)
		return NULL;

	g_atomic_ref_count_inc (&row->ref_count);
	return row;
}

void
gkbd_row_unref (GkbdRow *row)
{
	if (!row)
		return;

	if (g_atomic_ref_count_dec (&row->ref_count)) {
		g_list_free_full (g_steal_pointer (&row->keys), (GDestroyNotify) gkbd_key_unref);
		g_free (row);
	}
}

GList *
gkbd_row_get_keys (GkbdRow *row)
{
	g_return_val_if_fail (row != NULL, NULL);

	return row->keys;
}

float
gkbd_row_get_top (GkbdRow *row)
{
	g_return_val_if_fail (row != NULL, 0.f);

	return row->top;
}

float
gkbd_row_get_left (GkbdRow *row)
{
	g_return_val_if_fail (row != NULL, 0.f);

	return row->left;
}

gboolean
gkbd_row_get_vertical (GkbdRow *row)
{
	g_return_val_if_fail (row != NULL, FALSE);

	return row->vertical;
}

GkbdKey *
gkbd_key_ref (GkbdKey *key)
{
	if (!key)
		return NULL;

	g_atomic_ref_count_inc (&key->ref_count);
	return key;
}

void
gkbd_key_unref (GkbdKey *key)
{
	if (!key)
		return;

	if (g_atomic_ref_count_dec (&key->ref_count)) {
		g_clear_pointer (&key->name, g_free);
		g_free (key);
	}
}

const char *
gkbd_key_get_name (GkbdKey *key)
{
	g_return_val_if_fail (key != NULL, NULL);

	return key->name;
}

float
gkbd_key_get_gap (GkbdKey *key)
{
	g_return_val_if_fail (key != NULL, 0.f);

	return key->gap;
}

GkbdShape *
gkbd_key_get_shape (GkbdKey *key)
{
	g_return_val_if_fail (key != NULL, NULL);

	return key->shape;
}

GkbdAlias *
gkbd_alias_ref (GkbdAlias *alias)
{
	if (!alias)
		return NULL;

	g_atomic_ref_count_inc (&alias->ref_count);
	return alias;
}

void
gkbd_alias_unref (GkbdAlias *alias)
{
	if (!alias)
		return;

	if (g_atomic_ref_count_dec (&alias->ref_count)) {
		g_clear_pointer (&alias->name, g_free);
		g_clear_pointer (&alias->real, g_free);
		g_free (alias);
	}
}
