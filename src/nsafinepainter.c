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
 * $Id: nsafinepainter.c 34 2005-07-18 09:57:54Z masm $
 */

#include "../config.h"

#include <string.h>
#include <math.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "debug.h"
#include "nsafinepainter.h"

#include "i18n-support.h"

GType nsa_fine_painter_get_type(void);
static void nsa_fine_painter_class_init(NsaFinePainterClass *klass);
static void nsa_fine_painter_init(NsaFinePainter *painter);
static GObject *nsa_fine_painter_constructor(
	GType type,
	guint n_construct_properties,
	GObjectConstructParam *construct_properties);
static void nsa_fine_painter_destroy(GtkObject *object);
static void nsa_fine_painter_finalize(GObject *object);
static void nsa_fine_painter_paint(NsaPainter *painter, GdkRectangle *area);
static void nsa_fine_painter_paint_graph(NsaFinePainter *fine_painter, GdkRectangle *area);
static void nsa_fine_painter_paint_name(NsaFinePainter *fine_painter, GdkRectangle *area);
static void nsa_fine_painter_repaint(NsaPainter *painter);
static void nsa_fine_painter_shift_paint(NsaPainter *painter);
static void draw_pulse(NsaFinePainter *fine_painter, int x, struct val_t *vp);
static int bps_to_meter(NsaFinePainter *fine_painter, bps_t bps);
static void pixdata_alloc(struct pixdata_t *pdp, NsaFinePainter *fine_painter);
static void pixdata_realloc(struct pixdata_t *pdp, NsaFinePainter *fine_painter);
static void pixdata_free(struct pixdata_t *pdp);
static void pixdata_shift(struct pixdata_t *pdp);
static void pixdata_draw_line(struct pixdata_t *pdp, struct color_t *col, int x, int y0, int y1);
static void pixdata_draw(struct pixdata_t *pdp, NsaFinePainter *fine_painter);
static void pixdata_draw_layout(struct pixdata_t *pdp1, struct pixdata_t *pdp2, struct pixdata_t *pdp3,
	NsaFinePainter *fine_painter,
	PangoLayout *layout, struct color_t *c1, struct color_t *c2, struct color_t *c3);
static void pixdata_compose_text(struct pixdata_t *pdp,
	NsaFinePainter *fine_painter, struct pixdata_t *pdp1, struct pixdata_t *pdp2, struct pixdata_t *pdp3);

static NsaPainterClass *parent_class = NULL;

GType nsa_fine_painter_get_type(void)
{
    static GType fine_painter_type = 0;
    
    if (!fine_painter_type) {
	static const GTypeInfo fine_painter_info = {
	    sizeof (NsaFinePainterClass),
	    NULL,			/* base_init */
	    NULL,			/* base_finalize */
	    (GClassInitFunc) nsa_fine_painter_class_init,
	    NULL,			/* class_finalize */
	    NULL,			/* class_data */
	    sizeof (NsaFinePainter),
	    0,				/* n_preallocs */
	    (GInstanceInitFunc) nsa_fine_painter_init,
	};
	
	fine_painter_type = g_type_register_static(
		NSA_TYPE_PAINTER, "NsaFinePainter", &fine_painter_info, 0);
    }
    
    return fine_painter_type;
}

static void nsa_fine_painter_class_init(NsaFinePainterClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkObjectClass *object_class = GTK_OBJECT_CLASS(klass);
    NsaPainterClass *painter_class = NSA_PAINTER_CLASS(klass);
    
    parent_class = g_type_class_peek_parent(klass);
    
    gobject_class->constructor = nsa_fine_painter_constructor;
    gobject_class->finalize = nsa_fine_painter_finalize;
    
    object_class->destroy = nsa_fine_painter_destroy;
    
    painter_class->paint = nsa_fine_painter_paint;
    painter_class->repaint = nsa_fine_painter_repaint;
    painter_class->shift_paint = nsa_fine_painter_shift_paint;
    
    gdk_rgb_init();
}

static void nsa_fine_painter_init(NsaFinePainter *fine_painter)
{
    debug_log("fine_painter_init:\n");
    
    memset(&fine_painter->layers, 0, sizeof fine_painter->layers);
    
    memset(&fine_painter->black, 0, sizeof fine_painter->black);
    memset(&fine_painter->white, 0, sizeof fine_painter->white);
    debug_log("fine_painter_init: done.\n");
}

