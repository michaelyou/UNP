#include "apue.h"
#include <netdb.h>
#include <sys/socket.h>
#include "socket_api.h"
#include "error.h"


#define BUFLEN  128

extern int connect_retry(int domain,int type,int protocol,
                         const struct sockaddr * addr,socklen_t alen);

void print_uptime(int sockfd)
{
    int n;
    char buf_sock[BUFLEN];
    char buf_usr[BUFLEN];
    if(fgets(buf_usr, BUFLEN, stdin) == NULL)
        printf("fgets failed");
    send(sockfd, buf_usr, strlen(buf_usr), 0);
    while((n = recv(sockfd, buf_sock, BUFLEN, 0)) > 0) {
        write(STDOUT_FILENO, buf_sock, n);
    }
    if(n < 0)
        err_sys("recv error");
}

int main(int argc, char *argv[])
{
    struct addrinfo *ailist, *aip;
    struct addrinfo hint;
    int sockfd, err;

    if(argc != 2)
        err_quit("usage: %s", argv[0]);
        
    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;
    if((err = getaddrinfo(argv[1], "ruptime", &hint, &ailist)) != 0) {
        err_quit("getaddinfo error %s", gai_strerror(err));
    }
    
    for(aip = ailist; aip != NULL; aip = aip->ai_next) {
        if((sockfd = connect_retry(aip->ai_family, SOCK_STREAM, 0,
                                   aip->ai_addr, aip->ai_addrlen)) < 0)
        err = errno;
        else {
            print_uptime(sockfd);
            exit(0);
        }
    }
    err_exit(err, "can not connect to %s", argv[1]);
}