#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define MAXEVENTS 10

void set_nonblocking(int sockfd)
{
    int opts;

    opts = fcntl(sockfd, F_GETFL);
    if(opts < 0) {
        perror("fcntl");
        exit(1);
    }
    opts = (opts | O_NONBLOCK);
    if(fcntl(sockfd, F_SETFL, opts) < 0) {
        perror("fcntl");
        exit(1);
    }
}

int main(int argc, char **argv)
{
    struct epoll_event ev, events[MAXEVENTS];
    int addrlen, listenfd, conn_sock, nfds, epfd, fd, i, nread, n;
    struct sockaddr_in local, remote;
    char buf[BUFSIZ];   //BUFSIZ=8194

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    set_nonblocking(listenfd);

    bzero(&local, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(atoi(argv[1]));
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listenfd, (struct sockaddr *)&local, sizeof(local)) < 0) {
        perror("bind");
        exit(1);
    }

    if(listen(listenfd, 20) == -1) {
        perror("listen");
        exit(1);
    }

    epfd = epoll_create1(0);
    if(epfd == -1) {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    for(;;) {
        nfds = epoll_wait(epfd, events, MAXEVENTS, -1);
        if(nfds == -1) {
            perror("epoll_pwait");
            exit(EXIT_FAILURE);
        }
        for(i = 0; i < nfds; i++) {
            fd = events[i].data.fd;
            if(fd == listenfd) {
                while((conn_sock = accept(listenfd, (struct sockaddr *)&remote,
                                           (socklen_t *)&addrlen)) > 0) {
                    set_nonblocking(conn_sock);
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = conn_sock;
                    if(epoll_ctl(epfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                        perror("epoll_ctl: add");
                        exit(EXIT_FAILURE);
                    }
                }
                if(conn_sock == -1) {
                    if(errno != EAGAIN && errno != ECONNABORTED 
                          && errno != EPROTO && errno != EINTR)
                        perror("accept");
                }
                continue;
            }

            if(events[i].events & EPOLLIN) {
                n = 0;
                while((nread = read(fd, buf + n, BUFSIZ - 1)) > 0) {
                    n += nread;
                }
                if(nread == -1 && errno != EAGAIN) {
                    perror("read error");
                }
                //write(STDOUT_FILENO, buf, n);
                ev.data.fd = fd;
                ev.events = events[i].events | EPOLLOUT;   //会继续执行下面的if语句，向对端响应
                if(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
                    perror("epoll_ctl: mod");
                }
            }

            if(events[i].events & EPOLLOUT) {
                sprintf(buf, "HTTP:1.1 200 OK\r\nContent-Length: %d\r\n\r\nHello World", 11);
                int nwrite = 0, data_size = strlen(buf);
                n = data_size;
                while(n > 0) {
                    nwrite = write(fd, buf + data_size -n, n);
                    if(nwrite < n) {
                        if(nwrite == -1 && errno != EAGAIN) {
                            perror("write error");
                        }
                        break;
                        if(errno == EAGAIN) {
                        //说明网络堵塞，等待下次再发，这样的话n应该定义在if外，类似于全局变量
                        }
                    }
                    n -= nwrite;
                }
                ev.data.fd = fd;
                ev.events = EPOLLIN | EPOLLET;
                if(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
                    perror("epoll_ctl: mod");
                }
                
                //close(fd);
            }
        }
    }
    return(0);
    
}
