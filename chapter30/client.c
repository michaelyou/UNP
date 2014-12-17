#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#define MAXN 16384
#define MAXLINE 4096

/*本函数会返回所有合适的地址，但是只返回第一个可以connect成功的地址，
  其余地址均会被忽略*/
int tcp_connect(const char *host, const char *serv)
{
    int sockfd, n;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
        printf("tcp connect error for%s, %s: %s", host, serv, gai_strerror(n));

    ressave = res;
    do {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(sockfd < 0)
            continue;
        if(connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)  //success
            break;

        close(sockfd);
    }while((res = res->ai_next) != NULL);

    if(res == NULL)
        printf("tcp_connect error for %s, %s", host, serv);

    freeaddrinfo(ressave);

    return(sockfd);
}

int main(int argc, char **argv)
{
    int i, j, fd, nchildren, nloops, nbytes;
    pid_t pid;
    ssize_t n;
    char request[MAXLINE], reply[MAXLINE];

    if(argc != 6)
        printf("usage: client <hostname or IPaddr><port><#children>"
                       "<#loops/child><#bytes/request>\n");
    nchildren = atoi(argv[3]);
    nloops = atoi(argv[4]);
    nbytes = atoi(argv[5]);
    snprintf(request, sizeof(request), "%d\n", nbytes);

    for(i = 0; i < nchildren; i++) {
        if((pid = fork()) == 0) {
            for(j=0; j < nloops; j++) {
                fd = tcp_connect(argv[1], argv[2]);

                write(fd, request, strlen(request));
                if((n = read(fd, reply, nbytes)) != nbytes)
                    perror("read");

                close(fd);
                
            }
            printf("child %d done\n", i);
            exit(0);
        }
    }
    while(wait(NULL) > 0)
        ;
    if(errno != ECHILD)
        perror("wait");

    exit(0);
}
