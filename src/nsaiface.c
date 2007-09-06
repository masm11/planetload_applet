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
#ifdef HAVE_GNOME
#include <glibtop/netload.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <gtk/gtkmarshal.h>
#include "nsafinepainter.h"
#include "nsalightpainter.h"
#include "nsasplitpainter.h"
#include "debug.h"
#include "nsaiface.h"

#undef DEBUG

#include "i18n-support.h"

enum {
    NAME_CHANGED,
    LAST_SIGNAL
};

static void nsa_iface_class_init	(NsaIfaceClass *klass);
static void nsa_iface_init		(NsaIface *iface);
static void nsa_iface_destroy		(GtkObject *object);
static void nsa_iface_finalize		(GObject *object);
static void nsa_iface_realize		(GtkWidget *widget);
static void nsa_iface_unrealize		(GtkWidget *widget);
static void nsa_iface_size_request	(GtkWidget *widget, GtkRequisition *requisition);
static void nsa_iface_size_allocate	(GtkWidget *widget, GtkAllocation *allocation);
static gboolean nsa_iface_expose	(GtkWidget *widget, GdkEventExpose *event);
static void nsa_iface_style_set         (GtkWidget *widget, GtkStyle *previous_style);

static void restart(NsaIface *iface);
static void repaint(NsaIface *iface);
static void update(NsaIface *iface);
static void shift_paint(NsaIface *iface);
static int calc_mag(NsaIface *iface, int type);
static void get_bps(NsaIface *iface, struct val_t *vp);
static gchar *make_tips_string(NsaIface *iface);
static gboolean nsa_iface_button_press(GtkWidget *w, GdkEventButton *ev);

static guint signals[LAST_SIGNAL] = { 0, };
static GtkWidgetClass *parent_class = NULL;

#define MAX_SIZE 100

int nsa_iface_get_width(NsaIface *iface)
{
    if (iface != NULL) {
	int width = iface->widget.allocation.width - iface->widget.style->xthickness * 2;
	if (width >= 1)
	    return width;
    }
    return 1;
}

int nsa_iface_get_height(NsaIface *iface)
{
    if (iface != NULL) {
	int height = iface->widget.allocation.height - iface->widget.style->ythickness * 2;
	if (height >= 1)
	    return height;
    }
    return 1;
}

GType nsa_iface_get_type(void)
{
    static GType iface_type = 0;
    
    if (!iface_type) {
	static const GTypeInfo iface_info = {
	    sizeof (NsaIfaceClass),
	    NULL,			/* base_init */
	    NULL,			/* base_finalize */
	    (GClassInitFunc) nsa_iface_class_init,
	    NULL,			/* class_finalize */
	    NULL,			/* class_data */
	    sizeof (NsaIface),
	    0,				/* n_preallocs */
	    (GInstanceInitFunc) nsa_iface_init,
	};
	
	iface_type = g_type_register_static(
		GTK_TYPE_WIDGET, "NsaIface", &iface_info, 0);
    }
    
    return iface_type;
}

static void nsa_iface_class_init(NsaIfaceClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkObjectClass *object_class = GTK_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
//  GtkContainerClass *container_class = GTK_CONTAINER_CLASS(klass);
    
    parent_class = g_type_class_peek_parent(klass);
    
    widget_class->realize = nsa_iface_realize;
    widget_class->unrealize = nsa_iface_unrealize;
    widget_class->size_request = nsa_iface_size_request;
    widget_class->size_allocate = nsa_iface_size_allocate;
    widget_class->expose_event = nsa_iface_expose;
    widget_class->style_set = nsa_iface_style_set;
    widget_class->button_press_event = nsa_iface_button_press;
    
    object_class->destroy = nsa_iface_destroy;
    gobject_class->finalize = nsa_iface_finalize;
    
    signals[NAME_CHANGED] = g_signal_new(
	    "name_changed",
	    G_OBJECT_CLASS_TYPE(widget_class),
	    G_SIGNAL_RUN_FIRST,
	    0,
	    NULL, NULL,
	    gtk_marshal_VOID__VOID,
	    G_TYPE_NONE, 0);
}

