/* PlanetloadApplet
 *  Copyright (C) 2003-2007 Yuuki Harano
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

#ifndef NSA_PAINTER_H
#define NSA_PAINTER_H

#include "../config.h"

#include <gdk/gdk.h>
#include <sys/types.h>
#include <gtk/gtkobject.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NSA_TYPE_PAINTER                  (nsa_painter_get_type ())
#define NSA_PAINTER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NSA_TYPE_PAINTER, NsaPainter))
#define NSA_PAINTER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), NSA_TYPE_PAINTER, NsaPainterClass))
#define NSA_IS_PAINTER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NSA_TYPE_PAINTER))
#define NSA_IS_PAINTER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), NSA_TYPE_PAINTER))
#define NSA_PAINTER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), NSA_TYPE_PAINTER, NsaPainterClass))

enum {
    NSA_PAINTER_TYPE_NONE = -1,
    
    NSA_PAINTER_TYPE_FINE = 0,
    NSA_PAINTER_TYPE_LIGHT,
    NSA_PAINTER_TYPE_SPLIT
};

typedef struct _NsaPainter       NsaPainter;
typedef struct _NsaPainterClass  NsaPainterClass;

#include "nsaiface.h"

struct _NsaPainter
{
    GtkObject object;
    
    NsaIface *iface;
};

struct _NsaPainterClass
{
    GtkObjectClass parent_class;
    
    void (*paint)(NsaPainter *painter, GdkRectangle *area);
    void (*repaint)(NsaPainter *painter);
    void (*shift_paint)(NsaPainter *painter);
};


GType nsa_painter_get_type     (void) G_GNUC_CONST;
void  nsa_painter_paint        (NsaPainter *painter, GdkRectangle *area);
void  nsa_painter_repaint      (NsaPainter *painter);
void  nsa_painter_shift_paint  (NsaPainter *painter);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NSA_PAINTER_H */
