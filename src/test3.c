#include "../config.h"

#include <string.h>
#include <math.h>
#include <glib.h>
#include "i18n-support.h"

struct color_t {
    gushort a, r, g, b;
};

struct hsvcolor_t {
    int h;
    int s;
    int v;
};

static void rgb2hsv(struct hsvcolor_t *hsv, struct color_t *rgb)
{
    gushort max, min, d, rr, gg, bb;
    
    max = MAX(MAX(rgb->r, rgb->g), rgb->b);
    min = MIN(MIN(rgb->r, rgb->g), rgb->b);
    d = max - min;
    
    hsv->v = max;
    if (d > 0)
	hsv->s = d * 0xffff / max;
    else
	hsv->s = 0;
    
    if (hsv->s == 0) {
	hsv->h = 0;
    } else {
	rr = max - rgb->r * 60 / d;
	gg = max - rgb->g * 60 / d;
	bb = max - rgb->b * 60 / d;
	
	if (rgb->r == max)
	    hsv->h = bb - gg;
	else if (rgb->g == max)
	    hsv->h = 120 + rr - bb;
	else
	    hsv->h = 240 + gg - rr;
	
	if (hsv->h < 0)
	    hsv->h += 360;
    }
}

static void hsv2rgb(struct color_t *rgb, struct hsvcolor_t *hsv)
{
    gushort d, dd1, dd2, dd3;
    
    rgb->r = rgb->g = rgb->b = hsv->v;
    if (hsv->s > 0) {
	d = hsv->h % 60;
	dd1 = hsv->v * hsv->s / 0xffff;
	dd2 = dd1 * d / 60;
	dd3 = dd1 * (60 - d) / 60;
	
	switch (hsv->h / 60) {
	case 0:
	    rgb->g -= dd3;
	    rgb->b -= dd1;
	    break;
	case 1:
	    rgb->b -= dd1;
	    rgb->r -= dd2;
	    break;
	case 2:
	    rgb->b -= dd3;
	    rgb->r -= dd1;
	    break;
	case 3:
	    rgb->r -= dd1;
	    rgb->g -= dd2;
	    break;
	case 4:
	    rgb->r -= dd3;
	    rgb->g -= dd1;
	    break;
	default:
	    rgb->g -= dd1;
	    rgb->b -= dd2;
	}
    }
}

int main(void)
{
    struct color_t c;
    struct hsvcolor_t hc;
    
#if 0
    c.a = 0xffff;
    c.r = 0xaaaa;
    c.g = 0xaaaa;
    c.b = 0x0000;
#else
    c.a = 0xff;
    c.r = 0xaa;
    c.g = 0xaa;
    c.b = 0x00;
#endif
    printf("%04x:%04x:%04x:%04x\n", c.a, c.r, c.g, c.b);
    
    rgb2hsv(&hc, &c);
    printf("%d:%d:%d\n", hc.h, hc.s, hc.v);
    
    hsv2rgb(&c, &hc);
    printf("%04x:%04x:%04x:%04x\n", c.a, c.r, c.g, c.b);
    
    return 0;
}