static void nsa_iface_init(NsaIface *iface)
{
    int i;
    
    iface->painter = NULL;
    iface->painter_id = NSA_PAINTER_TYPE_NONE;
    
    iface->name = g_strdup("lo");
    
    iface->fontname = g_strdup("sans 8");
    
    iface->last_ifstat = NULL;
    iface->last_time.tv_sec = 0;
    iface->last_time.tv_usec = 0;
    
    iface->vert = FALSE;
    iface->size = 40;
    
    iface->tooltips = NULL;
    
    iface->history = NULL;
    iface->nhistory = 0;
    
    iface->mag = 0;
    
    for (i = 0; i < COL_NR; i++) {
	iface->color[i].a = 0;
	iface->color[i].r = 0;
	iface->color[i].g = 0;
	iface->color[i].b = 0;
    }
    
    iface->cmd_up = g_strdup("");
    iface->cmd_down = g_strdup("");
    
    iface->lock_file = g_strdup("");
    
    restart(iface);
}

static void nsa_iface_destroy(GtkObject *object)
{
    NsaIface *iface = NSA_IFACE(object);
    
    debug_log("nsa_iface_destroy:\n");
    
    if (iface->tooltips != NULL) {
	g_object_unref(iface->tooltips);
	iface->tooltips = NULL;
    }
    
    if (iface->painter != NULL) {
	NsaPainter *painter = iface->painter;
	
	iface->painter = NULL;
	iface->painter_id = NSA_PAINTER_TYPE_NONE;
	
//	gtk_object_destroy(GTK_OBJECT(painter));
	g_object_unref(painter);
    }
    
    (*GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}

static void nsa_iface_finalize(GObject *object)
{
    NsaIface *iface = NSA_IFACE(object);
    
    debug_log("nsa_iface_finalize:\n");
    
    if (iface->name != NULL) {
	g_free(iface->name);
	iface->name = NULL;
    }
    
    while (iface->history != NULL) {
	g_free(iface->history->data);
	iface->history = g_list_delete_link(iface->history, iface->history);
    }
    iface->nhistory = 0;
    
    if (iface->cmd_up != NULL) {
	g_free(iface->cmd_up);
	iface->cmd_up = NULL;
    }
    
    if (iface->cmd_down != NULL) {
	g_free(iface->cmd_down);
	iface->cmd_down = NULL;
    }
    
    if (iface->lock_file != NULL) {
	g_free(iface->lock_file);
	iface->lock_file = NULL;
    }
    
    if (iface->fontname != NULL) {
	g_free(iface->fontname);
	iface->fontname = NULL;
    }
    
    // fixme: free last_ifstat
    
    (*G_OBJECT_CLASS(parent_class)->finalize)(object);
}

static void nsa_iface_realize(GtkWidget *widget)
{
    NsaIface *iface = NSA_IFACE(widget);
    GdkWindowAttr attr;
    gint attr_mask;
    
    GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);
    
    attr.window_type = GDK_WINDOW_CHILD;
    attr.x = widget->allocation.x;
    attr.y = widget->allocation.y;
    attr.width = widget->allocation.width;
    attr.height = widget->allocation.height;
    attr.wclass = GDK_INPUT_OUTPUT;
    attr.visual = gtk_widget_get_visual(widget);
    attr.colormap = gtk_widget_get_colormap(widget);
    attr.event_mask = gtk_widget_get_events(widget);
    attr.event_mask |= GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK;
    
    attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
    
    widget->window = gdk_window_new(
	    gtk_widget_get_parent_window(widget), &attr, attr_mask);
    gdk_window_set_user_data(widget->window, widget);
    gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
    
    switch (iface->painter_id) {
    case NSA_PAINTER_TYPE_FINE:
	iface->painter = nsa_fine_painter_new(iface);
	break;
    case NSA_PAINTER_TYPE_LIGHT:
	iface->painter = nsa_light_painter_new(iface);
	break;
    case NSA_PAINTER_TYPE_SPLIT:
	iface->painter = nsa_split_painter_new(iface);
	break;
    default:
	iface->painter = NULL;
	break;
    }
    g_object_ref(iface->painter);
    gtk_object_sink(GTK_OBJECT(iface->painter));
}

