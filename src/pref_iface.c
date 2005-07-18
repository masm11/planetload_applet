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
 * $Id: pref_iface.c 21 2005-07-18 07:57:00Z masm $
 */

#include "../config.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <libgnomeui/gnome-color-picker.h>

#include "app.h"
#include "i18n-support.h"

struct prop_t {
    GtkWidget *iface;
    
    GtkWidget *dialog;
    GtkWidget *name;
    GtkWidget *ppp;
    GtkWidget *lock_file;
    GtkWidget *cmd_up, *cmd_down;
    GtkWidget *width;
};

static void color_picker_set_color(GtkWidget *w, const struct color_t *col)
{
    gnome_color_picker_set_i16(GNOME_COLOR_PICKER(w),
	    col->r, col->g, col->b, col->a);
}

static void color_picker_get_color(GtkWidget *w, struct color_t *col)
{
    gnome_color_picker_get_i16(GNOME_COLOR_PICKER(w),
	    &col->r, &col->g, &col->b, &col->a);
}

static void iface_changed_cb(GtkWidget *w, gpointer closure)
{
    struct prop_t *pp = closure;
    const gchar *name = gtk_entry_get_text(GTK_ENTRY(w));
    
    nsa_iface_set_name(NSA_IFACE(pp->iface), name);
}

static void ppp_changed_cb(GtkWidget *w, gpointer closure)
{
    struct prop_t *pp = closure;
    gboolean onoff = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    
    nsa_iface_set_is_ppp(NSA_IFACE(pp->iface), onoff);
}

static void lock_file_changed_cb(GtkWidget *w, gpointer closure)
{
    struct prop_t *pp = closure;
    const gchar *file = gtk_entry_get_text(GTK_ENTRY(w));
    
    nsa_iface_set_lock_file(NSA_IFACE(pp->iface), file);
}

static void size_changed_cb(GtkWidget *w, gpointer closure)
{
    struct prop_t *pp = closure;
    gint size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w));
    
    if (size <= 0)
	return;
    
    nsa_iface_set_size(NSA_IFACE(pp->iface), size);
}

static void command_up_changed_cb(GtkWidget *w, gpointer closure)
{
    struct prop_t *pp = closure;
    const gchar *cmd_up = gtk_entry_get_text(GTK_ENTRY(w));
    
    nsa_iface_set_cmd_up(NSA_IFACE(pp->iface), cmd_up);
}

static void command_down_changed_cb(GtkWidget *w, gpointer closure)
{
    struct prop_t *pp = closure;
    const gchar *cmd_down = gtk_entry_get_text(GTK_ENTRY(w));
    
    nsa_iface_set_cmd_down(NSA_IFACE(pp->iface), cmd_down);
}

static void color_set_cb(GtkWidget *w,
	guint arg1, guint arg2, guint arg3, guint arg4, gpointer closure)
{
    struct prop_t *pp = closure;
    int type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w), "color_kind"));
    struct color_t color;
    
    if (type < 0 || type >= COL_NR)
	return;
    
    color_picker_get_color(w, &color);
    nsa_iface_set_color(NSA_IFACE(pp->iface), type, &color);
}

static GtkWidget *make_color_frame(struct prop_t *pp)
{
    static struct design_table_t {
	int x, y;
	int kind;	// 0:label, 1:color
	gchar *label;
	int coltype;
    } table[] = {
	{ 0, 0, 0, N_("Text:"),   0, },
	{ 1, 0, 0, N_("UP:"),   0, },
	{ 2, 0, 1, NULL,          COL_TEXT, },
	{ 3, 0, 0, N_("DIAL:"), 0, },
	{ 4, 0, 1, NULL,          COL_TEXT_NOADDR, },
	{ 5, 0, 0, N_("DOWN:"), 0, },
	{ 6, 0, 1, NULL,          COL_TEXT_DOWN, },
	
	{ 0, 1, 0, N_("Bar:"),        0, },
	{ 2, 1, 1, NULL,          COL_HBAR, },
	
	{ 0, 2, 0, N_("Foreground:"), 0, },
	{ 1, 2, 0, N_("OUT:"), 0, },
	{ 2, 2, 1, NULL,          COL_FG2, },
	{ 3, 2, 0, N_("IN:"), 0, },
	{ 4, 2, 1, NULL,          COL_FG, },
	
	{ 0, 3, 0, N_("Background:"), 0, },
	{ 1, 3, 0, N_("UP:"), 0, },
	{ 2, 3, 1, NULL,          COL_BG, },
	{ 3, 3, 0, N_("DIAL:"), 0, },
	{ 4, 3, 1, NULL,          COL_BG_NOADDR, },
	{ 5, 3, 0, N_("DOWN:"), 0, },
	{ 6, 3, 1, NULL,          COL_DOWN, },
    };
    static int coltab_cols = 7, coltab_rows = 4;
    GtkWidget *frame, *tbl;
    int i;
    
    frame = gtk_frame_new(_("Colors"));
    
    tbl = gtk_table_new(coltab_cols, coltab_rows, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), tbl);
    gtk_container_set_border_width(GTK_CONTAINER(tbl), 5);
    gtk_table_set_row_spacings(GTK_TABLE(tbl), 3);
    gtk_table_set_col_spacings(GTK_TABLE(tbl), 0);
    
    for (i = 0; i < sizeof table / sizeof table[0]; i++) {
	struct design_table_t *tp = &table[i];
	GtkWidget *w;
	
	switch (tp->kind) {
	case 0:
	    w = gtk_label_new(_(tp->label));
	    gtk_table_attach_defaults(GTK_TABLE(tbl), w,
		    tp->x, tp->x + 1, tp->y, tp->y + 1);
	    break;
	    
	case 1:
	    w = gnome_color_picker_new();
	    gtk_table_attach_defaults(GTK_TABLE(tbl), w,
		    tp->x, tp->x + 1, tp->y, tp->y + 1);
	    gnome_color_picker_set_use_alpha(GNOME_COLOR_PICKER(w), TRUE);
	    color_picker_set_color(w,
		    nsa_iface_get_color(NSA_IFACE(pp->iface), tp->coltype));
	    g_object_set_data(G_OBJECT(w), "color_kind", GINT_TO_POINTER(tp->coltype));
	    g_signal_connect(G_OBJECT(w), "color_set",
		    G_CALLBACK(color_set_cb), pp);
	    break;
	}
    }
    
    return frame;
}

