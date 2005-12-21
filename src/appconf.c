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
 * $Id: appconf.c 44 2005-12-21 14:25:23Z masm $
 */

#include "../config.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "app.h"
#include "appconf.h"
#include "i18n-support.h"

static struct color_t default_color[] = {
    { 0xffff, 0xaaaa, 0xaaaa, 0x0000, },
    { 0xffff, 0x0000, 0x0000, 0x0000, },
    { 0xffff, 0x4848, 0x9a9a, 0x8383, },
    { 0x0000, 0x0000, 0x0000, 0x0000, },
    { 0xffff, 0x4848, 0x9a9a, 0x8383, },
    { 0xffff, 0x4848, 0x9a9a, 0x8383, },
    { 0xffff, 0x4848, 0x9a9a, 0x8383, },
    { 0xffff, 0x0000, 0x0000, 0x0000, },
    { 0xffff, 0x4848, 0x9a9a, 0x8383, },
};

struct appconf_save_work_t {
    struct app_t *app;
    
    int idx;
};

static gboolean str_to_col(gchar *str, struct color_t *col);
static gboolean col_to_str(const struct color_t *col, gchar *buf);

static void appconf_save_iter(GtkWidget *widget, gpointer data)
{
    struct appconf_save_work_t *w = data;
    PanelApplet *applet = w->app->applet;
    NsaIface *iface = NSA_IFACE(widget);
    char key[32], buf[32];
    int i;
    
    sprintf(key, "if%d.size", w->idx);
    panel_applet_gconf_set_int(applet, key, nsa_iface_get_size(iface), NULL);
    
    sprintf(key, "if%d.name", w->idx);
    panel_applet_gconf_set_string(applet, key, nsa_iface_get_name(iface), NULL);
    
    sprintf(key, "if%d.is_ppp", w->idx);
    panel_applet_gconf_set_bool(applet, key, nsa_iface_get_is_ppp(iface), NULL);
    
    sprintf(key, "if%d.lock_file", w->idx);
    panel_applet_gconf_set_string(applet, key, nsa_iface_get_lock_file(iface), NULL);
    
    for (i = 0; i < COL_NR; i++) {
	sprintf(key, "if%d.color%d", w->idx, i);
	col_to_str(nsa_iface_get_color(iface, i), buf);
	panel_applet_gconf_set_string(applet, key, buf, NULL);
    }
    
    sprintf(key, "if%d.cmd_up", w->idx);
    panel_applet_gconf_set_string(applet, key, nsa_iface_get_cmd_up(iface), NULL);
    
    sprintf(key, "if%d.cmd_down", w->idx);
    panel_applet_gconf_set_string(applet, key, nsa_iface_get_cmd_down(iface), NULL);
    
    w->idx++;
}

void appconf_save(struct app_t *app)
{
    PanelApplet *applet = app->applet;
    struct appconf_save_work_t work, *w = &work;
    
    memset(w, 0, sizeof *w);
    w->app = app;
    w->idx = 0;
    
    gtk_container_foreach(GTK_CONTAINER(app->pack), appconf_save_iter, w);
    
    panel_applet_gconf_set_int(applet, "nifaces", w->idx, NULL);
    
    panel_applet_gconf_set_int(applet, "painter", app->painter, NULL);
    panel_applet_gconf_set_string(applet, "font", app->fontname, NULL);
    panel_applet_gconf_set_string(applet, "cmd_list", app->cmd_to_get_scheme_list, NULL);
    panel_applet_gconf_set_string(applet, "cmd_chg", app->cmd_to_change_scheme, NULL);
    panel_applet_gconf_set_int(applet, "interval", app->interval, NULL);
    panel_applet_gconf_set_bool(applet, "disp_scheme", app->display_scheme, NULL);
    panel_applet_gconf_set_bool(applet, "use_planet", app->use_planet, NULL);
}


struct appconf_load_work_t {
    struct app_t *app;
    
    int idx;
};

