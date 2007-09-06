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

#include "../config.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#ifdef HAVE_GNOME
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#endif
#include <gtk/gtkfontbutton.h>

#include "app.h"
#include "appconf.h"
#include "debug.h"
#include "planetload_applet.h"
#include "pref_iface.h"
#include "preferences.h"
#include "i18n-support.h"

enum {
    LIST_COL_NAME,
    LIST_COL_W,
    LIST_COL_NR
};

struct dialog_t {
    struct app_t *app;
    
    GtkWidget *dialog;
    GtkWidget *list;
    GtkListStore *store;
    
    GtkWidget *btn_cp, *btn_rm, *btn_ed;
    GtkWidget *btn_up, *btn_dn;
    
    GtkWidget *planet_table;
};

static void name_changed_cb(GtkWidget *widget, gpointer closure)
{
    struct dialog_t *dp = closure;
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(dp->list));
    GtkTreeIter iter;
    
    if (!gtk_tree_model_get_iter_first(model, &iter))
	return;
    
    do {
	GtkWidget *w;
	gtk_tree_model_get(model, &iter, LIST_COL_W, &w, -1);
	if (w == widget) {
	    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
		    LIST_COL_NAME, nsa_iface_get_name(NSA_IFACE(w)),
		    -1);
	}
    } while (gtk_tree_model_iter_next(model, &iter));
}

static void append_iface(GtkWidget *w, gpointer data)
{
    struct dialog_t *dp = data;
    GtkListStore *store = dp->store;
    GtkTreeIter iter;
    
    gtk_list_store_append(store, &iter);
    
    gtk_list_store_set(store, &iter,
	    LIST_COL_NAME, nsa_iface_get_name(NSA_IFACE(w)),
	    LIST_COL_W, w,
	    -1);
    
    g_signal_connect(G_OBJECT(w), "name_changed",
	    G_CALLBACK(name_changed_cb), dp);
}

static void remove_iface_signal_handler(GtkWidget *w, gpointer data)
{
    g_signal_handlers_disconnect_by_func(w, name_changed_cb, data);
}

static void update_buttons_sensitive(struct dialog_t *dp)
{
    GtkTreeView *view = GTK_TREE_VIEW(dp->list);
    GtkTreeSelection *sel;
    GtkTreeModel *model;
    GtkTreeIter iter, it;
    GtkTreePath *path;
    gboolean sens_rm, sens_up, sens_dn;
    
    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    
    if (!gtk_tree_selection_get_selected(sel, &model, &iter)) {
	gtk_widget_set_sensitive(dp->btn_cp, FALSE);
	gtk_widget_set_sensitive(dp->btn_rm, FALSE);
	gtk_widget_set_sensitive(dp->btn_ed, FALSE);
	gtk_widget_set_sensitive(dp->btn_up, FALSE);
	gtk_widget_set_sensitive(dp->btn_dn, FALSE);
	return;
    }
    
    it = iter;
    if (gtk_tree_model_iter_next(model, &it))
	sens_dn = TRUE;
    else
	sens_dn = FALSE;
    
    path = gtk_tree_model_get_path(model, &iter);
    if (gtk_tree_path_prev(path))
	sens_up = TRUE;
    else
	sens_up = FALSE;
    
    // 上にも下にも移動できない
    // → 1個しかない。
    // → 削除させない。
    sens_rm = (sens_up | sens_dn);
    
    gtk_widget_set_sensitive(dp->btn_cp, TRUE);
    gtk_widget_set_sensitive(dp->btn_rm, sens_rm);
    gtk_widget_set_sensitive(dp->btn_ed, TRUE);
    gtk_widget_set_sensitive(dp->btn_up, sens_up);
    gtk_widget_set_sensitive(dp->btn_dn, sens_dn);
}

static void move(struct dialog_t *dp, int dir)
{
    GtkTreeIter iter;
    GtkTreeSelection *sel;
    GtkTreeModel *model;
    
    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(dp->list));
    
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
	GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
	GtkTreeIter iter_to;
	
	if (dir < 0)
	    gtk_tree_path_prev(path);
	else
	    gtk_tree_path_next(path);
	
	if (gtk_tree_model_get_iter(model, &iter_to, path))
	    gtk_list_store_swap(GTK_LIST_STORE(model), &iter, &iter_to);
	
	gtk_tree_path_free(path);
    }
    
    if (gtk_tree_model_get_iter_first(model, &iter)) {
	int pos = 0;
	do {
	    GtkWidget *w;
	    gtk_tree_model_get(model, &iter, LIST_COL_W, &w, -1);
	    gtk_box_reorder_child(GTK_BOX(dp->app->pack), w, pos);
	    pos++;
	} while (gtk_tree_model_iter_next(model, &iter));
    }
    
    update_buttons_sensitive(dp);
}

