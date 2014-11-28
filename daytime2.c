#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 4096

int main(int argc, char *argv[])
{
    int sockfd, n, err;
    char recvline[MAXLINE + 1];
    struct addrinfo *ailist, *aip;
    struct addrinfo hint;

    if(argc != 2) {
        printf("usage: %s <IPaddress>\n", argv[0]);
    }

    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;

    if((err = getaddrinfo(argv[1], "daytime", &hint, &ailist)) != 0)
        printf("getaddrinfo error: %s", gai_strerror(err));
    for(aip = ailist; aip != NULL; aip = aip->ai_next) {
        if((sockfd = socket(aip->ai_family, SOCK_STREAM, 0)) < 0) {
            printf("socket error\n");
            return(-1);
        }
        if(connect(sockfd, aip->ai_addr, aip->ai_addrlen) < 0) {
            printf("connect error\n");
            perror("connect");
        }
        while((n=read(sockfd,recvline,MAXLINE))>0){
            recvline[n] = 0;
            if(fputs(recvline, stdout) == EOF)
                printf("fputs error\n");
        }

        if(n < 0)
            printf("read error\n");

        exit(0);
    }
    return(0); 
}