gboolean appconf_load(struct app_t *app)
{
    PanelApplet *applet = app->applet;
    int idx, num;
    
    app->fontname = panel_applet_gconf_get_string(applet, "font", NULL);
    if (app->fontname == NULL) {
	return FALSE;
    }
    
    app->cmd_to_get_scheme_list = panel_applet_gconf_get_string(applet, "cmd_list", NULL);
    if (app->cmd_to_get_scheme_list == NULL)
	return FALSE;
    
    app->cmd_to_change_scheme = panel_applet_gconf_get_string(applet, "cmd_chg", NULL);
    if (app->cmd_to_change_scheme == NULL)
	return FALSE;
    
    app->interval = panel_applet_gconf_get_int(applet, "interval", NULL);
    if (app->interval <= 0)
	return FALSE;
    
    app->display_scheme = panel_applet_gconf_get_bool(applet, "disp_scheme", NULL);
    
    app->use_planet = panel_applet_gconf_get_bool(applet, "use_planet", NULL);
    
    num = panel_applet_gconf_get_int(applet, "nifaces", NULL);
    if (num <= 0)
	return FALSE;
    
    app->painter = panel_applet_gconf_get_int(applet, "painter", NULL);
    
    for (idx = 0; idx < num; idx++) {
	GtkWidget *w;
	NsaIface *iface;
	char key[32];
	int ci;
	int n;
	char *s;
	
	w = nsa_iface_new(app->is_vert);
	iface = NSA_IFACE(w);
	
	sprintf(key, "if%d.size", idx);
	n = panel_applet_gconf_get_int(applet, key, NULL);
	nsa_iface_set_size(iface, n);
	
	sprintf(key, "if%d.name", idx);
	s = panel_applet_gconf_get_string(applet, key, NULL);
	nsa_iface_set_name(iface, s ? s : "eth0");
	if (s != NULL)
	    g_free(s);
	
	sprintf(key, "if%d.is_ppp", idx);
	nsa_iface_set_is_ppp(iface, panel_applet_gconf_get_bool(applet, key, NULL));
	
	sprintf(key, "if%d.lock_file", idx);
	s = panel_applet_gconf_get_string(applet, key, NULL);
	nsa_iface_set_lock_file(iface, s ? s : "");
	if (s != NULL)
	    g_free(s);
	
	nsa_iface_set_font(iface, app->fontname);
	nsa_iface_set_painter(iface, app->painter);
	
	for (ci = 0; ci < COL_NR; ci++) {
	    struct color_t color;
	    sprintf(key, "if%d.color%d", idx, ci);
	    s = panel_applet_gconf_get_string(applet, key, NULL);
	    str_to_col(s ? s : "0:0:0:0", &color);
	    nsa_iface_set_color(iface, ci, &color);
	    if (s != NULL)
		g_free(s);
	}
	
	sprintf(key, "if%d.cmd_up", idx);
	s = panel_applet_gconf_get_string(applet, key, NULL);
	nsa_iface_set_cmd_up(iface, s ? s : "");
	if (s != NULL)
	    g_free(s);
	
	sprintf(key, "if%d.cmd_down", idx);
	s = panel_applet_gconf_get_string(applet, key, NULL);
	nsa_iface_set_cmd_down(iface, s ? s : "");
	if (s != NULL)
	    g_free(s);
	
	nsa_iface_set_tooltips(iface, app->tooltips);
	
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(app->pack), w, FALSE, FALSE, 0);
    }
    
    return TRUE;
}

void appconf_init(struct app_t *app)
{
//    PanelApplet *applet = app->applet;
    GtkWidget *w;
    
    app->fontname = g_strdup("sans 8");
    app->cmd_to_get_scheme_list = g_strdup("/sbin/netscheme -ql");
    app->cmd_to_change_scheme = g_strdup("/sbin/netscheme %s");
    app->interval = 1000;
    app->display_scheme = TRUE;
    app->use_planet = TRUE;
    app->painter = NSA_PAINTER_TYPE_FINE;
    
    w = nsa_iface_new_with_params(
	    "eth0",
	    FALSE,
	    "",
	    default_color,
	    "",		// cmd_up
	    "",		// cmd_down
	    40,
	    app->is_vert,
	    app->painter,
	    app->fontname,
	    app->tooltips);
    
    gtk_widget_show(w);
    gtk_box_pack_start(GTK_BOX(app->pack), w, FALSE, FALSE, 0);
}

static gboolean str_to_col(gchar *str, struct color_t *col)
{
    if (sscanf(str, "%hx:%hx:%hx:%hx",
		&col->a, &col->r, &col->g, &col->b) != 4)
	return FALSE;
    return TRUE;
}

static gboolean col_to_str(const struct color_t *col, gchar *buf)
{
    sprintf(buf, "%04x:%04x:%04x:%04x", col->a, col->r, col->g, col->b);
    return TRUE;
}


/*EOF*/