static void up_cb(GtkWidget *w, gpointer data)
{
    struct dialog_t *dp = data;
    move(dp, -1);
}

static void down_cb(GtkWidget *w, gpointer data)
{
    struct dialog_t *dp = data;
    move(dp, 1);
}

#if 0
static void add_cb(GtkWidget *w, gpointer data)
{
    struct dialog_t *dp = data;
    
    w = nsa_iface_new(dp->app->is_vert);
    nsa_iface_set_font(NSA_IFACE(w), dp->app->fontname);
    nsa_iface_set_painter(NSA_IFACE(w), dp->app->painter);
    nsa_iface_set_tooltips(NSA_IFACE(w), dp->app->tooltips);
    gtk_widget_show_all(w);
    gtk_box_pack_start(GTK_BOX(dp->app->pack), w, FALSE, FALSE, 0);
    
    append_iface(w, dp);
    
    update_buttons_sensitive(dp);
}
#endif

static void copy_cb(GtkWidget *w, gpointer data)
{
    struct dialog_t *dp = data;
    GtkTreeIter iter;
    GtkTreeSelection *sel;
    GtkTreeModel *model;
    
    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(dp->list));
    
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
	GtkWidget *ws, *wd;	// w.src, w.dst
	
	gtk_tree_model_get(model, &iter, LIST_COL_W, &ws, -1);
	
	wd = nsa_iface_new_from_iface(NSA_IFACE(ws));
	
	gtk_widget_show_all(wd);
	gtk_box_pack_start(GTK_BOX(dp->app->pack), wd, FALSE, FALSE, 0);
	
	append_iface(wd, dp);
	
	update_buttons_sensitive(dp);
    }
}

static void edit_cb(GtkWidget *w, gpointer data)
{
    struct dialog_t *dp = data;
    GtkTreeIter iter;
    GtkTreeSelection *sel;
    GtkTreeModel *model;
    
    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(dp->list));
    
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
	GtkWidget *w;
	gtk_tree_model_get(model, &iter, LIST_COL_W, &w, -1);
	prop_iface(w);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
		LIST_COL_NAME, nsa_iface_get_name(NSA_IFACE(w)),
		-1);
    }
}

static void remove_cb(GtkWidget *w, gpointer data)
{
    struct dialog_t *dp = data;
    GtkTreeIter iter;
    GtkTreeSelection *sel;
    GtkTreeModel *model;
    
    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(dp->list));
    
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
	GtkWidget *w;
	
	// 2つ以上あることを確認。
	{
	    GtkTreeIter it;
	    if (!gtk_tree_model_get_iter_first(model, &it))
		return;
	    if (!gtk_tree_model_iter_next(model, &it))
		return;
	}
	
	gtk_tree_model_get(model, &iter, LIST_COL_W, &w, -1);
	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	gtk_widget_destroy(w);
	
	update_buttons_sensitive(dp);
    }
}

static void cursor_changed(GtkWidget *w, gpointer data)
{
    struct dialog_t *dp = data;
    
    update_buttons_sensitive(dp);
}

static GtkWidget *make_interface_pane(struct dialog_t *dp)
{
    struct app_t *app = dp->app;
    GtkWidget *frame, *hbox;
    
    frame = gtk_frame_new(_("Interfaces"));
    
    hbox = gtk_hbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    
    {
	GtkListStore *store;
	GtkWidget *view;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	store = gtk_list_store_new(LIST_COL_NR, G_TYPE_STRING, G_TYPE_POINTER);
	dp->store = store;
	
	gtk_container_foreach(GTK_CONTAINER(app->pack), append_iface, dp);
	
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	dp->list = view;
	gtk_box_pack_start(GTK_BOX(hbox), view, FALSE, FALSE, 0);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
	g_signal_connect(G_OBJECT(view), "cursor-changed",
		G_CALLBACK(cursor_changed), dp);
	
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(
		_("Interfaces"), renderer,
		"text", LIST_COL_NAME,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
    }
    
    {
	GtkWidget *w, *vbox2;
	
	vbox2 = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, FALSE, FALSE, 0);
	
#if 0
	w = gtk_button_new_with_label(_("Add"));
	gtk_box_pack_start(GTK_BOX(vbox2), w, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(add_cb), dp);
#endif
	
	w = gtk_button_new_with_label(_("Copy"));
	dp->btn_cp = w;
	gtk_box_pack_start(GTK_BOX(vbox2), w, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(copy_cb), dp);
	
	w = gtk_button_new_with_label(_("Remove"));
	dp->btn_rm = w;
	gtk_box_pack_start(GTK_BOX(vbox2), w, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(remove_cb), dp);
	
#if 0
	w = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox2), w, FALSE, FALSE, 0);
