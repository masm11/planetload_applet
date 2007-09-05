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
 * $Id: color.h 24 2005-07-18 08:12:27Z masm $
 */

#ifndef COLOR_H__INCLUDED
#define COLOR_H__INCLUDED

#include "../config.h"

#include <sys/types.h>
#include <glib.h>

struct color_t {
    guint16 a, r, g, b;
};

void mix_color(struct color_t *c,
	struct color_t *c1, struct color_t *c2, int64_t v1, int64_t v2);

#endif	/* ifndef COLOR_H__INCLUDED */
