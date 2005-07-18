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
 * $Id: planetload_applet.c 21 2005-07-18 07:57:00Z masm $
 */

#include "../config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include <panel-applet.h>

#include "about.h"
#include "app.h"
#include "appconf.h"
#include "i18n-support.h"

#define SCHEME_FILE	"/var/lib/net/scheme"

void preferences(BonoboUIComponent *uic, gpointer data, const gchar *verbname);

/**************** scheme ****/

static gchar *scheme_get_current(struct app_t *app, gchar *buffer)
{
    FILE *fp = NULL;
    
    if (!app->use_planet) {
	buffer[0] = '\0';
	return buffer;
    }
    
    if ((fp = fopen(SCHEME_FILE, "rt")) == NULL)
	goto err;
    
    if (fscanf(fp, "%s", buffer) != 1)
	goto err;
    
    fclose(fp);
    
    return buffer;
    
 err:
    if (fp != NULL)
	fclose(fp);
    buffer[0] = '\0';
    return buffer;
}

static GList *scheme_get_list(struct app_t *app)
{
    gchar *line = NULL, *p;
    FILE *fp = NULL;
    GList *list;
    int st;
    
    if (!app->use_planet)
	return NULL;
    
    debug_log("scheme_get_list: %s\n", app->cmd_to_get_scheme_list);
    if ((fp = popen(app->cmd_to_get_scheme_list, "r")) == NULL) {
	debug_log("scheme_get_list: %s\n", strerror(errno));
	goto err;
    }
    
    line = g_new(gchar, 1024);
    
    while (fgets(line, 1024, fp) == NULL) {
	if (feof(fp)) {
	    debug_log("scheme_get_list: eof\n");
	    goto err;
	}
	if (!ferror(fp)) {
	    debug_log("scheme_get_list: ???\n");
	    goto err;
	}
	if (errno == EINTR)
	    continue;
	debug_log("scheme_get_list: %s\n", strerror(errno));
	goto err;
    }
    
    if ((st = pclose(fp)) != 0) {
	debug_log("scheme_get_list: st=%d\n", st);
	goto err;
    }
    
    if ((p = strchr(line, '\r')) != NULL)
	*p = '\0';
    if ((p = strchr(line, '\n')) != NULL)
	*p = '\0';
    
    list = NULL;
    
    p = line;
    while ((p = strtok(p, " \t\r\n")) != NULL) {
	list = g_list_append(list, g_strdup(p));
	p = NULL;
    }
    g_free(line);
    
    return list;
    
 err:
    if (fp != NULL)
	pclose(fp);
    if (line != NULL)
	g_free(line);
    return NULL;
}

static void scheme_change(struct app_t *app, gchar *name)
{
    if (app->use_planet) {
	gchar *cmd = g_strdup_printf(app->cmd_to_change_scheme, name);
	system(cmd);
	g_free(cmd);
    }
}

static void scheme_cb(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
    struct app_t *app = data;
    gchar *scheme;
    
    if ((scheme = strchr(verbname, '.')) != NULL) {
	scheme++;
	scheme_change(app, scheme);
    }
}

/**************** left-menu ****/

static void lmenu_cb(GtkWidget *w, gpointer data)
{
    struct app_t *app = data;
    gchar *name = g_object_get_data(G_OBJECT(w), "scheme_name");
    scheme_change(app, name);
}

static gboolean lmenu_popup(GtkWidget *w, GdkEventButton *ev, gpointer data)
{
    struct app_t *app = data;
    
    if (ev->type != GDK_BUTTON_PRESS)
	return FALSE;
    if (ev->button != 1)
	return FALSE;
    
    gtk_menu_popup(GTK_MENU(app->menu),
	    NULL, NULL,
	    NULL, NULL,
	    ev->button, ev->time);
    return TRUE;
}