static void nsa_iface_unrealize(GtkWidget *widget)
{
    NsaIface *iface = NSA_IFACE(widget);
    NsaPainter *painter = iface->painter;
    
    iface->painter = NULL;
    iface->painter_id = NSA_PAINTER_TYPE_NONE;
    
    if (painter != NULL) {
//	gtk_object_destroy(GTK_OBJECT(painter));
	g_object_unref(painter);
    }
}

static void nsa_iface_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
    NsaIface *iface = NSA_IFACE(widget);
    
    if (!iface->vert) {
	requisition->width = iface->size;
	requisition->height = MAX_SIZE;
    } else {
	requisition->width = MAX_SIZE;
	requisition->height = iface->size;
    }
}

static void nsa_iface_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    if (widget->allocation.width != allocation->width ||
	    widget->allocation.height != allocation->height ||
	    widget->allocation.x != allocation->x ||
	    widget->allocation.y != allocation->y) {
	widget->allocation = *allocation;
	repaint(NSA_IFACE(widget));
	if (GTK_WIDGET_REALIZED(widget)) {
	    gdk_window_move_resize(widget->window,
		    allocation->x, allocation->y,
		    allocation->width, allocation->height);
	}
    }
}

static gboolean nsa_iface_expose(GtkWidget *widget, GdkEventExpose *event)
{
    if (GTK_WIDGET_REALIZED(widget)) {
	NsaIface *iface = NSA_IFACE(widget);
	
	gtk_paint_shadow(widget->style, widget->window,
		GTK_STATE_NORMAL, GTK_SHADOW_IN,
		&event->area, widget, "frame",
		0, 0, widget->allocation.width, widget->allocation.height);
	
	if (iface->painter != NULL)
	    nsa_painter_paint(iface->painter, &event->area);
    }
    
    return TRUE;
}

static void nsa_iface_style_set(GtkWidget *widget, GtkStyle *previous_style)
{
    NsaIface *iface = NSA_IFACE(widget);
    
    if (GTK_WIDGET_CLASS(parent_class)->style_set != NULL)
	(*GTK_WIDGET_CLASS(parent_class)->style_set)(widget, previous_style);
    
    if (previous_style != NULL) {
	if (widget->style->xthickness != previous_style->xthickness ||
		widget->style->ythickness != previous_style->ythickness) {
	    repaint(iface);
	}
    }
}

static void nsa_iface_notify_name_changed(NsaIface *iface)
{
    g_signal_emit(iface, signals[NAME_CHANGED], 0);
}

void nsa_iface_set_vert(NsaIface *iface, gboolean vert)
{
    iface->vert = vert;
    debug_log("nsa_iface_set_vert.\n");
    gtk_widget_queue_resize(GTK_WIDGET(iface));
}

void nsa_iface_set_painter(NsaIface *iface, gint painter_id)
{
    if (painter_id == iface->painter_id)
	return;
    
    if (iface->painter != NULL) {
//	gtk_object_destroy(GTK_OBJECT(iface->painter));
	g_object_unref(iface->painter);
    }
    
    switch (painter_id) {
    case NSA_PAINTER_TYPE_FINE:
	iface->painter = nsa_fine_painter_new(iface);
	iface->painter_id = painter_id;
	break;
    case NSA_PAINTER_TYPE_LIGHT:
	iface->painter = nsa_light_painter_new(iface);
	iface->painter_id = painter_id;
	break;
    case NSA_PAINTER_TYPE_SPLIT:
	iface->painter = nsa_split_painter_new(iface);
	iface->painter_id = painter_id;
	break;
    default:
	iface->painter = NULL;
	iface->painter_id = NSA_PAINTER_TYPE_NONE;
	break;
    }
    
    if (iface->painter != NULL) {
	g_object_ref(iface->painter);
	gtk_object_sink(GTK_OBJECT(iface->painter));
    }
    
    repaint(iface);
}

void nsa_iface_set_tooltips(NsaIface *iface, GtkTooltips *tooltips)
{
    if (iface->tooltips != NULL)
	g_object_unref(iface->tooltips);
    
    iface->tooltips = tooltips;
    
    g_object_ref(iface->tooltips);
}

