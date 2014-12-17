#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/types.h>


#define MAXN 16384
#define MAXLINE 4096
#define LISTENQ 1024

static int nchildren;
static pid_t *pids;


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

void sig_int(int signo)
{
    int i;
    printf("caught sigint\n");
    
    for(i = 0; i < nchildren; i++) {
        kill(pids[i], SIGTERM);
        printf("send sigterm to %d child done", i);
        }

    printf("kill all done\n");
    while(wait(NULL) > 0)
        ;

    printf("wait done\n");
    
    if(errno != ECHILD)
        printf("wait error");

    pr_cpu_time();
    exit(0);
}

void child_main(int i, int listenfd, int addrlen)
{
    int connfd;
    void web_child(int);
    socklen_t clilen;
    struct sockaddr *cliaddr;

    cliaddr = malloc(addrlen);
    if(cliaddr == NULL)
        perror("malloc");
    printf("child %ld starting\n", (long)getpid());
    for(;;) {
        clilen = addrlen;
        connfd = accept(listenfd, cliaddr, &clilen);

        web_child(connfd);
        close(connfd);
    }
}

void Signal(int signo, void *func)
{
    printf("register sigint\n");
    struct sigaction act;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
   
    if(sigaction(signo, &act, NULL))
        perror("sigaction\n");
      
    
}

pid_t child_make(int i, int listenfd, int addrlen)
{
    pid_t pid;
    if((pid = fork()) > 0)
        return(pid);

    child_main(i, listenfd, addrlen);
}


int main(int argc, char **argv)
{
    int listenfd, i;
    socklen_t addrlen;
    //void sig_int(int);
    pid_t child_make(int, int, int);

    printf("the pid of main is %d\n", getpid());

    if(argc == 3)
        listenfd = tcp_listen(NULL, argv[1], &addrlen);
    else if(argc == 4)
        listenfd = tcp_listen(argv[1], argv[2], &addrlen);
    else {
        printf("usage; serv02 [<host>]<#port><#children>\n");
    }
    
    nchildren = atoi(argv[argc-1]);
    pids = calloc(nchildren, sizeof(pid_t));
    if(pids == NULL)
        perror("calloc");

    for(i = 0; i < nchildren; i++) {
        pids[i] = child_make(i, listenfd, addrlen);
        printf("%d\n", pids[i]);
    }
    

    Signal(SIGINT, sig_int);

    for(;;)
        pause();
}