static GObject *nsa_fine_painter_constructor(
	GType type,
	guint n_construct_properties,
	GObjectConstructParam *construct_properties)
{
    GObject *object;
    NsaFinePainter *fine_painter;
    GtkWidget *widget;
    
    debug_log("fine_painter_constructor:\n");
    
    object = (*G_OBJECT_CLASS(parent_class)->constructor)(
	    type, n_construct_properties, construct_properties);
    
    fine_painter = NSA_FINE_PAINTER(object);
    widget = GTK_WIDGET(NSA_PAINTER(fine_painter)->iface);
    
    pixdata_alloc(&fine_painter->layers.bg, fine_painter);
    pixdata_alloc(&fine_painter->layers.fg, fine_painter);
    pixdata_alloc(&fine_painter->layers.hbar, fine_painter);
    pixdata_alloc(&fine_painter->layers.text1, fine_painter);
    pixdata_alloc(&fine_painter->layers.text2, fine_painter);
    pixdata_alloc(&fine_painter->layers.text3, fine_painter);
    pixdata_alloc(&fine_painter->layers.text, fine_painter);
    
    fine_painter->black.red = fine_painter->black.green = fine_painter->black.blue = 0x0000;
    fine_painter->white.red = fine_painter->white.green = fine_painter->white.blue = 0xffff;
    gdk_color_alloc(gtk_widget_get_colormap(widget), &fine_painter->black);
    gdk_color_alloc(gtk_widget_get_colormap(widget), &fine_painter->white);
    
    debug_log("fine_painter_constructor: done.\n");
    
    return object;
}

static void nsa_fine_painter_destroy(GtkObject *object)
{
    debug_log("nsa_fine_painter_destroy:\n");
    
    (*GTK_OBJECT_CLASS(parent_class)->destroy)(object);
    
    debug_log("nsa_fine_painter_destroy: done.\n");
}

static void nsa_fine_painter_finalize(GObject *object)
{
    NsaFinePainter *fine_painter = NSA_FINE_PAINTER(object);
    
    debug_log("nsa_fine_painter_finalize:\n");
    
    pixdata_free(&fine_painter->layers.bg);
    pixdata_free(&fine_painter->layers.fg);
    pixdata_free(&fine_painter->layers.hbar);
    pixdata_free(&fine_painter->layers.text1);
    pixdata_free(&fine_painter->layers.text2);
    pixdata_free(&fine_painter->layers.text3);
    pixdata_free(&fine_painter->layers.text);
    
    (*G_OBJECT_CLASS(parent_class)->finalize)(object);
    
    debug_log("nsa_fine_painter_finalize: done.\n");
}

NsaPainter *nsa_fine_painter_new(NsaIface *iface)
{
    NsaFinePainter *fine_painter;
    
    fine_painter = g_object_new(
	    NSA_TYPE_FINE_PAINTER,
	    "iface", iface,
	    NULL);
    
    return NSA_PAINTER(fine_painter);
}

static void nsa_fine_painter_paint(NsaPainter *painter, GdkRectangle *area)
{
    NsaFinePainter *fine_painter = NSA_FINE_PAINTER(painter);
    NsaIface *iface = painter->iface;
    
    if (GTK_WIDGET_REALIZED(GTK_WIDGET(iface))) {
	nsa_fine_painter_paint_graph(fine_painter, area);
	nsa_fine_painter_paint_name(fine_painter, area);
    }
}

static void nsa_fine_painter_paint_graph(NsaFinePainter *fine_painter, GdkRectangle *area)
{
    pixdata_draw(&fine_painter->layers.bg, fine_painter);
    pixdata_draw(&fine_painter->layers.fg, fine_painter);
    pixdata_draw(&fine_painter->layers.hbar, fine_painter);
}

static void nsa_fine_painter_paint_name(NsaFinePainter *fine_painter, GdkRectangle *area)
{
    pixdata_compose_text(&fine_painter->layers.text,
	    fine_painter, &fine_painter->layers.text1, &fine_painter->layers.text2, &fine_painter->layers.text3);
    pixdata_draw(&fine_painter->layers.text, fine_painter);
}