void nsa_iface_set_font(NsaIface *iface, const gchar *fontname)
{
    g_return_if_fail(NSA_IS_IFACE(iface));
    g_return_if_fail(fontname != NULL);
    
    g_free(iface->fontname);
    iface->fontname = g_strdup(fontname);
    
    repaint(iface);
}

void nsa_iface_set_size(NsaIface *iface, gint size)
{
    g_return_if_fail(NSA_IS_IFACE(iface));
    g_return_if_fail(size > 0);
    
    iface->size = size;
    
    gtk_widget_queue_resize(GTK_WIDGET(iface));
}

void nsa_iface_set_name(NsaIface *iface, const gchar *name)
{
    g_return_if_fail(NSA_IS_IFACE(iface));
    g_return_if_fail(name != NULL);
    
    if (name[0] == '\0')
	name = "lo";
    
    iface->name = g_strdup(name);
    nsa_iface_notify_name_changed(iface);
    
    restart(iface);
}

void nsa_iface_set_is_ppp(NsaIface *iface, gboolean is_ppp)
{
    g_return_if_fail(NSA_IS_IFACE(iface));
    
    iface->is_ppp = is_ppp;
    
    repaint(iface);
}

void nsa_iface_set_color(NsaIface *iface, int part, const struct color_t *color)
{
    g_return_if_fail(NSA_IS_IFACE(iface));
    g_return_if_fail((u_int) part < COL_NR);
    g_return_if_fail(color != NULL);
    
    iface->color[part] = *color;
    
    repaint(iface);
}

void nsa_iface_set_lock_file(NsaIface *iface, const gchar *lock_file)
{
    g_return_if_fail(NSA_IS_IFACE(iface));
    g_return_if_fail(lock_file != NULL);
    
    g_free(iface->lock_file);
    iface->lock_file = g_strdup(lock_file);
}

void nsa_iface_set_cmd_up(NsaIface *iface, const gchar *cmd_up)
{
    g_return_if_fail(NSA_IS_IFACE(iface));
    g_return_if_fail(cmd_up != NULL);
    
    g_free(iface->cmd_up);
    iface->cmd_up = g_strdup(cmd_up);
}

void nsa_iface_set_cmd_down(NsaIface *iface, const gchar *cmd_down)
{
    g_return_if_fail(NSA_IS_IFACE(iface));
    g_return_if_fail(cmd_down != NULL);
    
    g_free(iface->cmd_down);
    iface->cmd_down = g_strdup(cmd_down);
}

gint nsa_iface_get_size(NsaIface *iface)
{
    g_return_val_if_fail(NSA_IS_IFACE(iface), 1);
    return iface->size;
}

const gchar *nsa_iface_get_name(NsaIface *iface)
{
    g_return_val_if_fail(NSA_IS_IFACE(iface), NULL);
    return iface->name;
}

gboolean nsa_iface_get_is_ppp(NsaIface *iface)
{
    g_return_val_if_fail(NSA_IS_IFACE(iface), FALSE);
    return iface->is_ppp;
}

const struct color_t *nsa_iface_get_color(NsaIface *iface, int part)
{
    g_return_val_if_fail(NSA_IS_IFACE(iface), NULL);
    g_return_val_if_fail((u_int) part < COL_NR, NULL);
    
    return &iface->color[part];
}

const gchar *nsa_iface_get_lock_file(NsaIface *iface)
{
    g_return_val_if_fail(NSA_IS_IFACE(iface), NULL);
    return iface->lock_file;
}

const gchar *nsa_iface_get_cmd_up(NsaIface *iface)
{
    g_return_val_if_fail(NSA_IS_IFACE(iface), NULL);
    return iface->cmd_up;
}

const gchar *nsa_iface_get_cmd_down(NsaIface *iface)
{
    g_return_val_if_fail(NSA_IS_IFACE(iface), NULL);
    return iface->cmd_down;
}

void nsa_iface_update(NsaIface *iface)
{
    g_return_if_fail(NSA_IS_IFACE(iface));
    
    update(iface);
}

