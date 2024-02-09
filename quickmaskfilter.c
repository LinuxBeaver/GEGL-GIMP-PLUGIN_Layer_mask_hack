/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright 2006 Øyvind Kolås <pippin@gimp.org>
 * 2024 Beaver Layer Mask Filter 

This filter was inspired by "Refine Edges" in Photoshop CS5 and Mr Q's live preview selection plugin on Gimp chat 

You can test this filter by pasting this code into GEGL Graph (ITS VERY SIMPLE)
noise-spread amount-x=0 amount-y=0 seed=32402130
median-blur radius=0 alpha-percentile=100 
gaussian-blur std-dev-x=2 std-dev-y=2
median-blur radius=0
levels out-high=1
component-extract invert=true
levels out-high=10
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES


property_boolean (invert, _("Invert Mask"), FALSE)
     description (_("Invert the layer mask or quick mask"))


property_double (feather, _("Blur to Feather  (use with high content opacity) "), 0)
   description (_("Gaussian Blur will feather the selection when used appropriately with high content opacity. Works best when opacity is at 3 to 5. "))
   value_range (0.0, 20.0)
   ui_range    (0.24, 20.0)
   ui_gamma    (3.0)

enum_start (gegl_gimplayershadow_grow_shape2)
  enum_value (GEGL_DROPSHADOW_GROW_SHAPE_SQUARE,  "square",  N_("Square"))
  enum_value (GEGL_DROPSHADOW_GROW_SHAPE_CIRCLE,  "circle",  N_("Circle"))
  enum_value (GEGL_DROPSHADOW_GROW_SHAPE_DIAMOND, "diamond", N_("Diamond"))
enum_end (GeglgimplayershadowGrowShape2)


property_enum   (growshrinkshape, _("Grow to Shrink base shape"),
                 GeglgimplayershadowGrowShape2, gegl_gimplayershadow_grow_shape2,
                 GEGL_DROPSHADOW_GROW_SHAPE_CIRCLE)
  description   (_("The base shape to expand or contract the selection in"))


property_int (growshrink, _("Grow to Shrink"), 0)
  value_range   (-50, 50)
  ui_range      (-15, 15)
  ui_gamma      (1.5)
  ui_meta       ("unit", "pixel-distance")
  description (_("The distance to Grow or Shrink the selection. Left grows and right shrinks."))

property_double (growshrinkpercentile, _("Median Percentile of Grow to Shrink"), 100)
   description (_("At lower values this will make grow shrink cover area of the subject/object."))
   value_range (50.0, 100.0)
   ui_range    (50.0, 100.0)
   ui_gamma    (3.0)

property_int (distort, _("Noise Spread to Distort"), 0)
   description (_("Noise Spread will distort the selection. Works best when blur is very mild or disabled."))
   value_range (0, 50)
   ui_range    (0, 30)
   ui_gamma    (3.0)

property_seed (distortseed, _("Random seed of Distort effect"), rand)

property_int (smooth, _("Default Median Blur to Smooth Selection"), 0)
  value_range   (-20, 20)
  ui_range      (-20, 20)
  ui_gamma      (1.5)
  description (_("This makes the selection more round in appearance"))


property_double (opacity, _("Opacity of the Content"), 1.0)
   description (_("Decrease or increase the opacity of the subject/object within the mask using levels. 1 is default, 0 is completely transparent and 3 is triple the default opacity."))
   value_range (0.0, 5.0)
   ui_range    (0.0, 3.0)
   ui_gamma    (3.0)

property_double (opacitymask, _("Opacity of the Mask"), 0.0)
   description (_("Increase to remove opacity of the mask. At 0 the mask has full opacity"))
   value_range (0.0, 1.0)
   ui_range    (0.0, 1.0)
   ui_gamma    (3.0)

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     quickmaskfilter
#define GEGL_OP_C_SOURCE quickmaskfilter.c

#include "gegl-op.h"

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;

  GeglNode *input  = gegl_node_get_input_proxy (gegl, "input");
  GeglNode *output = gegl_node_get_output_proxy (gegl, "output");


   GeglNode *spread   = gegl_node_new_child (gegl,
                                  "operation", "gegl:noise-spread",  NULL);

    GeglNode *blur      = gegl_node_new_child (gegl, "operation", "gegl:gaussian-blur", NULL);

    GeglNode *median     = gegl_node_new_child (gegl, "operation", "gegl:median-blur", NULL);

    GeglNode *median2     = gegl_node_new_child (gegl, "operation", "gegl:median-blur",  NULL);

    GeglNode *invertmask     = gegl_node_new_child (gegl, "operation", "gegl:component-extract",  NULL);
                                                                                
    GeglNode *increaseopacity     = gegl_node_new_child (gegl, "operation", "gegl:levels", NULL);
                           
  gegl_node_link_many (input, spread, median, blur, median2, invertmask, increaseopacity,   output, NULL);

  gegl_operation_meta_redirect (operation, "invert", invertmask, "invert");
  gegl_operation_meta_redirect (operation, "opacity", increaseopacity, "out-high");
  gegl_operation_meta_redirect (operation, "opacitymask", increaseopacity, "out-low");
  gegl_operation_meta_redirect (operation, "distort", spread, "amount-y");
  gegl_operation_meta_redirect (operation, "distort", spread, "amount-x");
  gegl_operation_meta_redirect (operation, "distortseed", spread, "seed");
  gegl_operation_meta_redirect (operation, "feather", blur, "std-dev-x");
  gegl_operation_meta_redirect (operation, "feather", blur, "std-dev-y");
  gegl_operation_meta_redirect (operation, "growshrink", median, "radius");
  gegl_operation_meta_redirect (operation, "growshrinkpercentile", median, "percentile");
  gegl_operation_meta_redirect (operation, "growshrinkshape", median, "neighborhood");
  gegl_operation_meta_redirect (operation, "smooth", median2, "radius");

}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;


  gegl_operation_class_set_keys (operation_class,
    "name",        "lb:modifyselection",
    "title",       _("Modify Selection (apply on layer mask)"),
    "reference-hash", "consciousnesscreatesreality0",
    "description", _("Apply this filter on a quick mask or layer mask to refine edeges of the selection. Fact 1: GEGL can apply filters on quick mask alone, but if you click on a Gimp layer it will then apply said effect to the layer you clicked. You can re-instruct GEGL to apply the filter on the quick mask by disabling and re-enabling the quick mask. Hint: When used unintendedly this filter can make a stained glass effect."
                     ""),
    "gimp:menu-path", "<Image>/Select/",
    "gimp:menu-label", _("Modify Selection - Layer Mask Filter."),
    NULL);
}

#endif