static void nsa_fine_painter_repaint(NsaPainter *painter)
{
    NsaFinePainter *fine_painter = NSA_FINE_PAINTER(painter);
    NsaIface *iface = painter->iface;
    GList *lp;
    int x;
    PangoFontDescription *pfd;
    PangoLayout *layout;
    
    debug_log("repaint: rect: %dx%d.\n",
	    nsa_iface_get_width(iface), nsa_iface_get_height(iface));
    
    pixdata_realloc(&fine_painter->layers.bg, fine_painter);
    pixdata_realloc(&fine_painter->layers.fg, fine_painter);
    pixdata_realloc(&fine_painter->layers.hbar, fine_painter);
    pixdata_realloc(&fine_painter->layers.text1, fine_painter);
    pixdata_realloc(&fine_painter->layers.text2, fine_painter);
    pixdata_realloc(&fine_painter->layers.text3, fine_painter);
    pixdata_realloc(&fine_painter->layers.text, fine_painter);
    
    pfd = pango_font_description_from_string(iface->fontname);
    gtk_widget_modify_font(GTK_WIDGET(iface), pfd);
    pango_font_description_free(pfd);
    
    layout = gtk_widget_create_pango_layout(GTK_WIDGET(iface), iface->name);
    pixdata_draw_layout(&fine_painter->layers.text1, &fine_painter->layers.text2, &fine_painter->layers.text3, fine_painter,
	    layout, &iface->color[COL_TEXT], &iface->color[COL_TEXT_DOWN], &iface->color[COL_TEXT_NOADDR]);
    g_object_unref(layout);
    
    for (x = nsa_iface_get_width(iface) - 1, lp = iface->history;
	    x >= 0 && lp != NULL; x--, lp = g_list_next(lp)) {
	struct val_t *vp = lp->data;
	draw_pulse(fine_painter, x, vp);
    }
}

static void nsa_fine_painter_shift_paint(NsaPainter *painter)
{
    NsaFinePainter *fine_painter = NSA_FINE_PAINTER(painter);
    NsaIface *iface = painter->iface;
    struct val_t *vp;
    
    if (iface->history == NULL)
	return;
    
    vp = iface->history->data;
    
    pixdata_shift(&fine_painter->layers.bg);
    pixdata_shift(&fine_painter->layers.fg);
    pixdata_shift(&fine_painter->layers.hbar);
    
    draw_pulse(fine_painter, nsa_iface_get_width(iface) - 1, vp);
}

static void draw_pulse(NsaFinePainter *fine_painter, int x, struct val_t *vp)
{
    NsaPainter *painter = NSA_PAINTER(fine_painter);
    NsaIface *iface = painter->iface;
    int height = nsa_iface_get_height(iface);
    
    if (vp->stat == IFSTAT_UP || vp->stat == IFSTAT_NOADDR) {
	int h = bps_to_meter(fine_painter, vp->value_in + vp->value_out);
	int i;
	
	if (vp->stat == IFSTAT_UP)
	    pixdata_draw_line(&fine_painter->layers.bg, &iface->color[COL_BG], x, 0, height - 1);
	else
	    pixdata_draw_line(&fine_painter->layers.bg, &iface->color[COL_BG_NOADDR], x, 0, height - 1);
	
	if (h > 0) {
	    struct color_t color;
	    
	    if (h > height)
		h = height;
	    
	    mix_color(&color, &iface->color[COL_FG], &iface->color[COL_FG2], vp->value_in, vp->value_out);
	    debug_log("color: a%d,r%d,g%d,b%d\n", color.a, color.r, color.g, color.b);
	    
	    pixdata_draw_line(&fine_painter->layers.fg , &color, x, height - h, height - 1);
	}
	
	for (i = 1; i < iface->mag; i++) {
	    h = height * i / iface->mag;
	    pixdata_draw_line(&fine_painter->layers.hbar, &iface->color[COL_HBAR], x, height - h, height - h);
	}
    } else {
	pixdata_draw_line(&fine_painter->layers.bg, &iface->color[COL_DOWN], x, 0, height - 1);
    }
}

static int bps_to_meter(NsaFinePainter *fine_painter, bps_t bps)
{
    NsaPainter *painter = NSA_PAINTER(fine_painter);
    NsaIface *iface = painter->iface;
    int h = 0;
    
    if (bps > 0) {
	h = nsa_iface_get_height(iface) * log(bps) / (iface->mag * log(1024));
	if (h <= 0)
	    h = 1;
    }
    
    return h;
}

/****************************************************************/

