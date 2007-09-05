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
#ifdef HAVE_GNOME
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#endif

#include "app.h"
#include "appconf.h"
#include "i18n-support.h"

static struct color_t default_color[] = {
    //   A       R       G       B
    { 0xffff, 0x0000, 0xffff, 0x0000, },// graph      in
    { 0xffff, 0x0000, 0x0000, 0x0000, },// bg    up
    { 0xffff, 0xffff, 0xffff, 0xffff, },// text  up
    { 0x0000, 0x0000, 0x0000, 0x0000, },// bg    down
    { 0xffff, 0xffff, 0xffff, 0xffff, },// line
    { 0xffff, 0x0000, 0x0000, 0x0000, },// text  down
    { 0xffff, 0xffff, 0x0000, 0x0000, },// graph      out
    { 0x0000, 0x0000, 0x0000, 0x0000, },// bg    conn
    { 0xffff, 0x0000, 0x0000, 0x0000, },// text  conn
};

struct appconf_save_work_t {
#ifdef HAVE_GNOME
    struct app_t *app;
#endif
#ifdef HAVE_XFCE4
    XfceRc *rc;
#endif
    
    int idx;
};

static gboolean str_to_col(const gchar *str, struct color_t *col);
static gboolean col_to_str(const struct color_t *col, gchar *buf);

static void appconf_save_iter(GtkWidget *widget, gpointer data)
{
#ifdef HAVE_GNOME
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
#endif
#ifdef HAVE_XFCE4
    struct appconf_save_work_t *w = data;
    XfceRc *rc = w->rc;
    NsaIface *iface = NSA_IFACE(widget);
    char key[32], buf[32];
    int i;
    
    sprintf(key, "if%d.size", w->idx);
    xfce_rc_write_int_entry(rc, key, nsa_iface_get_size(iface));
    
    sprintf(key, "if%d.name", w->idx);
    xfce_rc_write_entry(rc, key, nsa_iface_get_name(iface));
    
    sprintf(key, "if%d.is_ppp", w->idx);
    xfce_rc_write_bool_entry(rc, key, nsa_iface_get_is_ppp(iface));
    
    sprintf(key, "if%d.lock_file", w->idx);
    xfce_rc_write_entry(rc, key, nsa_iface_get_lock_file(iface));
    
    for (i = 0; i < COL_NR; i++) {
	sprintf(key, "if%d.color%d", w->idx, i);
	col_to_str(nsa_iface_get_color(iface, i), buf);
	xfce_rc_write_entry(rc, key, buf);
    }
    
    sprintf(key, "if%d.cmd_up", w->idx);
    xfce_rc_write_entry(rc, key, nsa_iface_get_cmd_up(iface));
    
    sprintf(key, "if%d.cmd_down", w->idx);
    xfce_rc_write_entry(rc, key, nsa_iface_get_cmd_down(iface));
    
    w->idx++;
#endif
}

void appconf_save(struct app_t *app)
{
#ifdef HAVE_GNOME
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
#endif
#ifdef HAVE_XFCE4
    XfceRc *rc;
    
    {
	gchar *file = xfce_panel_plugin_save_location(app->applet, TRUE);
	if (file == NULL)
	    return;

char buf[1024];
sprintf(buf, "echo %s >> /tmp/save.log", file);
system(buf);

	rc = xfce_rc_simple_open(file, FALSE);
	g_free(file);
    }
    
    struct appconf_save_work_t work, *w = &work;
    
    memset(w, 0, sizeof *w);
    w->rc = rc;
    w->idx = 0;
    
    gtk_container_foreach(GTK_CONTAINER(app->pack), appconf_save_iter, w);
    
    xfce_rc_write_int_entry(rc, "nifaces", w->idx);
    
    xfce_rc_write_int_entry(rc, "painter", app->painter);
    xfce_rc_write_entry(rc, "font", app->fontname);
    xfce_rc_write_entry(rc, "cmd_list", app->cmd_to_get_scheme_list);
    xfce_rc_write_entry(rc, "cmd_chg", app->cmd_to_change_scheme);
    xfce_rc_write_int_entry(rc, "interval", app->interval);
    xfce_rc_write_bool_entry(rc, "disp_scheme", app->display_scheme);
    xfce_rc_write_bool_entry(rc, "use_planet", app->use_planet);
    
    xfce_rc_close(rc);
#endif
}