#endif
	
	w = gtk_button_new_with_label(_("Edit..."));
	dp->btn_ed = w;
	gtk_box_pack_start(GTK_BOX(vbox2), w, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(edit_cb), dp);
	
	w = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox2), w, FALSE, FALSE, 0);
	
	w = gtk_button_new_with_label(_("Up"));
	dp->btn_up = w;
	gtk_box_pack_start(GTK_BOX(vbox2), w, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(up_cb), dp);
	
	w = gtk_button_new_with_label(_("Down"));
	dp->btn_dn = w;
	gtk_box_pack_start(GTK_BOX(vbox2), w, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(w), "clicked", G_CALLBACK(down_cb), dp);
    }
    
    return frame;
}

static void font_set_cb(GtkWidget *w, gpointer closure)
{
    struct dialog_t *dp = closure;
    struct app_t *app = dp->app;
    const gchar *font;
    
    font = gtk_font_button_get_font_name(GTK_FONT_BUTTON(w));
    app->fontname = g_strdup(font);
    
    font_change(app);
}

static void interval_changed_cb(GtkWidget *w, gpointer closure)
{
    struct dialog_t *dp = closure;
    struct app_t *app = dp->app;
    gint interval = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w));
    
    if (interval <= 0)
	return;
    
    app->interval = interval;
    
    timer_set(app);
}

static void painter_fine_cb(GtkWidget *w, gpointer closure)
{
    struct dialog_t *dp = closure;
    struct app_t *app = dp->app;
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))) {
	app->painter = NSA_PAINTER_TYPE_FINE;
	painter_change(app);
    }
}

static void painter_light_cb(GtkWidget *w, gpointer closure)
{
    struct dialog_t *dp = closure;
    struct app_t *app = dp->app;
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))) {
	app->painter = NSA_PAINTER_TYPE_LIGHT;
	painter_change(app);
    }
}

static void painter_split_cb(GtkWidget *w, gpointer closure)
{
    struct dialog_t *dp = closure;
    struct app_t *app = dp->app;
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))) {
	app->painter = NSA_PAINTER_TYPE_SPLIT;
	painter_change(app);
    }
}

static GtkWidget *make_options_pane(struct dialog_t *dp)
{
    struct app_t *app = dp->app;
    GtkWidget *frame, *table;
    GtkWidget *w;
    
    frame = gtk_frame_new(_("Options"));
    
    table = gtk_table_new(4, 3, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_container_set_border_width(GTK_CONTAINER(table), 5);
    
    w = gtk_label_new(_("Font:"));
    gtk_table_attach_defaults(GTK_TABLE(table), w, 0, 1, 0, 1);
    gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_RIGHT);
    
    w = gtk_font_button_new();
    gtk_table_attach_defaults(GTK_TABLE(table), w, 1, 4, 0, 1);
//    gtk_font_button_set_mode(GTK_FONT_BUTTON(w), GTK_FONT_BUTTON_MODE_FONT_INFO);
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(w), app->fontname);
    g_signal_connect(G_OBJECT(w), "font-set", G_CALLBACK(font_set_cb), dp);
    
    w = gtk_label_new(_("Update Interval:"));
    gtk_table_attach_defaults(GTK_TABLE(table), w, 0, 1, 1, 2);
    gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_RIGHT);
    
    w = gtk_spin_button_new_with_range(1, 100000, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), w, 1, 4, 1, 2);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), app->interval);
    g_signal_connect(G_OBJECT(w), "value-changed", G_CALLBACK(interval_changed_cb), dp);
    
    w = gtk_label_new(_("Painter:"));
    gtk_table_attach_defaults(GTK_TABLE(table), w, 0, 1, 2, 3);
    gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_RIGHT);
    
    w = gtk_radio_button_new_with_label(NULL, _("Fine"));
    gtk_table_attach_defaults(GTK_TABLE(table), w, 1, 2, 2, 3);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), app->painter == 0);
    g_signal_connect(G_OBJECT(w), "toggled",
	    G_CALLBACK(painter_fine_cb), dp);
    
    w = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(w), _("Light"));
    gtk_table_attach_defaults(GTK_TABLE(table), w, 2, 3, 2, 3);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), app->painter == 1);
    g_signal_connect(G_OBJECT(w), "toggled",
	    G_CALLBACK(painter_light_cb), dp);
    
    w = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(w), _("Split"));
    gtk_table_attach_defaults(GTK_TABLE(table), w, 3, 4, 2, 3);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), app->painter == 2);
    g_signal_connect(G_OBJECT(w), "toggled",
	    G_CALLBACK(painter_split_cb), dp);

    return frame;
}