static void pixdata_alloc(struct pixdata_t *pdp, NsaFinePainter *fine_painter)
{
    NsaIface *iface = NSA_PAINTER(fine_painter)->iface;
    int width = nsa_iface_get_width(iface);
    int height = nsa_iface_get_height(iface);
    GdkPixdata *pp = &pdp->pixdata;
    
    pp->magic = GDK_PIXBUF_MAGIC_NUMBER;
    pp->length = 0;
    pp->pixdata_type = GDK_PIXDATA_COLOR_TYPE_RGBA | GDK_PIXDATA_SAMPLE_WIDTH_8 | GDK_PIXDATA_ENCODING_RAW;
    pp->rowstride = width * 4;
    pp->width = width;
    pp->height = height;
    pp->pixel_data = g_new(guint8, width * height * 4);
    memset(pp->pixel_data, 0, width * height * 4);
    
    pdp->pixbuf = gdk_pixbuf_from_pixdata(&pdp->pixdata, FALSE, NULL);
}

static void pixdata_realloc(struct pixdata_t *pdp, NsaFinePainter *fine_painter)
{
    pixdata_free(pdp);
    pixdata_alloc(pdp, fine_painter);
}

static void pixdata_free(struct pixdata_t *pdp)
{
    if (pdp->pixbuf != NULL) {
	gdk_pixbuf_unref(pdp->pixbuf);
	pdp->pixbuf = NULL;
    }
    if (pdp->pixdata.pixel_data != NULL) {
	g_free(pdp->pixdata.pixel_data);
	pdp->pixdata.pixel_data = NULL;
    }
}

static void pixdata_shift(struct pixdata_t *pdp)
{
    GdkPixdata *pp = &pdp->pixdata;
    int y;
    
    for (y = 0; y < pp->height; y++) {
	guint8 *sp, *dp;
	int n;
	
	dp = &pp->pixel_data[pp->width * 4 * y];
	sp = dp + 4;
	
	for (n = (pp->width - 1) * 4; n > 0; n--)
	    *dp++ = *sp++;
	
	*dp++ = 0;
	*dp++ = 0;
	*dp++ = 0;
	*dp++ = 0;
    }
}

static void pixdata_draw_line(struct pixdata_t *pdp, struct color_t *col, int x, int y0, int y1)
{
    GdkPixdata *pp = &pdp->pixdata;
    int y;
    guint8 *p;
    int step;
    
    if (x < 0)
	return;
    if (x >= pp->width)
	return;
    if (y0 < 0)
	y0 = 0;
    if (y0 >= pp->height)
	y0 = pp->height - 1;
    if (y1 < 0)
	y1 = 0;
    if (y1 >= pp->height)
	y1 = pp->height - 1;
    
    if (y0 > y1) {
	int t = y0;
	y0 = y1;
	y1 = t;
    }
    
    p = &pp->pixel_data[pp->width * 4 * y0 + 4 * x];
    step = pp->width * 4;
    for (y = y0; y <= y1; y++, p += step) {
	p[0] = col->r >> 8;
	p[1] = col->g >> 8;
	p[2] = col->b >> 8;
	p[3] = col->a >> 8;
    }
}

static void pixdata_draw(struct pixdata_t *pdp, NsaFinePainter *fine_painter)
{
    NsaPainter *painter = NSA_PAINTER(fine_painter);
    NsaIface *iface = painter->iface;
    GtkWidget *widget = GTK_WIDGET(iface);
    debug_log("pixdata_draw: x=%d,y=%d,w=%d,h=%d\n",
	    iface->widget.style->xthickness, iface->widget.style->ythickness,
	    pdp->pixdata.width, pdp->pixdata.height);
    if (!GTK_WIDGET_REALIZED(widget))
	return;
    gdk_draw_pixbuf(widget->window, NULL, pdp->pixbuf,
	    0, 0,
	    widget->style->xthickness, widget->style->ythickness,
	    pdp->pixdata.width, pdp->pixdata.height,
	    GDK_RGB_DITHER_NONE, 0, 0);
}