void nsa_iface_panel_size_changed(NsaIface *iface)
{
    gtk_widget_queue_resize(GTK_WIDGET(iface));
}

static void restart(NsaIface *iface)
{
    iface->last_ifstat = NULL;
    
    while (iface->history != NULL) {
	g_free(iface->history->data);
	iface->history = g_list_delete_link(iface->history, iface->history);
    }
    iface->nhistory = 0;
    
    repaint(iface);
}

static void repaint(NsaIface *iface)
{
    if (iface->painter != NULL)
	nsa_painter_repaint(iface->painter);
    gtk_widget_queue_draw(GTK_WIDGET(iface));
}

static void update(NsaIface *iface)
{
    struct val_t *vp = g_new(struct val_t, 1);
    int new_mag, new_mag_in, new_mag_out;
    gchar *s;
    
    get_bps(iface, vp);
    
    iface->history = g_list_prepend(iface->history, vp);
    iface->nhistory++;
    
    if (iface->history != NULL) {
	GList *lp;
	for (lp = iface->history; lp->next != NULL; lp = g_list_next(lp))
	    /*NOP*/ ;
	while (iface->nhistory > nsa_iface_get_width(iface)) {
	    GList *p = lp;
	    lp = g_list_previous(lp);
	    g_free(p->data);
	    iface->history = g_list_delete_link(iface->history, p);
	    iface->nhistory--;
	}
    }
    
    new_mag = calc_mag(iface, 3);
    new_mag_in = calc_mag(iface, 1);
    new_mag_out = calc_mag(iface, 2);
    
    if (new_mag_in < new_mag_out)
	new_mag_in = new_mag_out;
    if (new_mag_out < new_mag_in)
	new_mag_out = new_mag_in;
    
    if (new_mag != iface->mag || new_mag_in != iface->mag_in || new_mag_out != iface->mag_out) {
	iface->mag = new_mag;
	iface->mag_in = new_mag_in;
	iface->mag_out = new_mag_out;
	repaint(iface);
    } else {
	shift_paint(iface);
    }
    
    /***/
    
    if (iface->tooltips != NULL) {
	s = make_tips_string(iface);
	gtk_tooltips_set_tip(iface->tooltips, GTK_WIDGET(iface), s, s);
	g_free(s);
    }
}

static void shift_paint(NsaIface *iface)
{
    if (iface->painter != NULL)
	nsa_painter_shift_paint(iface->painter);
    gtk_widget_queue_draw(GTK_WIDGET(iface));
}

static int calc_mag(NsaIface *iface, int type)
{
    GList *lp;
    int mag = 1;
    
    switch (type) {
    case 1:
	for (lp = iface->history; lp != NULL; lp = g_list_next(lp)) {
	    struct val_t *vp = lp->data;
	    
	    if (vp->stat == IFSTAT_UP && vp->value_in > 0) {
		int this_mag = ceil(log(vp->value_in) / log(1024));
		if (mag < this_mag)
		    mag = this_mag;
	    }
	}
	break;
	
    case 2:
	for (lp = iface->history; lp != NULL; lp = g_list_next(lp)) {
	    struct val_t *vp = lp->data;
	    
	    if (vp->stat == IFSTAT_UP && vp->value_out > 0) {
		int this_mag = ceil(log(vp->value_out) / log(1024));
		if (mag < this_mag)
		    mag = this_mag;
	    }
	}
	break;
	
    case 3:
	for (lp = iface->history; lp != NULL; lp = g_list_next(lp)) {
	    struct val_t *vp = lp->data;
	    
	    if (vp->stat == IFSTAT_UP && (vp->value_in + vp->value_out) > 0) {
		int this_mag = ceil(log(vp->value_in + vp->value_out) / log(1024));
		if (mag < this_mag)
		    mag = this_mag;
	    }
	}
	break;
	
    }
    
    return mag;
}

