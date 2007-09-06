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

#ifndef PLANETLOAD_APPLET_H__INCLUDED
#define PLANETLOAD_APPLET_H__INCLUDED

#include "../config.h"

void font_change(struct app_t *app);
void timer_set(struct app_t *app);
void painter_change(struct app_t *app);
void disp_scheme_onoff(struct app_t *app);
void setup_schemes_menu(struct app_t *app);

#endif	/* ifndef PLANETLOAD_APPLET_H__INCLUDED */