static void destroy_cb(GtkObject *obj, gpointer closure)
{
    struct prop_t *pp = closure;
    
    g_object_set_data_full(G_OBJECT(pp->iface), "preferences_dialog", NULL, NULL);
    g_free(pp);
}

static void close_cb(GtkWidget *widget, gint arg, gpointer closure)
{
    struct prop_t *pp = closure;
    
    gtk_widget_destroy(pp->dialog);
}

void prop_iface(GtkWidget *iface)
{
    GtkWidget *dialog;
    GtkWidget *vbox, *frame, *tbl, *lb;
    struct prop_t *pp;
    
    dialog = g_object_get_data(G_OBJECT(iface), "preferences_dialog");
    if (dialog != NULL) {
	gdk_window_raise(dialog->window);
	return;
    }
    
    pp = g_new(struct prop_t, 1);
    
    memset(pp, 0, sizeof *pp);
    
    pp->iface = iface;
    
    dialog = gtk_dialog_new_with_buttons(
	    _("Interface Settings"), NULL,
	    GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
    pp->dialog = dialog;
    
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
	    vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    
    tbl = gtk_table_new(2, 6, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), tbl, FALSE, FALSE, 0);
    
    
    lb = gtk_label_new(_("Network Interface:"));
    gtk_table_attach_defaults(GTK_TABLE(tbl), lb, 0, 1, 0, 1);
    
    pp->name = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(tbl), pp->name, 1, 2, 0, 1);
    gtk_entry_set_text(GTK_ENTRY(pp->name), nsa_iface_get_name(NSA_IFACE(pp->iface)));
    g_signal_connect(G_OBJECT(pp->name), "changed",
	    G_CALLBACK(iface_changed_cb), pp);
    
    lb = gtk_label_new(_("PPP:"));
    gtk_table_attach_defaults(GTK_TABLE(tbl), lb, 0, 1, 1, 2);
    
    pp->ppp = gtk_check_button_new();
    gtk_table_attach_defaults(GTK_TABLE(tbl), pp->ppp, 1, 2, 1, 2);
    
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pp->ppp),
	    nsa_iface_get_is_ppp(NSA_IFACE(pp->iface)));
    g_signal_connect(G_OBJECT(pp->ppp), "toggled",
	    G_CALLBACK(ppp_changed_cb), pp);
    
    lb = gtk_label_new(_("PPP Lock File:"));
    gtk_table_attach_defaults(GTK_TABLE(tbl), lb, 0, 1, 2, 3);
    
    pp->lock_file = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(tbl), pp->lock_file, 1, 2, 2, 3);
    gtk_entry_set_text(GTK_ENTRY(pp->lock_file), nsa_iface_get_lock_file(NSA_IFACE(pp->iface)));
    g_signal_connect(G_OBJECT(pp->lock_file), "changed",
	    G_CALLBACK(lock_file_changed_cb), pp);
    
    lb = gtk_label_new(_("Size:"));
    gtk_table_attach_defaults(GTK_TABLE(tbl), lb, 0, 1, 3, 4);
    
    pp->width = gtk_spin_button_new_with_range(1, 10000, 1);
    gtk_table_attach_defaults(GTK_TABLE(tbl), pp->width, 1, 2, 3, 4);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(pp->width), nsa_iface_get_size(NSA_IFACE(pp->iface)));
    g_signal_connect(G_OBJECT(pp->width), "value-changed",
	    G_CALLBACK(size_changed_cb), pp);
    
    lb = gtk_label_new(_("Click Command When Up:"));
    gtk_table_attach_defaults(GTK_TABLE(tbl), lb, 0, 1, 4, 5);
    
    pp->cmd_up = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(tbl), pp->cmd_up, 1, 2, 4, 5);
    gtk_entry_set_text(GTK_ENTRY(pp->cmd_up), nsa_iface_get_cmd_up(NSA_IFACE(pp->iface)));
    g_signal_connect(G_OBJECT(pp->cmd_up), "changed",
	    G_CALLBACK(command_up_changed_cb), pp);
    
    lb = gtk_label_new(_("Click Command When Down:"));
    gtk_table_attach_defaults(GTK_TABLE(tbl), lb, 0, 1, 5, 6);
    
    pp->cmd_down = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(tbl), pp->cmd_down, 1, 2, 5, 6);
    gtk_entry_set_text(GTK_ENTRY(pp->cmd_down), nsa_iface_get_cmd_down(NSA_IFACE(pp->iface)));
    g_signal_connect(G_OBJECT(pp->cmd_down), "changed",
	    G_CALLBACK(command_down_changed_cb), pp);
    
    frame = make_color_frame(pp);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
    
    gtk_widget_show_all(vbox);
    
    g_object_set_data_full(G_OBJECT(iface), "preferences_dialog",
	    dialog, (GDestroyNotify) gtk_widget_destroy);
    g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(destroy_cb), pp);
    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(close_cb), pp);
    
    gtk_widget_show(dialog);
}

/*EOF*/
