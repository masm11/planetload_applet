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
 * $Id: nsapainter.c 34 2005-07-18 09:57:54Z masm $
 */

#include "../config.h"

#include <string.h>
#include <math.h>
#include "debug.h"
#include "nsapainter.h"

#include "i18n-support.h"
enum {
    PROP_0,
    PROP_IFACE
};

static void nsa_painter_class_init(NsaPainterClass *klass);
static void nsa_painter_init(NsaPainter *painter);
static GObject *nsa_painter_constructor(
	GType type,
	guint n_construct_properties,
	GObjectConstructParam *construct_properties);
static void nsa_painter_set_property(
	GObject *object,
	guint prop_id, const GValue *value, GParamSpec *pspec);
static void nsa_painter_get_property(
	GObject *object,
	guint prop_id, GValue *value, GParamSpec *pspec);
static void nsa_painter_destroy(GtkObject *object);
static void nsa_painter_finalize(GObject *object);

static GtkObjectClass *parent_class = NULL;

GType nsa_painter_get_type(void)
{
    static GType painter_type = 0;
    
    if (!painter_type) {
	static const GTypeInfo painter_info = {
	    sizeof (NsaPainterClass),
	    NULL,			/* base_init */
	    NULL,			/* base_finalize */
	    (GClassInitFunc) nsa_painter_class_init,
	    NULL,			/* class_finalize */
	    NULL,			/* class_data */
	    sizeof (NsaPainter),
	    0,				/* n_preallocs */
	    (GInstanceInitFunc) nsa_painter_init,
	};
	
	painter_type = g_type_register_static(
		GTK_TYPE_OBJECT, "NsaPainter", &painter_info,
		G_TYPE_FLAG_ABSTRACT);
    }
    
    return painter_type;
}

static void nsa_painter_class_init(NsaPainterClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkObjectClass *object_class = GTK_OBJECT_CLASS(klass);
    
    parent_class = g_type_class_peek_parent(klass);
    
    gobject_class->finalize = nsa_painter_finalize;
    gobject_class->constructor = nsa_painter_constructor;
    gobject_class->set_property = nsa_painter_set_property;
    gobject_class->get_property = nsa_painter_get_property;
    
    object_class->destroy = nsa_painter_destroy;
    
    klass->paint = NULL;
    klass->repaint = NULL;
    klass->shift_paint = NULL;
    
    g_object_class_install_property(
	    gobject_class,
	    PROP_IFACE,
	    g_param_spec_pointer("iface",
		    _("NsaIface Pointer"),
		    _("The pointer to the nsa_iface"),
		    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void nsa_painter_init(NsaPainter *painter)
{
    debug_log("init:\n");
    painter->iface = NULL;
}

static GObject *nsa_painter_constructor(
	GType type,
	guint n_construct_properties,
	GObjectConstructParam *construct_properties)
{
    GObject *object;
    NsaPainter *painter;
    
    debug_log("painter_constructor:\n");
    object = (*G_OBJECT_CLASS(parent_class)->constructor)(
	    type, n_construct_properties, construct_properties);
    
    painter = NSA_PAINTER(object);
    
    debug_log("painter_constructor: done.\n");
    return object;
}

static void nsa_painter_set_property(
	GObject *object,
	guint prop_id, const GValue *value, GParamSpec *pspec)
{
    NsaPainter *painter = NSA_PAINTER(object);
    
    debug_log("set_prop:\n");
    
    switch (prop_id) {
    case PROP_IFACE:
	debug_log("set_prop: is iface.\n");
	if (painter->iface != NULL)
	    g_object_unref(painter->iface);
	painter->iface = g_value_get_pointer(value);
	g_object_ref(painter->iface);
	debug_log("set_prop: iface=%p\n", painter->iface);
	break;
    default:
	debug_log("set_prop: unknown id.\n");
	break;
    }
}

static void nsa_painter_get_property(
	GObject *object,
	guint prop_id, GValue *value, GParamSpec *pspec)
{
    NsaPainter *painter = NSA_PAINTER(object);
    
    switch (prop_id) {
    case PROP_IFACE:
	g_value_set_pointer(value, painter->iface);
	break;
    default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	break;
    }
}

static void nsa_painter_destroy(GtkObject *object)
{
    NsaPainter *painter = NSA_PAINTER(object);
    NsaIface *iface = painter->iface;
    
    debug_log("nsa_painter_destroy:\n");
    
    painter->iface = NULL;
    g_object_unref(iface);
    
    (*GTK_OBJECT_CLASS(parent_class)->destroy)(object);
    
    debug_log("nsa_painter_destroy: done.\n");
}

static void nsa_painter_finalize(GObject *object)
{
    debug_log("nsa_painter_finalize:\n");
    
    (*G_OBJECT_CLASS(parent_class)->finalize)(object);
    
    debug_log("nsa_painter_finalize: done.\n");
}

void nsa_painter_paint(NsaPainter *painter, GdkRectangle *area)
{
    (*NSA_PAINTER_GET_CLASS(painter)->paint)(painter, area);
}

void nsa_painter_repaint(NsaPainter *painter)
{
    (*NSA_PAINTER_GET_CLASS(painter)->repaint)(painter);
}

void nsa_painter_shift_paint(NsaPainter *painter)
{
    (*NSA_PAINTER_GET_CLASS(painter)->shift_paint)(painter);
}

/*EOF*/
