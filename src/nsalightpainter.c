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

#include "../config.h"

#include <string.h>
#include <math.h>
#include "debug.h"
#include "nsalightpainter.h"

#include "i18n-support.h"

GType nsa_light_painter_get_type(void);
static void nsa_light_painter_class_init(NsaLightPainterClass *klass);
static void nsa_light_painter_init(NsaLightPainter *painter);
static GObject *nsa_light_painter_constructor(
	GType type,
	guint n_construct_properties,
	GObjectConstructParam *construct_properties);
static void nsa_light_painter_destroy(GtkObject *object);
static void nsa_light_painter_finalize(GObject *object);
static void nsa_light_painter_paint(NsaPainter *painter, GdkRectangle *area);
static void nsa_light_painter_paint_graph(NsaLightPainter *light_painter, GdkRectangle *area);
static void nsa_light_painter_paint_name(NsaLightPainter *light_painter, GdkRectangle *area);
static void nsa_light_painter_repaint(NsaPainter *painter);
static void nsa_light_painter_shift_paint(NsaPainter *painter);
static void draw_pulse(NsaLightPainter *light_painter, int x, struct val_t *vp);
static void draw_line(NsaLightPainter *light_painter, struct color_t *col, GdkGC *gc, int x, int y0, int y1);
static int bps_to_meter(NsaLightPainter *light_painter, bps_t bps);

static NsaPainterClass *parent_class = NULL;

GType nsa_light_painter_get_type(void)
{
    static GType light_painter_type = 0;
    
    if (!light_painter_type) {
	static const GTypeInfo light_painter_info = {
	    sizeof (NsaLightPainterClass),
	    NULL,			/* base_init */
	    NULL,			/* base_finalize */
	    (GClassInitFunc) nsa_light_painter_class_init,
	    NULL,			/* class_finalize */
	    NULL,			/* class_data */
	    sizeof (NsaLightPainter),
	    0,				/* n_preallocs */
	    (GInstanceInitFunc) nsa_light_painter_init,
	};
	
	light_painter_type = g_type_register_static(
		NSA_TYPE_PAINTER, "NsaLightPainter", &light_painter_info, 0);
    }
    
    return light_painter_type;
}

static void nsa_light_painter_class_init(NsaLightPainterClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkObjectClass *object_class = GTK_OBJECT_CLASS(klass);
    NsaPainterClass *painter_class = NSA_PAINTER_CLASS(klass);
    
    parent_class = g_type_class_peek_parent(klass);
    
    gobject_class->constructor = nsa_light_painter_constructor;
    gobject_class->finalize = nsa_light_painter_finalize;

    object_class->destroy = nsa_light_painter_destroy;
    
    painter_class->paint = nsa_light_painter_paint;
    painter_class->repaint = nsa_light_painter_repaint;
    painter_class->shift_paint = nsa_light_painter_shift_paint;
    
    gdk_rgb_init();
}

static void nsa_light_painter_init(NsaLightPainter *light_painter)
{
    int i;
    
    light_painter->pixmap = NULL;
    light_painter->mask = NULL;
    
    light_painter->gc_0 = light_painter->gc_1 = NULL;
    for (i = 0; i < COL_NR; i++)
	light_painter->gc[i] = NULL;
    light_painter->gc_fg_mid = NULL;
    light_painter->gc_copy_pix = light_painter->gc_copy_mask = NULL;
    light_painter->gc_copy = NULL;
    light_painter->label_layout = NULL;
    light_painter->text_colors[0].pixel = 0;
    light_painter->text_colors[1].pixel = 0;
    light_painter->text_colors[2].pixel = 0;
    light_painter->curr_text_color = NULL;
}

static GObject *nsa_light_painter_constructor(
	GType type,
	guint n_construct_properties,
	GObjectConstructParam *construct_properties)
{
    GObject *object;
    NsaLightPainter *light_painter;
    GtkWidget *widget;
    
    debug_log("light_painter_constructor:\n");
    
    object = (*G_OBJECT_CLASS(parent_class)->constructor)(
	    type, n_construct_properties, construct_properties);
    
    light_painter = NSA_LIGHT_PAINTER(object);
    widget = GTK_WIDGET(NSA_PAINTER(light_painter)->iface);
    
    nsa_light_painter_repaint(NSA_PAINTER(light_painter));
    
    debug_log("light_painter_constructor: done.\n");
    
    return object;
}