static void lmenu_setup(struct app_t *app, GList *schemes)
{
    GtkWidget *menu;
    
    menu = gtk_menu_new();
    
    for ( ; schemes != NULL; schemes = g_list_next(schemes)) {
	gchar *name = schemes->data;
	GtkWidget *item;
	
	item = gtk_menu_item_new_with_label(name);
	gtk_menu_append(GTK_MENU(menu), item);
	g_object_set_data(G_OBJECT(item), "scheme_name", g_strdup(name));
	g_signal_connect(G_OBJECT(item), "activate",
		G_CALLBACK(lmenu_cb), app);
	
	gtk_widget_show(item);
    }
    
    if (app->menu != NULL)
	gtk_widget_destroy(app->menu);
    
    app->menu = menu;
}

/**************** right-menu ****/

static const BonoboUIVerb rmenu_verbs [] = 
{
//    BONOBO_UI_VERB("PlanetloadAppletDetails", showinfo_cb),
    BONOBO_UI_VERB("PlanetloadAppletProperties", preferences),
//    BONOBO_UI_VERB("PlanetloadAppletHelp", help_cb),
    BONOBO_UI_VERB("PlanetloadAppletAbout", about),
    
    BONOBO_UI_VERB_END
};

static gchar *rmenu_make_verb(gchar *scheme)
{
    return g_strdup_printf("PlanetloadAppletScheme.%s", scheme);
}

#if 0
static void show_menu(int indent, BonoboUINode *node)
{
    int i;
    for (i = 0; i < indent; i++)
	debug_log("  ");
    debug_log("%s\n", bonobo_ui_node_get_name(node));
    
    for (node = bonobo_ui_node_children(node); node != NULL; node = bonobo_ui_node_next(node)) {
	show_menu(indent + 1, node);
    }
}
#endif

static gchar *rmenu_next_name(gchar *buf)
{
    static GStaticMutex mutex = G_STATIC_MUTEX_INIT;
    static int ctr = 0;
    
    g_static_mutex_lock(&mutex);
    sprintf(buf, "Change Scheme %d", ctr++);
    g_static_mutex_unlock(&mutex);
    
    return buf;
}

static void rmenu_setup(struct app_t *app, GList *schemes)
{
    BonoboUIComponent *uic = panel_applet_get_popup_component(app->applet);
    BonoboUINode *placeholder;
    gchar namebuf[128];
    
    debug_log("rmenu_setup:\n");
    
#if 0
    show_menu(0, bonobo_ui_component_get_tree(uic, "/", TRUE, NULL));
#endif
    
    placeholder = bonobo_ui_component_get_tree(uic, "/popups/popup/Schemes List", TRUE, NULL);
    if (placeholder == NULL) {
	debug_log("no placeholder\n");
	return;
    }
    
    debug_log("has placeholder\n");
    
    // 前のを削除。
    {
	BonoboUINode *node;
	
	bonobo_ui_component_freeze(uic, NULL);
	
	for (node = bonobo_ui_node_children(placeholder);
		node != NULL; node = bonobo_ui_node_next(node)) {
	    gchar path[1024];
	    sprintf(path, "/popups/popup/Schemes List/%s",
		    bonobo_ui_node_get_attr(node, "name"));
	    debug_log("rmenu_setup: remove %s\n", path);
	    bonobo_ui_component_rm(uic, path, NULL);
	}
	
	bonobo_ui_component_thaw(uic, NULL);
    }
    
    // 最新のを追加。
    if (app->use_planet) {
	gchar xmlbuf[1024];
	
	bonobo_ui_component_freeze(uic, NULL);
	
	for ( ; schemes != NULL; schemes = g_list_next(schemes)) {
	    gchar *p = schemes->data;
	    gchar *verb = rmenu_make_verb(p);
	    gchar *name;
	    
	    name = rmenu_next_name(namebuf);
	    sprintf(xmlbuf, "<menuitem name=\"%s\" verb=\"%s\" label=\"%s\"/>", name, verb, p);
	    
	    bonobo_ui_component_add_verb(uic, verb, scheme_cb, app);
	    debug_log("rmenu_setup: add %s\n", name);
	    bonobo_ui_component_set(uic, "/popups/popup/Schemes List", xmlbuf, NULL);
	    
	    g_free(verb);
	}
	
	sprintf(xmlbuf, "<separator name=\"%s\"/>", rmenu_next_name(namebuf));
	bonobo_ui_component_set(uic, "/popups/popup/Schemes List", xmlbuf, NULL);
	
	bonobo_ui_component_thaw(uic, NULL);
    }
    
#if 0
    show_menu(0, bonobo_ui_component_get_tree(uic, "/", TRUE, NULL));
#endif
    
    debug_log("rmenu_setup: done.\n");
}