static void get_bps(NsaIface *iface, struct val_t *vp)
{
#define GTIMEVAL_TO_SEC(tv)	\
	((tv).tv_sec + (gdouble) (tv).tv_usec / G_USEC_PER_SEC)
    
    struct ifstat_t *ifs;
    GTimeVal curtime;
    
    vp->stat = IFSTAT_DOWN;
    vp->value_in = vp->value_out = 0;
    
    g_get_current_time(&curtime);
    debug_log("cur: %f\n", curtime.tv_sec + curtime.tv_usec / 1000000.0);
    
    ifs = ifstat_get(iface->name, iface->is_ppp, iface->lock_file);
    
    if (iface->last_ifstat != NULL) {
	gdouble sec = 0;
	guint64 bits;
	
	sec = GTIMEVAL_TO_SEC(curtime)
		- GTIMEVAL_TO_SEC(iface->last_time);
	debug_log("sec=%f\n", sec);
	
	bits = (ifs->bytes_in - iface->last_ifstat->bytes_in) * 8;
	debug_log("bits_in=%d\n", bits);
	vp->value_in = bits / sec;
	
	bits = (ifs->bytes_out - iface->last_ifstat->bytes_out) * 8;
	debug_log("bits_out=%d\n", bits);
	vp->value_out = bits / sec;
	
	vp->stat = ifs->stat;
    }
    
    if (iface->last_ifstat != NULL)
	ifstat_free(iface->last_ifstat);
    iface->last_ifstat = ifs;
    iface->last_time = curtime;
    
#undef GTIMEVAL_TO_SEC
}

static gchar *make_bpsstr(gchar *buf, bps_t bps)
{
    if (bps >= 1024 * 1024 * 1024) {
	sprintf(buf, "%.1fGbps", bps / 1024.0 / 1024 / 1024);
    } else if (bps >= 1024 * 1024) {
	sprintf(buf, "%.1fMbps", bps / 1024.0 / 1024);
    } else if (bps >= 1024) {
	sprintf(buf, "%.1fKbps", bps / 1024.0);
    } else {
	sprintf(buf, "%.1fbps", (double) bps);
    }
    
    return buf;
}

static gchar *make_ipv6_addr_str(gchar *buf, guchar *addr)
{
    gushort a[8];
    int zerolen;
    int zeropos;
    int i;
    
    for (i = 0; i < 8; i++)
	a[i] = addr[i * 2 + 0] << 8 | addr[i * 2 + 1];
    
    zerolen = 0;
    zeropos = 0;
    for (i = 0; i < 8; i++) {
	if (a[i] == 0) {
	    int j;
	    for (j = i + 1; j < 8; j++) {
		if (a[j] != 0)
		    break;
	    }
	    if (j - i > zerolen) {
		zeropos = i;
		zerolen = j - i;
	    }
	}
    }
    
    if (zerolen > 0) {
	char a1[128] = { '\0', };
	char a2[128] = { '\0', };
	char *delim = "";
	
	for (i = 0; i < zeropos; i++) {
	    char t[16];
	    sprintf(t, "%s%x", delim, a[i]);
	    strcat(a1, t);
	    delim = ":";
	}
	
	delim = "";
	for (i = zeropos + zerolen; i < 8; i++) {
	    char t[16];
	    sprintf(t, "%s%x", delim, a[i]);
	    strcat(a2, t);
	    delim = ":";
	}
	
	sprintf(buf, "%s::%s", a1, a2);
    } else {
	sprintf(buf, "%x:%x:%x:%x:%x:%x:%x:%x",
		a[0], a[1], a[2], a[3],
		a[4], a[5], a[6], a[7]);
    }
    
    return buf;
}