static void nsa_light_painter_destroy(GtkObject *object)
{
    NsaLightPainter *light_painter = NSA_LIGHT_PAINTER(object);
    int i;
    
    debug_log("nsa_light_painter_destroy:\n");
    
#define UNREF(v)	\
	((v) != NULL ? (g_object_unref(v), (v) = NULL) : (void) 0)
    UNREF(light_painter->pixmap);
    UNREF(light_painter->mask);
    UNREF(light_painter->gc_0);
    UNREF(light_painter->gc_1);
    for (i = 0; i < COL_NR; i++)
	UNREF(light_painter->gc[i]);
    UNREF(light_painter->gc_fg_mid);
    UNREF(light_painter->gc_copy_pix);
    UNREF(light_painter->gc_copy_mask);
    UNREF(light_painter->gc_copy);
    UNREF(light_painter->label_layout);
#undef UNREF
    
    (*GTK_OBJECT_CLASS(parent_class)->destroy)(object);
    
    debug_log("nsa_light_painter_destroy: done.\n");
}

static void nsa_light_painter_finalize(GObject *object)
{
    debug_log("nsa_fine_painter_finalize:\n");
    
    (*G_OBJECT_CLASS(parent_class)->finalize)(object);
    
    debug_log("nsa_fine_painter_finalize: done.\n");
}

NsaPainter *nsa_light_painter_new(NsaIface *iface)
{
    NsaLightPainter *light_painter = g_object_new(
	    NSA_TYPE_LIGHT_PAINTER,
	    "iface", iface,
	    NULL);
    
    return NSA_PAINTER(light_painter);
}

static void nsa_light_painter_paint(NsaPainter *painter, GdkRectangle *area)
{
    NsaLightPainter *light_painter = NSA_LIGHT_PAINTER(painter);
    NsaIface *iface = painter->iface;
    
    if (GTK_WIDGET_REALIZED(GTK_WIDGET(iface))) {
	nsa_light_painter_paint_graph(light_painter, area);
	nsa_light_painter_paint_name(light_painter, area);
    }
}

static void nsa_light_painter_paint_graph(NsaLightPainter *light_painter, GdkRectangle *area)
{
    NsaPainter *painter = NSA_PAINTER(light_painter);
    NsaIface *iface = painter->iface;
    GtkWidget *widget = GTK_WIDGET(iface);
//    debug_log("paint_graph: area: %d,%d,%d,%d.\n", area->x, area->y, area->width, area->height);
    
    gdk_gc_set_clip_mask(light_painter->gc_copy, light_painter->mask);
    gdk_gc_set_clip_origin(light_painter->gc_copy,
	    widget->style->xthickness, widget->style->ythickness);
    
    gdk_draw_drawable(widget->window, light_painter->gc_copy, light_painter->pixmap,
	    0, 0,
	    widget->style->xthickness, widget->style->ythickness,
	    nsa_iface_get_width(iface), nsa_iface_get_height(iface));
    
    gdk_gc_set_clip_mask(light_painter->gc_copy, NULL);
    gdk_gc_set_clip_origin(light_painter->gc_copy, 0, 0);
}

static void nsa_light_painter_paint_name(NsaLightPainter *light_painter, GdkRectangle *area)
{
    NsaPainter *painter = NSA_PAINTER(light_painter);
    NsaIface *iface = painter->iface;
    GtkWidget *widget = GTK_WIDGET(iface);
    GdkColor *p;
    GList *list = iface->history;
    
    p = &light_painter->text_colors[0];
    if (list != NULL) {
	switch (((struct val_t *) list->data)->stat) {
	case IFSTAT_DOWN:
	    p = &light_painter->text_colors[0];
	    break;
	case IFSTAT_UP:
	    p = &light_painter->text_colors[1];
	    break;
	case IFSTAT_NOADDR:
	    p = &light_painter->text_colors[2];
	    break;
	}
    }
    
    if (p != light_painter->curr_text_color) {
	gtk_widget_modify_fg(GTK_WIDGET(iface), GTK_STATE_NORMAL, p);
	gtk_widget_modify_text(GTK_WIDGET(iface), GTK_STATE_NORMAL, p);
	light_painter->curr_text_color = p;
    }
    gtk_paint_layout(widget->style, widget->window, GTK_STATE_NORMAL, TRUE,
	    area, GTK_WIDGET(iface), "label",
	    widget->style->xthickness + 1,
	    widget->style->ythickness + 1,
	    light_painter->label_layout);
}