void setup_schemes_menu(struct app_t *app)
{
    GList *list = NULL;
    
    list = scheme_get_list(app);
    
    rmenu_setup(app, list);
    lmenu_setup(app, list);
    lmenu_setup(app, list);
    
    // fixme: free list.
}

/**************** properties changed ****/

void disp_scheme_onoff(struct app_t *app)
{
    if (app->display_scheme && app->use_planet)
	gtk_widget_show(app->scheme_label_bar);
    else
	gtk_widget_hide(app->scheme_label_bar);
}

static void font_change_iter(GtkWidget *w, gpointer data)
{
    struct app_t *app = data;
    nsa_iface_set_font(NSA_IFACE(w), app->fontname);
}

void font_change(struct app_t *app)
{
    PangoFontDescription *pfd;
    pfd = pango_font_description_from_string(app->fontname);
    gtk_widget_modify_font(app->scheme_label_text, pfd);
    pango_font_description_free(pfd);
    
    gtk_container_foreach(GTK_CONTAINER(app->pack), font_change_iter, app);
}

static void painter_change_iter(GtkWidget *w, gpointer data)
{
    struct app_t *app = data;
    nsa_iface_set_painter(NSA_IFACE(w), app->painter);
}

void painter_change(struct app_t *app)
{
    gtk_container_foreach(GTK_CONTAINER(app->pack), painter_change_iter, app);
}

/**************** panel orient changed ****/

static gboolean get_applet_vert(PanelApplet *applet)
{
    switch (panel_applet_get_orient(applet)) {
    case PANEL_APPLET_ORIENT_UP:
    case PANEL_APPLET_ORIENT_DOWN:
	return FALSE;
	break;
    default:
	return TRUE;
	break;
    }
}

static void reparent_iter(GtkWidget *w, gpointer data)
{
    GtkWidget *new_parent = data;
    
    gtk_widget_reparent(w, new_parent);
}

static void setvert_iter(GtkWidget *w, gpointer data)
{
    gint is_vert = GPOINTER_TO_INT(data);
    
    nsa_iface_set_vert(NSA_IFACE(w), is_vert);
}

static void change_orient_cb(PanelApplet *applet, gint arg1, gpointer closure)
{
    struct app_t *app = closure;
    int is_vert;
    
    is_vert = get_applet_vert(applet);
    
    if (is_vert != app->is_vert) {
	GtkWidget *from, *to, *parent;
	
	app->is_vert = is_vert;
	
	from = app->pack;
	if (is_vert)
	    to = gtk_vbox_new(FALSE, 0);
	else
	    to = gtk_hbox_new(FALSE, 0);
	parent = gtk_widget_get_parent(from);
	
	gtk_container_foreach(GTK_CONTAINER(from), reparent_iter, to);
	gtk_container_foreach(GTK_CONTAINER(to), setvert_iter, GINT_TO_POINTER(is_vert));
	
	app->pack = to;
	
	gtk_box_pack_start(GTK_BOX(parent), to, TRUE, TRUE, 0);
	gtk_widget_destroy(from);
	gtk_widget_show_all(to);
    }
}

/**************** panel size changed ****/

static void change_size_iter(GtkWidget *widget, gpointer data)
{
    (void) data;
    nsa_iface_panel_size_changed(NSA_IFACE(widget));
}

