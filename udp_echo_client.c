#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXLINE 4096
#define SERV_PORT 9877

void dg_cli(int sockfd, const struct sockaddr *pservaddr, socklen_t servlen)
{
    int n;
    char sendline[MAXLINE], recvline[MAXLINE];
    #if 0
    for(;;) {
        n = read(STDIN_FILENO, sendline, MAXLINE);

        //sendline[n] = '\0';

        sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);

        n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);

        //recvline[n] = '\0';

        write(STDOUT_FILENO, recvline, n);
    }
    #endif
     memset(sendline, 'a', 345);
     sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
     memset(sendline, 'b', 567);
     sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
}
   
    
int main(int argc, char **argv) 
{
    struct sockaddr_in servaddr;
    int sockfd;
    socklen_t servlen;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    //servaddr.sin_addr.s_addr = htonl(argv[1]);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    dg_cli(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    exit(0);
}
