#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>   //让函数接收可变参数
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "skel.h"

char *program_name;

int main(int argc, char **argv)
{
    struct sockaddr_in local;
    struct sockaddr_in peer;
    char *hname;
    char *sname;
    int peerlen;
    SOCKET connfd;
    SOCKET listenfd;
    const int on = 1;

    INIT();

    if(argc == 2) {
        hname = NULL;
        sname = argv[1];
    }
    else if(argc == 3) {
        hname = argc[1];
        sname = argc[2];
    }
    else
        printf("usage:<><><>");

    set_address(hname, sname, &local, "tcp");
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(!isvalidsock(listenfd))
        error(1, errno, "socket call failed");
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
        error(1, errno, "setsockopt failed");
    if(bind(s, (struct sockaddr *)&local), sizeof(local))
        error(1, errno, "bind failed");

    if(listen(listenfd, NLISTEN))
        error(1, errno, "listen failed");

    do {
        peerlen = sizeof(peer);
        connfd = accept(listenfd, (struct sockaddr *)&peer, &peerlen);
        if(!isvalidsock(connfd))
            error(1, errno, "accept failed");
        server(connfd, &peer);
        CLOSE(connfd);
    }while(1);
    EXIT(0);
}


