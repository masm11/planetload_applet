#include <stdio.h>
#include <errno.h>

typedef char gchar;

static gchar *get_scheme_list(void)
{
    gchar *line;
    int fds[2];
    
    if (pipe(fds) == -1) {
	perror("pipe");
	return NULL;
    }
    
    switch (fork()) {
    case -1:
	perror("fork");
	return NULL;
	
    case 0:
	close(fds[0]);
	close(0);
	dup2(fds[1], 1);
	dup2(fds[1], 2);
	execl("/sbin/netscheme", "/sbin/netscheme", "-ql", NULL);
	perror("/sbin/netscheme");
	exit(1);
    }
    
    close(fds[1]);
    line = g_strdup("");;
    while (1) {
	gchar buf[128];
	int siz;
	
	siz = read(fds[0], buf, sizeof buf - 1);
	if (siz == -1) {
	    if (errno == EINTR)
		continue;
	    perror("read");
	    goto pipeerr;
	}
	
	if (siz == 0)
	    break;
	
	buf[siz] = '\0';
	
	{
	    gchar *old = line;
	    line = g_strconcat(old, buf, NULL);
	    g_free(old);
	}
    }
    
    close(fds[0]);
    
    return line;
    
 pipeerr:
    close(fds[0]);
    if (line != NULL)
	g_free(line);
    return NULL;
}

int main(void)
{
    printf("%s\n", get_scheme_list());
}
