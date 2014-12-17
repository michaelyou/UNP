#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <signal.h>

#define MAXLINE 4096
#define SERV_PORT 9877
#define LISTENQ 10

#if 0
void sig_child(int signo)
{
    pid_t pid;
    int stat;

    pid = wait(&stat);
    printf("child %d terminated\n", pid);
    return;
}
#endif

void sig_child(int signo)
{
    pid_t pid;
    int stat;

    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}

void str_echo(int sockfd)
{
    ssize_t n;
    char buf[MAXLINE];

again:
    while((n = recv(sockfd, buf, MAXLINE, 0)) > 0) {
        if(send(sockfd, buf, strlen(buf), 0) != n)
            printf("write error!\n");
        memset(buf, 0, MAXLINE);
    }
    if(n < 0 && errno == EINTR)
        goto again;
    else if(n < 0)
        printf("str_echo : read error!\n");
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error: %s", strerror(errno));
        exit(-1);
    }
    
    printf("sockfd is %d\n", listenfd);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);


    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("bind error: %s\n", strerror(errno));
        exit(-1);
    }

    if(listen(listenfd, LISTENQ) < 0) {
        printf("listen error: %s \n", strerror(errno));
        exit(-1);
    }

    struct sigaction sa;
    sa.sa_handler=sig_child;    //信号处理函数
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    sigaction(SIGCHLD, &sa, NULL);


    for(;;) {
        clilen = sizeof(cliaddr);
    
        if((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            if(errno == EINTR)
                continue;
            else
                perror("accept");
        }
        
        printf("the connfd is %d\n", connfd); 
        if((childpid = fork()) == 0) {
            close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        close(connfd);
    }
}
