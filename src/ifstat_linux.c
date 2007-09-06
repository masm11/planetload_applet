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
 */

#include "../config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <net/if.h>
#include "ifstat.h"

static gboolean get_up(char *ifname)
{
    struct ifreq ifr;
    int s;
    gboolean retval = 0;
    
    s = socket(PF_INET, SOCK_DGRAM, 0);
    if (s == -1)
	return 0;
    
    strcpy(ifr.ifr_name, ifname);
    
    if (ioctl(s, SIOCGIFFLAGS, &ifr) == 0) {
	if (ifr.ifr_flags & IFF_UP)
	    retval = 1;
    }
    
    close(s);
    
    return retval;
}

static gboolean get_addr4(char *ifname, struct ifstat_t *ifs)
{
    struct ifreq ifr;
    int s;
    
    s = socket(PF_INET, SOCK_DGRAM, 0);
    if (s == -1)
	return FALSE;
    
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    
    gboolean retval = FALSE;
    if (ioctl(s, SIOCGIFADDR, &ifr) == 0) {
	struct in_addr addr4 = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr;
	
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(s, SIOCGIFNETMASK, &ifr) == 0) {
	    guchar *m = (guchar *) &((struct sockaddr_in *) &ifr.ifr_netmask)->sin_addr;
	    gint plen = 0;
	    gint i, j;
	    for (i = 0; i < 4; i++) {
		for (j = 0; j < 8; j++) {
		    if (!(m[i] & (0x80 >> j)))
			goto found_plen4;
		    plen++;
		}
	    }
	 found_plen4:
	    ifs->naddr4 = 1;
	    ifs->addr4 = malloc(sizeof *ifs->addr4);
	    memset(ifs->addr4, 0, sizeof *ifs->addr4);
	    ifs->addr4->addr[0] = ((guchar *) &addr4)[0];
	    ifs->addr4->addr[1] = ((guchar *) &addr4)[1];
	    ifs->addr4->addr[2] = ((guchar *) &addr4)[2];
	    ifs->addr4->addr[3] = ((guchar *) &addr4)[3];
	    ifs->addr4->prefixlen = plen;
	    retval = TRUE;
	}
    }
    
    close(s);
    
    return retval;
}

static void get_bytes(char *ifname, struct ifstat_t *ifs)
{
    char buf[1024];
    FILE *fp;
    int ver;
    
    if ((fp = fopen("/proc/net/dev", "rt")) == NULL) {
	return;
    }
    
    fgets(buf, sizeof buf, fp);
    fgets(buf, sizeof buf, fp);
    if (strstr(buf, "compressed") != NULL)
	ver = 3;
    else if (strstr(buf, "bytes") != NULL)
	ver = 2;
    else
	ver = 1;
    
    while (fgets(buf, sizeof buf, fp) != NULL) {
	gchar *colon = strchr(buf, ':');
	gchar *p;
	guint64 in, out;
	gchar ifn[16];
	for (p = colon + 1; *p != '\0'; p++) {
	    if (isdigit(*p))
		break;
	}
	if (*p == ':')
	    colon = p;
	*colon = '\0';
	if (sscanf(buf, "%s", ifn) != 1)
	    continue;
	if (strcmp(ifn, ifname) != 0)
	    continue;
	
	switch (ver) {
	case 3:
	    if (sscanf(colon + 1, "%llu %*u %*u %*u %*u %*u %*u %*u %llu %*u %*u %*u %*u %*u %*u %*u",
			    &in, &out) != 2)
		continue;
	    break;
	    
	case 2:
	    if (sscanf(colon + 1, "%llu %*u %*u %*u %*u %*u %llu %*u %*u %*u %*u %*u %*u",
			    &in, &out) != 2)
		continue;
	    break;
	    
	case 1:
	    in = out = 0;
	    break;
	    
	default:
	    in = out = 0;
	}
	
	ifs->bytes_in = in;
	ifs->bytes_out = out;
    }
    
    fclose(fp);
}

static void get_addr6(char *ifname, struct ifstat_t *ifs)
{
    FILE *fp;
    char buf[1024];
    
    if ((fp = fopen("/proc/net/if_inet6", "rt")) == NULL) {
	return;
    }
    
    while (fgets(buf, sizeof buf, fp) != NULL) {
	char addrstr[40], *p;
	int idx;
	int plen;
	int scope;
	int dad_status;
	char devname[16];
	int i;
	struct ifstat_addr_t addr6;
	
	if (sscanf(buf, "%40s %x %x %x %x %16s",
			addrstr, &idx, &plen, &scope, &dad_status, devname) != 6) {
	    continue;
	}
	
	if (strlen(addrstr) != 16 * 2)
	    continue;
	
	if (strcmp(devname, ifname) != 0)
	    continue;
	
	memset(&addr6, 0, sizeof addr6);
	p = addrstr;
	for (i = 0; i < 16; i++) {
	    int hi = *p++;
	    int lo = *p++;
	    
	    if (hi >= '0' && hi <= '9')
		hi = hi - '0';
	    else if (hi >= 'A' && hi <= 'F')
		hi = hi - 'A' + 0x0a;
	    else if (hi >= 'a' && hi <= 'f')
		hi = hi - 'a' + 0x0a;
	    else
		break;
	    
	    if (lo >= '0' && lo <= '9')
		lo = lo - '0';
	    else if (lo >= 'A' && lo <= 'F')
		lo = lo - 'A' + 0x0a;
	    else if (lo >= 'a' && lo <= 'f')
		lo = lo - 'a' + 0x0a;
	    else
		break;
	    
	    addr6.addr[i] = hi << 4 | lo;
	}
	addr6.prefixlen = plen;
	addr6.scope = scope;
	
	if (ifs->addr6 != NULL) {
	    ifs->addr6 = realloc(ifs->addr6, sizeof *ifs->addr6 * (ifs->naddr6 + 1));
	} else {
	    ifs->addr6 = malloc(sizeof *ifs->addr6 * (ifs->naddr6 + 1));
	}
	
	ifs->addr6[ifs->naddr6++] = addr6;
    }
    
    fclose(fp);
}

struct ifstat_t *ifstat_get(char *ifname, gboolean is_ppp, const gchar *lockfile)
{
    struct ifstat_t *ifs;
    
    ifs = malloc(sizeof *ifs);
    memset(ifs, 0, sizeof *ifs);
    ifs->stat = IFSTAT_DOWN;
    
    if (get_up(ifname)) {
	ifs->stat = IFSTAT_NOADDR;
	if (get_addr4(ifname, ifs))
	    ifs->stat = IFSTAT_UP;
	get_bytes(ifname, ifs);
	get_addr6(ifname, ifs);
    }
    
    if (is_ppp) {
	if (ifs->stat == IFSTAT_DOWN) {
	    struct stat st;
	    ifs->stat = IFSTAT_NOADDR;
	    if (stat(lockfile, &st) == -1) {
		if (errno == ENOENT)
		    ifs->stat = IFSTAT_DOWN;
		if (errno == ENOTDIR)
		    ifs->stat = IFSTAT_DOWN;
		if (errno == EACCES)
		    ifs->stat = IFSTAT_DOWN;
	    }
	}
    }
    
    return ifs;
}

void ifstat_free(struct ifstat_t *ifs)
{
    if (ifs != NULL) {
	if (ifs->addr4 != NULL)
	    free(ifs->addr4);
	if (ifs->addr6 != NULL)
	    free(ifs->addr6);
	free(ifs);
    }
}

/*EOF*/
