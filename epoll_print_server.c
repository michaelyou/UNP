#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

#define MAXEVENTS 64

static int create_and_bind(char *port)
{
    struct addrinfo hints;
    struct addrinfo *results, *rp;
    int s, sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;       //return ipv4 and ipv6 choice  
    hints.ai_socktype = SOCK_STREAM;   //tcp
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, port, &hints, &results);
    if(s != 0) {
        fprintf(stderr, "getaddrinfo : %s\n", gai_strerror(s));
    }

    for(rp = results; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sfd == -1)
            continue;

        s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
        if(s == 0) {
            break;            //bind successfully, do not need to con
        }

        close(sfd);           //bind failed, close the fd
    }
    if(rp == NULL) {
        fprintf(stderr, "could not bind\n");
        return(-1);
    }
    freeaddrinfo(results);
    return sfd;
}

static int make_socket_unblocking(int sfd)
{
    int flags, s;
    flags = fcntl(sfd, F_GETFL, 0);  //get sfd's flags
    if(flags == -1) {
        perror("fcntl");
        return(-1);
    }
    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if(s == -1) {
        perror("fcntl");
        return(-1);
    }
}

int main(int argc, char **argv)
{
    int sfd, s;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;

    if(argc != 2) {
        fprintf(stderr, "usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sfd = create_and_bind(argv[1]);
    if(sfd == -1) 
        abort();

    s = make_socket_unblocking(sfd);
    if(s == -1)
        abort();       //异常终止此进程

    s = listen(sfd, SOMAXCONN);  //somaxconn: default is 128
    if(s == -1) {
        perror("listen");
        abort();
    }
    
    efd = epoll_create1(0); // same function as epoll_create
    if(efd == -1) {
        perror("epoll_create1");
        abort();
    }

    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET; //读入，边缘触发方式
    s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);  //注册sfd到efd中
    if(s == -1) {
        perror("epoll_ctl");
        abort();
    }

    events = calloc(MAXEVENTS, sizeof(event));

    while(1) {
        int n, i;
        n = epoll_wait(efd, events, MAXEVENTS, -1);
        for(i = 0; i < n; i++) {
            if((events[i].events & EPOLLERR) || 
               (events[i].events & EPOLLHUP) ||
               (!(events[i].events & EPOLLIN))) {
               //an error has occored on this fd, or the socket is not
               //ready for reading
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            else if(sfd == events[i].data.fd) {
                //we have a notification on the listening socket, which 
                // means one or more incoming connections
                while(1) {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof(in_addr);
                    infd = accept(sfd, &in_addr, &in_len);
                    if(infd == -1) {
                        if((errno == EAGAIN) ||
                           (errno == EWOULDBLOCK)) {
                            //we have processed all the incoming connections
                            break;
                           }
                        else {
                            printf("accept\n");
                            break;
                        }
                    }
                    //将地址转化为主机名和服务名
                    s = getnameinfo(&in_addr, in_len, hbuf, sizeof(hbuf),
                                    sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
                    if(s == 0) {
                        printf("accept connection on descriptor %d"
                               "(host = %s, port = %s)\n", infd, hbuf, sbuf);
                    }           

                    //make the coming socket non-blocking and add it to the list of fds to monitor
                    s = make_socket_unblocking(infd);
                    if(s == -1)
                        abort();

                    event.data.fd = infd;
                    event.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
                    if(s == -1) {
                        perror("epoll_ctl");
                        abort();
                    }
                }
                continue;  // we have processed all the on sfd, so we continue with the for loop
            }
            else {
                //we have data on the fd waiting to be read. read
                //and display it. we must read whatever data is avaliable
                //completely, as we are running in edge-triggered mode and
                //won't get a notification again for the same data
                int done = 0;
                while(1) {
                    ssize_t count;
                    char buf[512];

                    count = read(events[i].data.fd, buf, sizeof(buf));
                    if(count == -1) {
                        //if errno == EAGAIN, thar means we have read all
                        //the data. so go back to the main loop
                        if(errno != EAGAIN) {
                            perror("read");
                            done = 1;
                        }
                        break;
                    }
                    else if(count == 0) {
                        //end of file. the remote has closed the connection
                        done = 1;
                        break;
                    }
                    //write the buffer to the stdout
                    s = write(STDOUT_FILENO, buf, count);
                    if(s == -1) {
                        perror("write");
                        abort();
                    }
                }
                if(done) {
                    printf("closed connection on descriptor %d\n", events[1].data.fd);
                    //close the descriptor will make epoll remove it from
                    //the set of descriptors which are monitored
                    close(events[i].data.fd);
                }
            }
        }
       
    }
    free(events);
    close(sfd);
    return(EXIT_SUCCESS);
    
    
}
