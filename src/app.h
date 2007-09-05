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
 * $Id: app.h 24 2005-07-18 08:12:27Z masm $
 */

#ifndef APP_H__INCLUDED
#define APP_H__INCLUDED

#include "../config.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#ifdef HAVE_GNOME
#include <panel-applet.h>
#endif
#ifdef HAVE_XFCE4
#include <libxfce4panel/xfce-panel-plugin.h>
#endif

#include "nsaiface.h"

struct app_t {
#ifdef HAVE_GNOME
    PanelApplet *applet;
#endif
#ifdef HAVE_XFCE4
    XfcePanelPlugin *applet;
#endif
    GtkWidget *pack;
    GtkWidget *scheme_label_bar, *scheme_label_text, *menu;
    GtkTooltips *tooltips;
    
    gint timeout_id;
    gint interval;
    
    gint painter;
    
    struct iface_t *ifaces;
    
    gchar *fontname;
    
    gboolean use_planet;
    
    gchar *cmd_to_get_scheme_list;
    gchar *cmd_to_change_scheme;
    
    gboolean display_scheme;
    
    gboolean is_vert;
    
    struct dialog_t *dialog;
};

#endif	/* ifndef APP_H__INCLUDED */
