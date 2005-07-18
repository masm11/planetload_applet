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
 * $Id: color.c 21 2005-07-18 07:57:00Z masm $
 */

#include "color.h"

struct hsvcolor_t {
    gshort h;
    gushort s;
    gushort v;
};

static void rgb2hsv(struct hsvcolor_t *hsv, struct color_t *rgb);
static void hsv2rgb(struct color_t *rgb, struct hsvcolor_t *hsv);

void mix_color(struct color_t *c,
	struct color_t *c1, struct color_t *c2, int64_t v1, int64_t v2)
{
    double r1, r2;
    struct hsvcolor_t hc, hc1, hc2;
    
    r1 = v1 / (double) (v1 + v2);
    r2 = v2 / (double) (v1 + v2);
    
    rgb2hsv(&hc1, c1);
    rgb2hsv(&hc2, c2);
    
    if (hc1.s == 0 || hc2.s == 0 || hc1.h - hc2.h == 1800 || hc2.h - hc1.h == 1800) {
	c->r = c1->r * r1 + c2->r * r2;
	c->g = c1->g * r1 + c2->g * r2;
	c->b = c1->b * r1 + c2->b * r2;
    } else {
	if (hc1.h - hc2.h > 1800)
	    hc2.h += 3600;
	else if (hc2.h - hc1.h > 1800)
	    hc1.h += 3600;
	hc.h = hc1.h * r1 + hc2.h * r2;
	if (hc.h >= 3600)
	    hc.h -= 3600;
	hc.s = hc1.s * r1 + hc2.s * r2;
	hc.v = hc1.v * r1 + hc2.v * r2;
	hsv2rgb(c, &hc);
    }
    c->a = c1->a * r1 + c2->a * r2;
}

static void rgb2hsv(struct hsvcolor_t *hsv, struct color_t *rgb)
{
    gushort max, min, d;
    
    max = MAX(MAX(rgb->r, rgb->g), rgb->b);
    min = MIN(MIN(rgb->r, rgb->g), rgb->b);
    d = max - min;
    
    hsv->v = max;
    if (d > 0)
	hsv->s = (guint)(d * 0xffff) / max;
    else
	hsv->s = 0;
    
    if (hsv->s == 0)
	hsv->h = 0;
    else {
	if (rgb->r == max)
	    hsv->h = (rgb->g - rgb->b) * 600 / d;
	else if (rgb->g == max)
	    hsv->h = 1200 + (rgb->b - rgb->r) * 600 / d;
	else
	    hsv->h = 2400 + (rgb->r - rgb->g) * 600 / d;
	
	if (hsv->h < 0)
	    hsv->h += 3600;
    }
}

static void hsv2rgb(struct color_t *rgb, struct hsvcolor_t *hsv)
{
    gushort f, t1, t2, t3;
    
    rgb->r = rgb->g = rgb->b = hsv->v;
    if (hsv->s > 0) {
	f = hsv->h % 600;
	t1 = (guint)(hsv->v * hsv->s) / 0xffff;
	t2 = t1 * f / 600;
	t3 = t1 * (600 - f) / 600;
	
	switch (hsv->h / 600) {
	case 0:
	    rgb->g -= t3;
	    rgb->b -= t1;
	    break;
	case 1:
	    rgb->b -= t1;
	    rgb->r -= t2;
	    break;
	case 2:
	    rgb->b -= t3;
	    rgb->r -= t1;
	    break;
	case 3:
	    rgb->r -= t1;
	    rgb->g -= t2;
	    break;
	case 4:
	    rgb->r -= t3;
	    rgb->g -= t1;
	    break;
	case 5:
	default:
	    rgb->g -= t1;
	    rgb->b -= t2;
	    break;
	}
    }
}
