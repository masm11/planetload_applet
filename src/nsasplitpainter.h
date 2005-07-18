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
 * $Id: nsasplitpainter.h 21 2005-07-18 07:57:00Z masm $
 */

#ifndef NSA_SPLITPAINTER_H
#define NSA_SPLITPAINTER_H

#include <gdk/gdk.h>
#include <sys/types.h>
#include <pango/pango-layout.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#include "nsapainter.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NSA_TYPE_SPLIT_PAINTER             (nsa_split_painter_get_type ())
#define NSA_SPLIT_PAINTER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NSA_TYPE_SPLIT_PAINTER, NsaSplitPainter))
#define NSA_SPLIT_PAINTER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NSA_TYPE_SPLIT_PAINTER, NsaSplitPainterClass))
#define NSA_IS_SPLIT_PAINTER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NSA_TYPE_SPLIT_PAINTER))
#define NSA_IS_SPLIT_PAINTER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NSA_TYPE_SPLIT_PAINTER))
#define NSA_SPLIT_PAINTER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NSA_TYPE_SPLIT_PAINTER, NsaSplitPainterClass))

typedef struct _NsaSplitPainter       NsaSplitPainter;
typedef struct _NsaSplitPainterClass  NsaSplitPainterClass;

struct _NsaSplitPainter
{
    NsaPainter painter;
    
    struct {
	struct nsa_split_painter_pixdata_t {
	    GdkPixdata pixdata;
	    GdkPixbuf *pixbuf;
	} bg, fg, hbar, text1, text2, text3, text;
    } layers;
    
    GdkColor black, white;
};

struct _NsaSplitPainterClass
{
    NsaPainterClass parent_class;
};


GType       nsa_split_painter_get_type	(void) G_GNUC_CONST;

NsaPainter *nsa_split_painter_new	(NsaIface *iface);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NSA_PAINTER_H */