static void cmd_list_changed_cb(GtkWidget *w, gpointer closure)
{
    struct dialog_t *dp = closure;
    struct app_t *app = dp->app;
    const gchar *cmd = gtk_entry_get_text(GTK_ENTRY(w));
    
    if (app->cmd_to_get_scheme_list != NULL)
	g_free(app->cmd_to_get_scheme_list);
    app->cmd_to_get_scheme_list = g_strdup(cmd);
}

static void cmd_chg_changed_cb(GtkWidget *w, gpointer closure)
{
    struct dialog_t *dp = closure;
    struct app_t *app = dp->app;
    const gchar *cmd = gtk_entry_get_text(GTK_ENTRY(w));
    
    if (app->cmd_to_change_scheme != NULL)
	g_free(app->cmd_to_change_scheme);
    app->cmd_to_change_scheme = g_strdup(cmd);
}

static void scheme_label_on_cb(GtkWidget *w, gpointer closure)
{
    struct dialog_t *dp = closure;
    struct app_t *app = dp->app;
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))) {
	app->display_scheme = TRUE;
	disp_scheme_onoff(app);
    }
}

static void scheme_label_off_cb(GtkWidget *w, gpointer closure)
{
    struct dialog_t *dp = closure;
    struct app_t *app = dp->app;
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))) {
	app->display_scheme = FALSE;
	disp_scheme_onoff(app);
    }
}

static void use_planet_toggled_cb(GtkWidget *w, gpointer closure)
{
    struct dialog_t *dp = closure;
    struct app_t *app = dp->app;
    
    app->use_planet = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
    gtk_widget_set_sensitive(dp->planet_table, app->use_planet);
    
    disp_scheme_onoff(app);
    setup_schemes_menu(app);
}

static void destroy_cb(GtkWidget *widget, gpointer closure)
{
    struct dialog_t *dp = closure;
    struct app_t *app = dp->app;
    
    gtk_container_foreach(GTK_CONTAINER(app->pack), remove_iface_signal_handler, dp);
    
    debug_log("destroy_cb: NULL.\n");
    app->dialog = NULL;
    debug_log("destroy_cb: free.\n");
    g_free(dp);
    debug_log("destroy_cb: done.\n");
}

static void close_cb(GtkWidget *widget, gint arg, gpointer closure)
{
    struct dialog_t *dp = closure;
    struct app_t *app = dp->app;
    
    debug_log("close_cb: arg=%d.\n", arg);
    debug_log("close_cb: save.\n");
    appconf_save(app);
    
    debug_log("close_cb: destroy.\n");
    gtk_widget_destroy(dp->dialog);
    
    debug_log("close_cb: done.\n");
}