gboolean appconf_load(struct app_t *app)
{
#ifdef HAVE_GNOME
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
#endif
    
#ifdef HAVE_XFCE4
    XfceRc *rc;
    int idx, num;
    
    {
	gchar *file = xfce_panel_plugin_save_location(app->applet, FALSE);
	if (file == NULL)
	    return FALSE;
	rc = xfce_rc_simple_open(file, TRUE);
	g_free(file);
    }
    
    app->fontname = g_strdup(xfce_rc_read_entry(rc, "font", NULL));
    if (app->fontname == NULL)
	return FALSE;
    
    app->cmd_to_get_scheme_list = g_strdup(xfce_rc_read_entry(rc, "cmd_list", NULL));
    if (app->cmd_to_get_scheme_list == NULL)
	return FALSE;
    
    app->cmd_to_change_scheme = g_strdup(xfce_rc_read_entry(rc, "cmd_chg", NULL));
    if (app->cmd_to_change_scheme == NULL)
	return FALSE;
    
    app->interval = xfce_rc_read_int_entry(rc, "interval", 0);
    if (app->interval <= 0)
	return FALSE;
    
    app->display_scheme = xfce_rc_read_bool_entry(rc, "disp_scheme", TRUE);
    
    app->use_planet = xfce_rc_read_bool_entry(rc, "use_planet", TRUE);
    
    num = xfce_rc_read_int_entry(rc, "nifaces", 0);
    if (num <= 0)
	return FALSE;
    
    app->painter = xfce_rc_read_int_entry(rc, "painter", NSA_PAINTER_TYPE_FINE);
    
    for (idx = 0; idx < num; idx++) {
	GtkWidget *w;
	NsaIface *iface;
	char key[32];
	int ci;
	int n;
	
	w = nsa_iface_new(app->is_vert);
	iface = NSA_IFACE(w);
	
	sprintf(key, "if%d.size", idx);
	n = xfce_rc_read_int_entry(rc, key, 0);
	nsa_iface_set_size(iface, n);
	
	sprintf(key, "if%d.name", idx);
	nsa_iface_set_name(iface, xfce_rc_read_entry(rc, key, "eth0"));
	
	sprintf(key, "if%d.is_ppp", idx);
	nsa_iface_set_is_ppp(iface, xfce_rc_read_bool_entry(rc, key, FALSE));
	
	sprintf(key, "if%d.lock_file", idx);
	nsa_iface_set_lock_file(iface, xfce_rc_read_entry(rc, key, ""));
	
	nsa_iface_set_font(iface, app->fontname);
	nsa_iface_set_painter(iface, app->painter);
	
	for (ci = 0; ci < COL_NR; ci++) {
	    struct color_t color;
	    sprintf(key, "if%d.color%d", idx, ci);
	    str_to_col(xfce_rc_read_entry(rc, key, "0:0:0:0"), &color);
	    nsa_iface_set_color(iface, ci, &color);
	}
	
	sprintf(key, "if%d.cmd_up", idx);
	nsa_iface_set_cmd_up(iface, xfce_rc_read_entry(rc, key, ""));
	
	sprintf(key, "if%d.cmd_down", idx);
	nsa_iface_set_cmd_down(iface, xfce_rc_read_entry(rc, key, ""));
	
	nsa_iface_set_tooltips(iface, app->tooltips);
	
	gtk_widget_show(w);
	gtk_box_pack_start(GTK_BOX(app->pack), w, FALSE, FALSE, 0);
    }
    
    xfce_rc_close(rc);
    
    return TRUE;
#endif
    return FALSE;
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

static gboolean str_to_col(const gchar *str, struct color_t *col)
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
