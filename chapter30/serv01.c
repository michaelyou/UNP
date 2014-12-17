#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include <signal.h>

#define MAXN 16384
#define MAXLINE 4096
#define LISTENQ 1024

int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
    int listenfd, n;
    const int on = 1;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((n = getaddrinfo(host, serv, &hints, &res)) != 0)
        printf("getaddrinfo error for %s, %s, %s", host, serv, gai_strerror(n));

    ressave = res;

    do {
        listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(listenfd < 0)
            continue;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if(bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
            break;

        close(listenfd);
    }while((res = res->ai_next) != NULL);

    if(listen(listenfd, LISTENQ) < 0)
        perror("listen");

    if(res == NULL)
        printf("tcp_listen error\n");

    if(addrlenp)
        *addrlenp = res->ai_addrlen;
    freeaddrinfo(ressave);

    return(listenfd);
}


void pr_cpu_time(void)
{
    printf("generate time used\n");
    double user, sys;
    struct rusage myusage, childusage;

    if(getrusage(RUSAGE_SELF, &myusage) < 0)
        printf("getrusage error\n");
    
    if(getrusage(RUSAGE_CHILDREN, &childusage) < 0)
        printf("getrusage error\n");

    user = (double)myusage.ru_utime.tv_sec + myusage.ru_utime.tv_usec/ 1000000.0;
    user += (double)childusage.ru_utime.tv_sec + childusage.ru_utime.tv_usec/ 1000000.0;
    sys = (double)myusage.ru_stime.tv_sec + myusage.ru_stime.tv_usec/ 1000000.0;
    sys += (double)childusage.ru_stime.tv_sec + childusage.ru_stime.tv_usec/ 1000000.0;

    printf("user time = %g, sys time = %g\n", user, sys);
}

void sig_int(int signo)
{
    //void pr_cpu_time(void);
    printf("got a sigint\n");
    pr_cpu_time();

    exit(0);
}

void sig_chld(int signo)
{
    pid_t pid;
    int stat;

    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        ;
    return;
}


void web_child(int sockfd)
{
    int ntowrite;
    ssize_t nread;
    char line[MAXLINE], result[MAXLINE];

    for(;;) {
        if((nread = read(sockfd, line, MAXLINE)) == 0)
            return;
        ntowrite = atol(line);
        if((ntowrite <= 0) || (ntowrite > MAXN))
            printf("client request for %d bytes\n", ntowrite);

        write(sockfd, result, ntowrite);
    }
}

//typedef void Sigfunc(int);

#if 0
void Signal(int signo, void *func)
{
    printf("catch sigint\n");
    struct sigaction act;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
   
    if(sigaction(signo, &act, NULL))
        perror("sigaction\n");
      
    
}
#endif


int main(int argc, char **argv)
{
    int listenfd, connfd;
    pid_t childpid;
    //void sig_chld(int), sig_int(int), web_child(int);
    socklen_t clilen, addrlen;
    struct sockaddr *cliaddr;

    //Signal(SIGCHLD, sig_chld);
    struct sigaction act;
    act.sa_handler = sig_int;
    act.sa_flags = 0;
    if(sigaction(SIGINT, &act, NULL))
        perror("sigaction\n");
    

    if(argc == 2)
        listenfd = tcp_listen(NULL, argv[1], &addrlen);
    else if(argc == 3)
        listenfd = tcp_listen(argv[1], argv[2], &addrlen);
    else {
        printf("usage: serv01 [<host>] <port>\n");
        exit(-1);
    }

    cliaddr = malloc(addrlen);

    for(;;) {
        clilen = addrlen;
        if((connfd = accept(listenfd, cliaddr, &clilen)) < 0) {
            //printf("accept error :%s\n", strerror(errno));
            if(errno = EINTR)
                continue;
            else
                perror("accept");
        }
        else
            //printf("accept success\n");

        if((childpid = fork()) == 0) {
            close(listenfd);
            web_child(connfd);
            exit(0);
        }
        close(connfd);
    }
        
}
