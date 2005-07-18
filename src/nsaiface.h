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
 * $Id: nsaiface.h 21 2005-07-18 07:57:00Z masm $
 */

#ifndef NSA_IFACE_H
#define NSA_IFACE_H

#include <gdk/gdk.h>
#include <gtk/gtktooltips.h>
#include <sys/types.h>
#include <pango/pango-layout.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#include "ifstat.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NSA_TYPE_IFACE                  (nsa_iface_get_type ())
#define NSA_IFACE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), NSA_TYPE_IFACE, NsaIface))
#define NSA_IFACE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), NSA_TYPE_IFACE, NsaIfaceClass))
#define NSA_IS_IFACE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NSA_TYPE_IFACE))
#define NSA_IS_IFACE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), NSA_TYPE_IFACE))
#define NSA_IFACE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), NSA_TYPE_IFACE, NsaIfaceClass))


typedef struct _NsaIface       NsaIface;
typedef struct _NsaIfaceClass  NsaIfaceClass;

enum {
    COL_FG,		// グラフ色 (in)
    COL_BG,		// 背景色 (up)
    COL_TEXT,		// 文字色 (up)
    COL_DOWN,		// 背景色 (down)
    COL_HBAR,		// 横線色
    COL_TEXT_DOWN,	// 文字色 (down)
    COL_FG2,		// グラフ色 (out)
    COL_BG_NOADDR,	// 背景色 (noaddr)
    COL_TEXT_NOADDR,	// 文字色 (noaddr)
    COL_NR
};

#include "nsapainter.h"
#include "color.h"
#include "ifstat.h"

typedef int64_t bps_t;

struct val_t {
    enum ifstat_stat_t stat;
    bps_t value_in, value_out;
};

struct _NsaIface
{
    GtkWidget widget;
    
    NsaPainter *painter;
    gint painter_id;
    
    gchar *name;
    
    struct ifstat_t *last_ifstat;
    GTimeVal last_time;
    
    gboolean vert;
    gint size;
    
    GtkTooltips *tooltips;
    
    GList *history;
    int nhistory;
    
    int mag, mag_in, mag_out;
    
    struct color_t color[COL_NR];
    
    gboolean is_ppp;
    gchar *lock_file;
    
    gchar *cmd_up, *cmd_down;
    
    gchar *fontname;
    
    gboolean repaint_doing;
};

struct _NsaIfaceClass
{
    GtkWidgetClass parent_class;
};


GType      nsa_iface_get_type     (void) G_GNUC_CONST;

GtkWidget *nsa_iface_new		(gboolean vert);
GtkWidget *nsa_iface_new_with_params	(const gchar *name,
					 gboolean is_ppp,
					 const gchar *lock_file,
					 const struct color_t *colors,
					 const gchar *cmd_up,
					 const gchar *cmd_down,
					 gint size,
					 gboolean vert,
					 gint painter,
					 const gchar *fontname,
					 GtkTooltips *tooltips);
GtkWidget *nsa_iface_new_from_iface	(NsaIface *from_iface);

void       nsa_iface_update		(NsaIface *iface);
void       nsa_iface_panel_size_changed	(NsaIface *iface);

void       nsa_iface_set_tooltips	(NsaIface *iface,
					 GtkTooltips *tooltips);
void       nsa_iface_set_font		(NsaIface *iface,
					 const gchar *fontname);
void       nsa_iface_set_vert		(NsaIface *iface,
					 gboolean vert);
void       nsa_iface_set_painter	(NsaIface *iface,
					 gint painter);

void       nsa_iface_set_size		(NsaIface *iface,
					 gint size);
void       nsa_iface_set_name		(NsaIface *iface,
					 const gchar *name);
void       nsa_iface_set_is_ppp		(NsaIface *iface,
					 gboolean is_ppp);
void       nsa_iface_set_lock_file	(NsaIface *iface,
					 const gchar *lock_file);
void       nsa_iface_set_color		(NsaIface *iface,
					 int part,
					 const struct color_t *color);
void       nsa_iface_set_cmd_up		(NsaIface *iface,
					 const gchar *cmd_up);
void       nsa_iface_set_cmd_down	(NsaIface *iface,
					 const gchar *cmd_down);

gint       nsa_iface_get_size		(NsaIface *iface);
const gchar *nsa_iface_get_name		(NsaIface *iface);
gboolean   nsa_iface_get_is_ppp		(NsaIface *iface);
const gchar *nsa_iface_get_lock_file	(NsaIface *iface);
const struct color_t *nsa_iface_get_color (NsaIface *iface,
					   int part);
const gchar *nsa_iface_get_cmd_up	(NsaIface *iface);
const gchar *nsa_iface_get_cmd_down	(NsaIface *iface);

int nsa_iface_get_width			(NsaIface *iface);
int nsa_iface_get_height		(NsaIface *iface);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NSA_IFACE_H */