GtkWidget *make_planet_pane(struct dialog_t *dp)
{
    struct app_t *app = dp->app;
    GtkWidget *frame, *table;
    GtkWidget *w;
    GtkWidget *frame_label;
    
    frame = gtk_frame_new(NULL);
    
    frame_label = gtk_check_button_new_with_label(_("Planet"));
    gtk_frame_set_label_widget(GTK_FRAME(frame), frame_label);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(frame_label), app->use_planet);
    g_signal_connect(G_OBJECT(frame_label), "toggled",
	    G_CALLBACK(use_planet_toggled_cb), dp);
    
    table = gtk_table_new(3, 3, FALSE);
    dp->planet_table = table;
    gtk_widget_set_sensitive(dp->planet_table, app->use_planet);
    gtk_container_add(GTK_CONTAINER(frame), table);
    gtk_container_set_border_width(GTK_CONTAINER(table), 5);
    
    
    w = gtk_label_new(_("Command to Get Scheme List:"));
    gtk_table_attach_defaults(GTK_TABLE(table), w, 0, 1, 0, 1);
    gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_RIGHT);
    
    w = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(table), w, 1, 3, 0, 1);
    gtk_entry_set_text(GTK_ENTRY(w), app->cmd_to_get_scheme_list);
    g_signal_connect(G_OBJECT(w), "changed",
	    G_CALLBACK(cmd_list_changed_cb), dp);
    
    w = gtk_label_new(_("Command to Change Scheme:"));
    gtk_table_attach_defaults(GTK_TABLE(table), w, 0, 1, 1, 2);
    gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_RIGHT);
    
    w = gtk_entry_new();
    gtk_table_attach_defaults(GTK_TABLE(table), w, 1, 3, 1, 2);
    gtk_entry_set_text(GTK_ENTRY(w), app->cmd_to_change_scheme);
    g_signal_connect(G_OBJECT(w), "changed",
	    G_CALLBACK(cmd_chg_changed_cb), dp);
    
    w = gtk_label_new(_("Scheme Label:"));
    gtk_table_attach_defaults(GTK_TABLE(table), w, 0, 1, 2, 3);
    gtk_label_set_justify(GTK_LABEL(w), GTK_JUSTIFY_RIGHT);
    
    w = gtk_radio_button_new_with_label(NULL, "ON");
    gtk_table_attach_defaults(GTK_TABLE(table), w, 1, 2, 2, 3);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), app->display_scheme);
    g_signal_connect(G_OBJECT(w), "toggled",
	    G_CALLBACK(scheme_label_on_cb), dp);
    
    w = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(w), "OFF");
    gtk_table_attach_defaults(GTK_TABLE(table), w, 2, 3, 2, 3);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), !app->display_scheme);
    g_signal_connect(G_OBJECT(w), "toggled",
	    G_CALLBACK(scheme_label_off_cb), dp);
    
    return frame;
}

#ifdef HAVE_GNOME
void preferences(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
    struct app_t *app = data;
    GtkWidget *dialog;
    GtkWidget *hbox, *vbox, *w;
    struct dialog_t *dp;
    
    if (app->dialog != NULL)
	return;
    
    dp = g_new(struct dialog_t, 1);
    app->dialog = dp;
    
    memset(dp, 0, sizeof *dp);
    
    dp->app = app;
    
    dialog = dp->dialog = gtk_dialog_new_with_buttons(
	    _("Planetload Applet Settings"), NULL,
	    GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
    
    hbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    
    w = make_interface_pane(dp);
    gtk_box_pack_start(GTK_BOX(hbox), w, FALSE, FALSE, 0);
    
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
    
    w = make_options_pane(dp);
    gtk_box_pack_start(GTK_BOX(vbox), w, FALSE, FALSE, 0);
    
    w = make_planet_pane(dp);
    gtk_box_pack_start(GTK_BOX(vbox), w, FALSE, FALSE, 0);
    
    gtk_widget_show_all(hbox);
    
    g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(destroy_cb), dp);
    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(close_cb), dp);
    
    gtk_widget_show(dialog);
}
#endif

#ifdef HAVE_XFCE4
void preferences(XfcePanelPlugin *plugin, gpointer data)
{
    struct app_t *app = data;
    GtkWidget *dialog;
    GtkWidget *hbox, *vbox, *w;
    struct dialog_t *dp;
    
    if (app->dialog != NULL)
	return;
    
    dp = g_new(struct dialog_t, 1);
    app->dialog = dp;
    
    memset(dp, 0, sizeof *dp);
    
    dp->app = app;
    
    dialog = dp->dialog = gtk_dialog_new_with_buttons(
	    _("Planetload Applet Settings"), NULL,
	    GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
    
    hbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    
    w = make_interface_pane(dp);
    gtk_box_pack_start(GTK_BOX(hbox), w, FALSE, FALSE, 0);
    
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
    
    w = make_options_pane(dp);
    gtk_box_pack_start(GTK_BOX(vbox), w, FALSE, FALSE, 0);
    
    w = make_planet_pane(dp);
    gtk_box_pack_start(GTK_BOX(vbox), w, FALSE, FALSE, 0);
    
    gtk_widget_show_all(hbox);
    
    g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(destroy_cb), dp);
    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(close_cb), dp);
    
    gtk_widget_show(dialog);
}
#endif

void preferences_destroy(struct app_t *app)
{
    if (app->dialog == NULL)
	return;
    
    gtk_widget_destroy(app->dialog->dialog);
}

/*EOF*/