static void pixdata_draw_layout(struct pixdata_t *pdp1, struct pixdata_t *pdp2, struct pixdata_t *pdp3,
	NsaFinePainter *fine_painter,
	PangoLayout *layout, struct color_t *c1, struct color_t *c2, struct color_t *c3)
{
    GdkPixmap *pixmap;
    GdkGC *gc;
    int x, y;
    guint8 *sp, *dp1, *dp2, *dp3;
    NsaPainter *painter = NSA_PAINTER(fine_painter);
    NsaIface *iface = painter->iface;
    int width = nsa_iface_get_width(iface);
    int height = nsa_iface_get_height(iface);
    GdkPixbuf *pixbuf;
    GdkPixdata pixdata;
    
    if (!GTK_WIDGET_REALIZED(GTK_WIDGET(iface)))
	return;
    
    debug_log("pixdata_draw_layout:\n");
    pixmap = gdk_pixmap_new(GTK_WIDGET(iface)->window, width, height, -1);
    gc = gdk_gc_new(pixmap);
    
    debug_log("pixdata_draw_layout: clear.\n");
    gdk_gc_set_foreground(gc, &fine_painter->black);
    gdk_gc_set_function(gc, GDK_COPY);
    gdk_draw_rectangle(pixmap, gc, TRUE, 0, 0, width, height);
    
    debug_log("pixdata_draw_layout: draw.\n");
    gdk_gc_set_foreground(gc, &fine_painter->white);
    gdk_gc_set_function(gc, GDK_COPY);
    gdk_draw_layout(pixmap, gc, 1, 1, layout);
    
    debug_log("pixdata_draw_layout: get image.\n");
    pixbuf = gdk_pixbuf_get_from_drawable(NULL, pixmap, gtk_widget_get_colormap(GTK_WIDGET(iface)), 0, 0, 0, 0, nsa_iface_get_width(iface), nsa_iface_get_height(iface));
    
    debug_log("pixdata_draw_layout: get image.\n");
    gdk_pixdata_from_pixbuf(&pixdata, pixbuf, FALSE);
    
    dp1 = pdp1->pixdata.pixel_data;
    dp2 = pdp2->pixdata.pixel_data;
    dp3 = pdp3->pixdata.pixel_data;
    for (y = 0; y < height; y++) {
	sp = pixdata.pixel_data + pixdata.rowstride * y;
	for (x = 0; x < width; x++) {
	    guint8 r = *sp++;
	    guint8 g = *sp++;
	    guint8 b = *sp++;
	    int a = (r * 3 + g * 5 + b * 2) / 10;
	    
	    *dp1++ = c1->r >> 8;
	    *dp1++ = c1->g >> 8;
	    *dp1++ = c1->b >> 8;
	    *dp1++ = a * (c1->a >> 8) / 255;
	    
	    *dp2++ = c2->r >> 8;
	    *dp2++ = c2->g >> 8;
	    *dp2++ = c2->b >> 8;
	    *dp2++ = a * (c2->a >> 8) / 255;
	    
	    *dp3++ = c3->r >> 8;
	    *dp3++ = c3->g >> 8;
	    *dp3++ = c3->b >> 8;
	    *dp3++ = a * (c3->a >> 8) / 255;
	}
    }
    
    debug_log("pixdata_draw_layout: unref.\n");
    g_object_unref(pixbuf);
    
    g_object_unref(gc);
    g_object_unref(pixmap);
    
    debug_log("pixdata_draw_layout: done.\n");
}

static void pixdata_compose_text(struct pixdata_t *pdp,
	NsaFinePainter *fine_painter, struct pixdata_t *pdp1, struct pixdata_t *pdp2, struct pixdata_t *pdp3)
{
    guint8 *dp, *sp1, *sp2, *sp3;
    NsaPainter *painter = NSA_PAINTER(fine_painter);
    NsaIface *iface = painter->iface;
    guint width = nsa_iface_get_width(iface);
    guint height = nsa_iface_get_height(iface);
    enum ifstat_stat_t *stats = g_new0(enum ifstat_stat_t, width);
    GList *lp;
    int i, x, y;
    
    for (i = width - 1, lp = iface->history;
	    lp != NULL && i >= 0; i--, lp = g_list_next(lp)) {
	struct val_t *vp = lp->data;
	stats[i] = vp->stat;
    }
    
    dp = pdp->pixdata.pixel_data;
    sp1 = pdp1->pixdata.pixel_data;
    sp2 = pdp2->pixdata.pixel_data;
    sp3 = pdp3->pixdata.pixel_data;
    for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++) {
	    switch (stats[x]) {
	    case IFSTAT_UP:
		*dp++ = sp1[0];
		*dp++ = sp1[1];
		*dp++ = sp1[2];
		*dp++ = sp1[3];
		break;
	    case IFSTAT_DOWN:
		*dp++ = sp2[0];
		*dp++ = sp2[1];
		*dp++ = sp2[2];
		*dp++ = sp2[3];
		break;
	    case IFSTAT_NOADDR:
		*dp++ = sp3[0];
		*dp++ = sp3[1];
		*dp++ = sp3[2];
		*dp++ = sp3[3];
		break;
	    }
	    sp1 += 4;
	    sp2 += 4;
	    sp3 += 4;
	}
    }
    
    g_free(stats);
}

/*EOF*/