static void nsa_light_painter_repaint(NsaPainter *painter)
{
    NsaLightPainter *light_painter = NSA_LIGHT_PAINTER(painter);
    NsaIface *iface = painter->iface;
    GList *lp;
    int x;
    GdkColor colors[COL_NR], color_fg_mid;
    gboolean suc[COL_NR], suc_fg_mid;
    PangoFontDescription *pfd;
    guint width = nsa_iface_get_width(iface);
    guint height = nsa_iface_get_height(iface);
    int i;
    
    debug_log("repaint: rect: %dx%d.\n", width, height);
    
    if (light_painter->pixmap != NULL)
	g_object_unref(light_painter->pixmap);
    light_painter->pixmap = gdk_pixmap_new(
	    GDK_ROOT_PARENT(),
	    width, height, -1);
debug_log("1\n");
    
    if (light_painter->mask != NULL)
	g_object_unref(light_painter->mask);
    light_painter->mask = gdk_pixmap_new(
	    GDK_ROOT_PARENT(),
	    width, height, 1);
debug_log("2\n");
    
    if (light_painter->gc_0 != NULL)
	g_object_unref(light_painter->gc_0);
    light_painter->gc_0 = gdk_gc_new(light_painter->mask);
    gdk_gc_set_function(light_painter->gc_0, GDK_CLEAR);
debug_log("3\n");
    
    if (light_painter->gc_1 != NULL)
	g_object_unref(light_painter->gc_1);
    light_painter->gc_1 = gdk_gc_new(light_painter->mask);
    gdk_gc_set_function(light_painter->gc_1, GDK_SET);
debug_log("4\n");
    
    gdk_draw_rectangle(light_painter->mask, light_painter->gc_0, TRUE,
	    0, 0, width, height);
debug_log("5\n");
    
debug_log("5.0\n");
    for (i = 0; i < COL_NR; i++) {
	colors[i].pixel = 0;
	colors[i].red = iface->color[i].r;
	colors[i].green = iface->color[i].g;
	colors[i].blue = iface->color[i].b;
    }
debug_log("5.1\n");
    mix_color(&light_painter->color_fg_mid,
	    &iface->color[COL_FG], &iface->color[COL_FG2], 1, 1);
debug_log("5.2\n");
    color_fg_mid.pixel = 0;
debug_log("5.3\n");
    color_fg_mid.red = light_painter->color_fg_mid.r;
    color_fg_mid.green = light_painter->color_fg_mid.g;
    color_fg_mid.blue = light_painter->color_fg_mid.b;
debug_log("5.4\n");
debug_log("6\n");
    
    gdk_colormap_alloc_colors(
	    gdk_colormap_get_system(),
	    colors, COL_NR, FALSE, TRUE, suc);
    gdk_colormap_alloc_colors(
	    gdk_colormap_get_system(),
	    &color_fg_mid, 1, FALSE, TRUE, &suc_fg_mid);
debug_log("7\n");
    
    pfd = pango_font_description_from_string(iface->fontname);
    gtk_widget_modify_font(GTK_WIDGET(iface), pfd);
    pango_font_description_free(pfd);
debug_log("8\n");
    
    light_painter->text_colors[0] = colors[COL_TEXT_DOWN];
    light_painter->text_colors[1] = colors[COL_TEXT];
    light_painter->text_colors[2] = colors[COL_TEXT_NOADDR];
    gtk_widget_modify_fg(GTK_WIDGET(iface), GTK_STATE_NORMAL, &colors[COL_TEXT]);
    gtk_widget_modify_text(GTK_WIDGET(iface), GTK_STATE_NORMAL, &colors[COL_TEXT]);
    light_painter->curr_text_color = NULL;
debug_log("9\n");
    
    if (light_painter->label_layout != NULL)
	g_object_unref(light_painter->label_layout);
    light_painter->label_layout = gtk_widget_create_pango_layout(GTK_WIDGET(iface), iface->name);
debug_log("10\n");
    
    for (i = 0; i < COL_NR; i++) {
	if (light_painter->gc[i] != NULL)
	    g_object_unref(light_painter->gc[i]);
	light_painter->gc[i] = gdk_gc_new(light_painter->pixmap);
	gdk_gc_set_foreground(light_painter->gc[i], &colors[i]);
    }
debug_log("11\n");
    
    if (light_painter->gc_fg_mid != NULL)
	g_object_unref(light_painter->gc_fg_mid);
    light_painter->gc_fg_mid = gdk_gc_new(light_painter->pixmap);
    gdk_gc_set_foreground(light_painter->gc_fg_mid, &color_fg_mid);
debug_log("12\n");
    
    gdk_draw_rectangle(light_painter->mask, light_painter->gc_0, TRUE,
	    0, 0, width, height);
debug_log("13\n");
    
    debug_log("pre draw_pulse:\n");
    for (x = width - 1, lp = iface->history;
	    x >= 0 && lp != NULL; x--, lp = g_list_next(lp)) {
	struct val_t *vp = lp->data;
	draw_pulse(light_painter, x, vp);
    }
    debug_log("post draw_pulse:\n");
    
    gtk_widget_queue_draw(GTK_WIDGET(iface));
    
    if (light_painter->gc_copy_pix != NULL)
	g_object_unref(light_painter->gc_copy_pix);
    light_painter->gc_copy_pix = gdk_gc_new(light_painter->pixmap);
    gdk_gc_set_function(light_painter->gc_copy_pix, GDK_COPY);
    
    if (light_painter->gc_copy_mask != NULL)
	g_object_unref(light_painter->gc_copy_mask);
    light_painter->gc_copy_mask = gdk_gc_new(light_painter->mask);
    gdk_gc_set_function(light_painter->gc_copy_mask, GDK_COPY);
    
    if (light_painter->gc_copy != NULL)
	g_object_unref(light_painter->gc_copy);
    light_painter->gc_copy = gdk_gc_new(light_painter->pixmap);
    gdk_gc_set_function(light_painter->gc_copy, GDK_COPY);
}

