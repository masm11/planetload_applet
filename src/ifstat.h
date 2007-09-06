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

#ifndef IFSTAT_H__INCLUDED
#define IFSTAT_H__INCLUDED

#include "../config.h"

#include <glib.h>

struct ifstat_addr_t {
    guchar addr[16];
    gint prefixlen;
    gint scope;
};

enum ifstat_stat_t {
    IFSTAT_DOWN,
    IFSTAT_NOADDR,
    IFSTAT_UP,
};

struct ifstat_t {
    enum ifstat_stat_t stat;
    
    gint naddr4;
    struct ifstat_addr_t *addr4;
    
    gint naddr6;
    struct ifstat_addr_t *addr6;
    
    guint64 bytes_in, bytes_out;
};

struct ifstat_t *ifstat_get(char *ifname, gboolean is_ppp, const gchar *lock_file);
void ifstat_free(struct ifstat_t *ifs);

#endif	/* ifndef IFSTAT_H__INCLUDED */
