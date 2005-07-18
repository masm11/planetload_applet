/* PlanetloadApplet
 *  Copyright (C) 2003-2005 Yuuki Harano
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * 
 * $Id: nsalightpainter.h 24 2005-07-18 08:12:27Z masm $
 */

#ifndef NSA_LIGHTPAINTER_H
#define NSA_LIGHTPAINTER_H

#include "../config.h"

#include <gdk/gdk.h>
#include <sys/types.h>
#include <pango/pango-layout.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#include "nsapainter.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NSA_TYPE_LIGHT_PAINTER             (nsa_light_painter_get_type ())
#define NSA_LIGHT_PAINTER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NSA_TYPE_LIGHT_PAINTER, NsaLightPainter))
#define NSA_LIGHT_PAINTER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NSA_TYPE_LIGHT_PAINTER, NsaLightPainterClass))
#define NSA_IS_LIGHT_PAINTER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NSA_TYPE_LIGHT_PAINTER))
#define NSA_IS_LIGHT_PAINTER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NSA_TYPE_LIGHT_PAINTER))
#define NSA_LIGHT_PAINTER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NSA_TYPE_LIGHT_PAINTER, NsaLightPainterClass))

typedef struct _NsaLightPainter       NsaLightPainter;
typedef struct _NsaLightPainterClass  NsaLightPainterClass;

struct _NsaLightPainter
{
    NsaPainter painter;
    
    GdkPixmap *pixmap;
    GdkBitmap *mask;
    GdkGC *gc_0, *gc_1;
    GdkGC *gc[COL_NR], *gc_fg_mid;
    GdkGC *gc_copy_pix, *gc_copy_mask;
    GdkGC *gc_copy;
    PangoLayout *label_layout;
    
    GdkColor text_colors[3];	// 0:down, 1:up, 2:noaddr
    GdkColor *curr_text_color;
    
    struct color_t color_fg_mid;
};

struct _NsaLightPainterClass
{
    NsaPainterClass parent_class;
};


GType       nsa_light_painter_get_type	(void) G_GNUC_CONST;

NsaPainter *nsa_light_painter_new	(NsaIface *iface);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NSA_PAINTER_H */
