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
 */

#ifndef NSA_FINEPAINTER_H
#define NSA_FINEPAINTER_H

#include "../config.h"

#include <gdk/gdk.h>
#include <sys/types.h>
#include <pango/pango-layout.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#include "nsapainter.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NSA_TYPE_FINE_PAINTER             (nsa_fine_painter_get_type ())
#define NSA_FINE_PAINTER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NSA_TYPE_FINE_PAINTER, NsaFinePainter))
#define NSA_FINE_PAINTER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NSA_TYPE_FINE_PAINTER, NsaFinePainterClass))
#define NSA_IS_FINE_PAINTER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NSA_TYPE_FINE_PAINTER))
#define NSA_IS_FINE_PAINTER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NSA_TYPE_FINE_PAINTER))
#define NSA_FINE_PAINTER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NSA_TYPE_FINE_PAINTER, NsaFinePainterClass))

typedef struct _NsaFinePainter       NsaFinePainter;
typedef struct _NsaFinePainterClass  NsaFinePainterClass;

struct _NsaFinePainter
{
    NsaPainter painter;
    
    struct {
	struct pixdata_t {
	    GdkPixdata pixdata;
	    GdkPixbuf *pixbuf;
	} bg, fg, hbar, text1, text2, text3, text;
    } layers;
    
    GdkColor black, white;
};

struct _NsaFinePainterClass
{
    NsaPainterClass parent_class;
};


GType       nsa_fine_painter_get_type	(void) G_GNUC_CONST;

NsaPainter *nsa_fine_painter_new	(NsaIface *iface);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NSA_PAINTER_H */
