#include "../config.h"
#include <stdio.h>
#include "ifstat.h"

int main(int argc, char **argv)
{
    struct ifstat_t *ifs;
    
    while (1) {
	int i;
	
	ifs = ifstat_get(argv[1]);
	
	printf("%d ", ifs->up);
	printf("%d ", ifs->naddr4);
	for (i = 0; i < ifs->naddr4; i++) {
	    printf("[%d.%d.%d.%d/%d] ",
		    ifs->addr4[i].addr[0],
		    ifs->addr4[i].addr[1],
		    ifs->addr4[i].addr[2],
		    ifs->addr4[i].addr[3],
		    ifs->addr4[i].prefixlen);
	}
	printf("%d ", ifs->naddr6);
	for (i = 0; i < ifs->naddr6; i++) {
	    printf("[%x:%x:%x:%x:%x:%x:%x:%x/%d] ",
		    ifs->addr6[i].addr[ 0] << 8 | ifs->addr6[i].addr[ 1],
		    ifs->addr6[i].addr[ 2] << 8 | ifs->addr6[i].addr[ 3],
		    ifs->addr6[i].addr[ 4] << 8 | ifs->addr6[i].addr[ 5],
		    ifs->addr6[i].addr[ 6] << 8 | ifs->addr6[i].addr[ 7],
		    ifs->addr6[i].addr[ 8] << 8 | ifs->addr6[i].addr[ 9],
		    ifs->addr6[i].addr[10] << 8 | ifs->addr6[i].addr[11],
		    ifs->addr6[i].addr[12] << 8 | ifs->addr6[i].addr[13],
		    ifs->addr6[i].addr[14] << 8 | ifs->addr6[i].addr[15],
		    ifs->addr6[i].prefixlen);
	}
	printf("%lld %lld", ifs->bytes_in, ifs->bytes_out);
	printf("\n");
	
	usleep(100000);
	ifstat_free(ifs);
    }
    
    return 0;
}

/*EOF*/