static void nsa_light_painter_shift_paint(NsaPainter *painter)
{
    NsaLightPainter *light_painter = NSA_LIGHT_PAINTER(painter);
    NsaIface *iface = painter->iface;
    guint width = nsa_iface_get_width(iface);
    guint height = nsa_iface_get_height(iface);
    struct val_t *vp;
    
    if (iface->history == NULL)
	return;
    
    vp = iface->history->data;
    
    gdk_draw_drawable(light_painter->pixmap, light_painter->gc_copy_pix, light_painter->pixmap,
	    1, 0, 0, 0, width - 1, height);
    gdk_draw_drawable(light_painter->mask, light_painter->gc_copy_mask, light_painter->mask,
	    1, 0, 0, 0, width - 1, height);
    
    draw_pulse(light_painter, width - 1, vp);
    
    gtk_widget_queue_draw(GTK_WIDGET(iface));
}

static void draw_pulse(NsaLightPainter *light_painter, int x, struct val_t *vp)
{
    NsaPainter *painter = NSA_PAINTER(light_painter);
    NsaIface *iface = painter->iface;
    guint height = nsa_iface_get_height(iface);
    
    if (vp->stat == IFSTAT_UP || vp->stat == IFSTAT_NOADDR) {
	int h = bps_to_meter(light_painter, vp->value_in + vp->value_out);
	int i;
	
	if (vp->stat == IFSTAT_UP) {
	    draw_line(light_painter, &iface->color[COL_BG], light_painter->gc[COL_BG],
		    x, 0, height - 1);
	} else {
	    draw_line(light_painter, &iface->color[COL_BG_NOADDR], light_painter->gc[COL_BG_NOADDR],
		    x, 0, height - 1);
	}
	if (h > 0) {
	    struct color_t *color;
	    GdkGC *gc;
	    if (h > height)
		h = height;
	    if (vp->value_in >= vp->value_out * 2) {
		color = &iface->color[COL_FG];
		gc = light_painter->gc[COL_FG];
	    } else if (vp->value_out >= vp->value_in * 2) {
		color = &iface->color[COL_FG2];
		gc = light_painter->gc[COL_FG2];
	    } else {
		color = &light_painter->color_fg_mid;
		gc = light_painter->gc_fg_mid;
	    }
	    
	    draw_line(light_painter, color, gc, x, height - h, height - 1);
	}
	
	for (i = 1; i < iface->mag; i++) {
	    h = height * i / iface->mag;
	    draw_line(light_painter, &iface->color[COL_HBAR], light_painter->gc[COL_HBAR],
		    x, height - h, height - h);
	}
    } else {
	draw_line(light_painter, &iface->color[COL_DOWN], light_painter->gc[COL_DOWN],
		x, 0, height - 1);
    }
}

static void draw_line(NsaLightPainter *light_painter, struct color_t *col, GdkGC *gc, int x, int y0, int y1)
{
    if (col->a & 0x8000) {
	gdk_draw_line(light_painter->mask, light_painter->gc_1, x, y0, x, y1);
	gdk_draw_line(light_painter->pixmap, gc, x, y0, x, y1);
    } else {
	gdk_draw_line(light_painter->mask, light_painter->gc_0, x, y0, x, y1);
    }
}

static int bps_to_meter(NsaLightPainter *light_painter, bps_t bps)
{
    NsaPainter *painter = NSA_PAINTER(light_painter);
    NsaIface *iface = painter->iface;
    int h = 0;
    
    if (bps > 0) {
	h = nsa_iface_get_height(iface) * log(bps) / (iface->mag * log(1024));
	if (h <= 0)
	    h = 1;
    }
    
    return h;
}

/*EOF*/