static gchar *make_tips_string(NsaIface *iface)
{
    GString *buf = g_string_new("");
    gint i;
    
    g_string_append(buf, iface->name);
    
    if (iface->last_ifstat != NULL) {
	if (iface->last_ifstat->naddr4 > 0) {
	    struct ifstat_t *ifs = iface->last_ifstat;
	    g_string_append_printf(buf,
		    ":%d.%d.%d.%d/%d",
		    ifs->addr4->addr[0],
		    ifs->addr4->addr[1],
		    ifs->addr4->addr[2],
		    ifs->addr4->addr[3],
		    ifs->addr4->prefixlen);
	}
	
	for (i = 0; i < iface->last_ifstat->naddr6; i++) {
	    struct ifstat_t *ifs = iface->last_ifstat;
	    gchar addrbuf[128];
	    g_string_append_printf(buf,
		    "\n%s/%d",
		    make_ipv6_addr_str(addrbuf, ifs->addr6[i].addr),
		    ifs->addr6[i].prefixlen);
	}
    }
    
    if (iface->history != NULL) {
	struct val_t *vp = iface->history->data;
	if (vp->stat == IFSTAT_UP) {
	    gchar bpsbuf[16];
	    g_string_append_printf(buf, "\nOUT:%s", make_bpsstr(bpsbuf, vp->value_out));
	    g_string_append_printf(buf, "\nIN:%s", make_bpsstr(bpsbuf, vp->value_in));
	}
    }
    
    return g_string_free(buf, FALSE);
}

static gboolean nsa_iface_button_press(GtkWidget *w, GdkEventButton *ev)
{
    NsaIface *iface = NSA_IFACE(w);
    
    if (ev->button != 1)
	return FALSE;
    
    gchar *cmd;
    struct ifstat_t *ifs = ifstat_get(iface->name, iface->is_ppp, iface->lock_file);
    
    switch (ifs->stat) {
    case IFSTAT_DOWN:
	cmd = iface->cmd_down;
	break;
    case IFSTAT_NOADDR:
    case IFSTAT_UP:
	cmd = iface->cmd_up;
	break;
    default:
	cmd = "";
	break;
    }
    
    if (cmd != '\0')
	system(cmd);
    
    ifstat_free(ifs);
    
    return TRUE;
}

GtkWidget* nsa_iface_new(gboolean vert)
{
    GtkWidget *widget = g_object_new(NSA_TYPE_IFACE, NULL);
    nsa_iface_set_vert(NSA_IFACE(widget), vert);
    return widget;
}

GtkWidget *nsa_iface_new_with_params(
	const gchar *name,
	gboolean is_ppp,
	const gchar *lock_file,
	const struct color_t *colors,
	const gchar *cmd_up,
	const gchar *cmd_down,
	gint size,
	gboolean vert,
	gint painter_id,
	const gchar *fontname,
	GtkTooltips *tooltips)
{
    GtkWidget *widget = g_object_new(NSA_TYPE_IFACE, NULL);
    NsaIface *iface = NSA_IFACE(widget);
    int i;
    
    nsa_iface_set_tooltips(iface, tooltips);
    nsa_iface_set_font(iface, fontname);
    nsa_iface_set_vert(iface, vert);
    nsa_iface_set_painter(iface, painter_id);
    nsa_iface_set_size(iface, size);
    nsa_iface_set_name(iface, name);
    nsa_iface_set_is_ppp(iface, is_ppp);
    for (i = 0; i < COL_NR; i++)
	nsa_iface_set_color(iface, i, &colors[i]);
    nsa_iface_set_lock_file(iface, lock_file);
    nsa_iface_set_cmd_up(iface, cmd_up);
    nsa_iface_set_cmd_down(iface, cmd_down);
    
    return widget;
}

GtkWidget *nsa_iface_new_from_iface(
	NsaIface *from_iface)
{
    GtkWidget *widget = g_object_new(NSA_TYPE_IFACE, NULL);
    NsaIface *iface = NSA_IFACE(widget);
    int i;
    
    nsa_iface_set_tooltips(iface, from_iface->tooltips);
    nsa_iface_set_font(iface, from_iface->fontname);
    nsa_iface_set_vert(iface, from_iface->vert);
    nsa_iface_set_painter(iface, from_iface->painter_id);
    nsa_iface_set_size(iface, from_iface->size);
    nsa_iface_set_name(iface, from_iface->name);
    nsa_iface_set_is_ppp(iface, from_iface->is_ppp);
    for (i = 0; i < COL_NR; i++)
	nsa_iface_set_color(iface, i, &from_iface->color[i]);
    nsa_iface_set_lock_file(iface, from_iface->lock_file);
    nsa_iface_set_cmd_up(iface, from_iface->cmd_up);
    nsa_iface_set_cmd_down(iface, from_iface->cmd_down);
    
    return widget;
}
