#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>


int main(int argc, char *argv[])
{
    struct hostent *hptr;
    char *message = NULL;	
    char **pptr;
    char str[32];
    char *path = argv[1];
    int addr = 0;

    if (path == NULL)
    {
	printf("your input para is NULL\n");
	return -1;
    }

    hptr = gethostbyname(path);
    if (hptr == NULL)
    {
	printf("gethostbyname error for host:%s\n", path);
	return -1;
    }

    printf("official hostname:%s\n", hptr->h_name);

    for (pptr = hptr->h_aliases; *pptr != NULL; pptr++)
    {
	printf("alias:%s\n", *pptr);
    }

//    printf("AF_INET = %d\n", AF_INET);
//    printf("AF_INET6 = %d\n", AF_INET6);
//    printf("hostent type = %d\n", hptr->h_addrtype);
//    printf("hostent length = %d\n", hptr->h_length);
//    printf("switch begin\n");

    switch(hptr->h_addrtype)
    {
	case AF_INET:
	case AF_INET6:
//	    printf("switch handle\n"); 
//	    printf("h_addr:%s\n", hptr->h_addr);	
	    addr = inet_ntop(hptr->h_addrtype, (void *)hptr->h_addr, str, sizeof(str));	    
	    printf("addr = %d\n", addr);
	    if (addr == 0)
	    {
		printf("get ip failed\n");
	    }

	    printf("first address:%s\n", str);

	    pptr = hptr->h_addr_list;
	    for(; *pptr != NULL; pptr++)
	    {
		addr = inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
		printf("addr=%d\n", addr);
		if (addr == 0)
            	{
                    printf("get ip failed\n");
            	}
	        printf("address list:%s\n", str);	
	    }
	break;
	default:
	    printf("unkown address type\n");
	break;
    }
//    printf("switch end\n");
    return 0;
}
