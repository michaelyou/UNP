#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>

#define MAXLINE 4096
#define SERV_PORT 9877

void dg_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen)
{
    int n;
    socklen_t len;
    char mesg[MAXLINE];
    #if 0
    for(;;) {
        len = clilen;
        n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);

        sendto(sockfd, mesg, n, 0, pcliaddr, len);
        
    }
    #endif
    n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
    printf("i recv %d bytes\n", n);
    n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
    printf("i recv %d bytes\n", n);
}

int main(int argc, char **argv) 
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    dg_echo(sockfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
}
