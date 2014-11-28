#include <stdio.h>
#include <netdb.h>

int main()
{
    struct hostent *host;
    host = gethostent();
    printf("host name is %s\n", host->h_name);
    printf("the addres type is %d\n", host->h_addrtype);
    printf("the addres length is %d\n", host->h_length);
}
