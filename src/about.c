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
 * $Id: about.c 29 2005-07-18 09:04:38Z masm $
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

static const gchar *authors[] = {
    "Yuuki Harano <masm@flowernet.gr.jp>",
    "Shun-ichi TAHARA <jado@flowernet.gr.jp>",
    NULL
};

static const gchar *documenters[] = {
    NULL
};

static const gchar *translator_credits = NULL;

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
    
    w = gnome_about_new(
	    "Planetload Applet", VERSION,
	    "(C) 2003-2005 Yuuki Harano",
	    _("Released under the GNU General Public License.\n\n"
	    "A network traffic monitor, cooperative with Planet."),
	    authors,
	    documenters,
	    translator_credits,
	    NULL);
    
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
    
    info = xfce_about_info_new(
	    "Planetload Applet", VERSION,
	    "A network traffic monitor, cooperative with Planet.",
	    XFCE_COPYRIGHT_TEXT("2003-2007", "Yuuki Harano"),
	    XFCE_LICENSE_GPL);
    xfce_about_info_add_credit(info, "Yuuki Harano", "masm@flowernet.gr.jp", NULL);
    
    w = xfce_about_dialog_new_with_values(NULL, info, NULL);
    gtk_window_set_screen(GTK_WINDOW(w),
	    gtk_widget_get_screen(GTK_WIDGET(app->applet)));
    gtk_window_set_wmclass(GTK_WINDOW(w), "planetloadApplet", "PlanetloadApplet");
    
    gtk_dialog_run(GTK_DIALOG(w));
    
    gtk_widget_destroy(w);
    
    xfce_about_info_free(info);
}
#endif
