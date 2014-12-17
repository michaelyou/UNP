#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

#define	max(a,b)	((a) > (b) ? (a) : (b))

#define MAXLINE 4096
#define SERV_PORT 9877

void str_cli(FILE *fp, int sockfd)
{
    char buf[MAXLINE];
    int maxfdp1, stdineof;
    fd_set rset;
    int n;

    stdineof = 0;
    FD_ZERO(&rset);
    for(;;) {
        if(stdineof == 0)
            FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(sockfd, &rset)) {   //从socket读，写到标准输出
            if((n = read(sockfd, buf, MAXLINE)) == 0) {
                if(stdineof == 1)
                    return;
                else
                    printf("str_cli: server terminated prematurely\n");
            }
            write(fileno(stdout), buf, n);
        }

        if(FD_ISSET(fileno(fp), &rset)) {   //从标准输入读，往socket写
            if((n = read(fileno(fp), buf, MAXLINE)) == 0) {
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            write(sockfd, buf, n);
        }
    }
}

int main(int argc , char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;

    if(argc != 2)
        printf("usage: tcpli <IPaddress>\n");

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror("socket");
    printf("sockfd is %d\n", sockfd);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 0)
        perror("inet_pton");
    
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
        perror("connect");

    str_cli(stdin, sockfd);

    exit(0);
}