static void change_size_cb(GtkWidget *w, gint size, gpointer closure)
{
    struct app_t *app = closure;
    
    gtk_container_foreach(GTK_CONTAINER(app->pack), change_size_iter, NULL);
}

/**************** update by timer ****/

static void update_iter(GtkWidget *widget, gpointer data)
{
    (void) data;
    nsa_iface_update(NSA_IFACE(widget));
}

static gboolean update(gpointer data)
{
    struct app_t *app = data;
    gchar buf[128];
    
    gtk_container_foreach(GTK_CONTAINER(app->pack), update_iter, NULL);
    
    gtk_label_set_label(GTK_LABEL(app->scheme_label_text), scheme_get_current(app, buf));
    
    return TRUE;
}

void timer_set(struct app_t *app)
{
    if (app->timeout_id != 0)
	gtk_timeout_remove(app->timeout_id);
    app->timeout_id = gtk_timeout_add(app->interval, update, app);
}

/**************** main ****/

static void destroy_cb(PanelApplet *applet, gpointer data)
{
    struct app_t *app = data;
    
    debug_log("destroy_cb:\n");
    
    if (app->timeout_id != 0)
	gtk_timeout_remove(app->timeout_id);
    
    preferences_destroy(app);
    
//    g_free(app);
}

static gboolean planetload_applet_start(
	PanelApplet *applet, const gchar *iid, gpointer data)
{
    struct app_t *app;
    GtkWidget *vbox;
    
    debug_init();
    
    if (strcmp (iid, "OAFIID:PlanetloadApplet") != 0)
	return FALSE;
    
    app = g_new(struct app_t, 1);
    memset(app, 0, sizeof *app);
    
    app->use_planet = TRUE;
    
    app->applet = applet;
    
    panel_applet_add_preferences (applet, "/schemas/apps/planetload_applet/prefs", NULL);
    panel_applet_set_flags(applet, PANEL_APPLET_EXPAND_MINOR);
    
    g_signal_connect(G_OBJECT(applet), "destroy",
	    G_CALLBACK(destroy_cb), app);
    g_signal_connect(G_OBJECT(applet), "change_size",
	    G_CALLBACK(change_size_cb), app);
    g_signal_connect(G_OBJECT(applet), "change_orient",
	    G_CALLBACK(change_orient_cb), app);
    
    app->is_vert = get_applet_vert(applet);
    
    app->tooltips = gtk_tooltips_new();
    
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(applet), vbox);
    
    if (!app->is_vert)
	app->pack = gtk_hbox_new(FALSE, 0);
    else
	app->pack = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app->pack, TRUE, TRUE, 0);
    
    if (!appconf_load(app)) {
	appconf_init(app);
	appconf_save(app);
    }
    
    panel_applet_setup_menu_from_file(applet, NULL, "PlanetloadApplet.xml", NULL, rmenu_verbs, app);
    setup_schemes_menu(app);
    
    app->scheme_label_bar = gtk_event_box_new();
    gtk_box_pack_end(GTK_BOX(vbox), app->scheme_label_bar, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(app->scheme_label_bar), "button-press-event",
	    G_CALLBACK(lmenu_popup), app);
    
    {
	gchar buf[128];
	app->scheme_label_text = gtk_label_new(scheme_get_current(app, buf));
	gtk_container_add(GTK_CONTAINER(app->scheme_label_bar), app->scheme_label_text);
    }
    
    {
	PangoFontDescription *pfd;
	pfd = pango_font_description_from_string(app->fontname);
	gtk_widget_modify_font(app->scheme_label_text, pfd);
	pango_font_description_free(pfd);
    }
    
    gtk_widget_show_all(GTK_WIDGET(applet));
    
    disp_scheme_onoff(app);
    
    timer_set(app);
    
    return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY(
	"OAFIID:PlanetloadApplet_Factory",
	PANEL_TYPE_APPLET,
	PACKAGE,
	VERSION,
	planetload_applet_start,
	NULL);
