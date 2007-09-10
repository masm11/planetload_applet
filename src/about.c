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
#include <unistd.h>
#include <sys/types.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#ifdef HAVE_GNOME
#include <libgnomeui/gnome-about.h>
#endif
#ifdef HAVE_XFCE4
#include <libxfcegui4/xfce_aboutdialog.h>
#endif

#include "app.h"
#include "about.h"
#include "i18n-support.h"

#define COPYRIGHT_YEAR		"2003-2007"
#define COPYRIGHT_HOLDER	"Yuuki Harano"

static struct {
    const gchar *name;
    const gchar *mail;
} authors[] = {
    { "Yuuki Harano",		"masm@flowernet.gr.jp", },
    { "Shun-ichi TAHARA",	"jado@flowernet.gr.jp", },
};
#define NAUTHORS (sizeof authors / sizeof authors[0])

#ifdef HAVE_GNOME
void about(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
    static GtkWidget *w = NULL;
    struct app_t *app = data;
    
    if (w != NULL) {
	gtk_window_set_screen(GTK_WINDOW(w),
		gtk_widget_get_screen(GTK_WIDGET(app->applet)));
	gtk_window_present(GTK_WINDOW(w));
	return;
    }
    
    int i;
    gchar **author_list = g_new(gchar *, NAUTHORS + 1);
    for (i = 0; i < NAUTHORS; i++) {
	author_list[i] = g_strdup_printf("%s <%s>",
		authors[i].name, authors[i].mail);
    }
    author_list[i] = NULL;
    
    w = gnome_about_new(
	    "Planetload Applet", VERSION,
	    "(C) " COPYRIGHT_YEAR " " COPYRIGHT_HOLDER,
	    _("Released under the GNU General Public License.\n\n"
	    "A network traffic monitor, cooperative with Planet."),
	    (const gchar **) author_list,
	    NULL,
	    NULL,
	    NULL);
    
    g_strfreev(author_list);
    author_list = NULL;
    
    gtk_window_set_screen(GTK_WINDOW(w),
	    gtk_widget_get_screen(GTK_WIDGET(app->applet)));
    gtk_window_set_wmclass(GTK_WINDOW(w), "planetloadApplet", "PlanetloadApplet");
    g_signal_connect(G_OBJECT(w), "destroy",
	    G_CALLBACK(gtk_widget_destroyed), &w);
    
    gtk_widget_show(w);
}
#endif

#ifdef HAVE_XFCE4
void about(XfcePanelPlugin *plugin, gpointer data)
{
    XfceAboutInfo *info;
    GtkWidget *w;
    struct app_t *app = data;
    int i;
    
    info = xfce_about_info_new(
	    "Planetload Applet", VERSION,
	    "A network traffic monitor, cooperative with Planet.",
	    XFCE_COPYRIGHT_TEXT(COPYRIGHT_YEAR, COPYRIGHT_HOLDER),
	    XFCE_LICENSE_GPL);
    for (i = 0; i < NAUTHORS; i++)
	xfce_about_info_add_credit(info, authors[i].name, authors[i].mail, NULL);
    
    w = xfce_about_dialog_new_with_values(NULL, info, NULL);
    gtk_window_set_screen(GTK_WINDOW(w),
	    gtk_widget_get_screen(GTK_WIDGET(app->applet)));
    gtk_window_set_wmclass(GTK_WINDOW(w), "planetloadApplet", "PlanetloadApplet");
    
    gtk_dialog_run(GTK_DIALOG(w));
    
    gtk_widget_destroy(w);
    
    xfce_about_info_free(info);
}
#endif
