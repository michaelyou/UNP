/*
 *此函数使用了指数补偿算法。如果调用connect失败，进程会休眠
 *一小段时间，每次循环休眠的时间会以指数级增加。在基于BSD的
 *套接字实现中，如果第一次连接尝试失败，那么在TCP中继续使用
 *同一个套接字描述符，接下来仍然会失败。这是一个历史遗留问题
 *为了增加本函数的移植性。所以在connect失败后，会关闭套接字
 *重新的时候，打开一个新的套接字
 */

#include "apue.h"
#include <sys/socket.h>
#include "socket_api.h"

#define MAXSLEEP 128

int connect_retry(int domain, int type, int protocol,
                  const struct sockaddr *addr, socklen_t alen)
{
    int numsec, fd;
    /*
     *try to reconnect with exponential backoff
     */
    for(numsec = 1; numsec <= MAXSLEEP; numsec << 1) {
        if((fd = socket(domain, type, protocol)) < 0) {
            printf("create socket failed\n");
            return(-1);
        }
        if(connect(fd, addr, alen) == 0) {
            printf("connect to socket success\n");
            return(fd);
        }
        close(fd);

        //delay before try again
        if(numsec <= MAXSLEEP/2) {
            sleep(numsec);
        }
        printf("try to connnet again\n");
    }
    printf("connect socket failed, stop trying\n");
    return(-1);
}
