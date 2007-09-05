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
 * $Id: about.h 24 2005-07-18 08:12:27Z masm $
 */

#ifndef ABOUT_H__INCLUDED
#define ABOUT_H__INCLUDED

#include "../config.h"

#ifdef HAVE_GNOME
void about(BonoboUIComponent *uic, gpointer data, const gchar *verbname);
#endif
#ifdef HAVE_XFCE4
void about(XfcePanelPlugin *plugin, gpointer data);
#endif

#endif	/* ifndef ABOUT_H__INCLUDED */
