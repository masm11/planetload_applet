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
#include <stdarg.h>
#include "i18n-support.h"
#include "debug.h"

#ifdef DEBUG

#include <signal.h>

static FILE *fp = NULL;

static void sig(int s)
{
    if (fp != NULL)
	fflush(fp);
//  raise(SIGSEGV);
}

void debug_init(void)
{
    if (fp == NULL)
	fp = fopen("/home/masm/planetload_applet.log", "at");
//  signal(SIGSEGV, sig);
}

void debug_log(char *fmt, ...)
{
    va_list ap;
    if (fp != NULL) {
	va_start(ap, fmt);
	vfprintf(fp, fmt, ap);
	va_end(ap);
	fflush(fp);
    }
}

#else

void debug_init(void) {}
void debug_log(char *fmt, ...) {(void) fmt;}

#endif

/*EOF*/
