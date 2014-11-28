/*
 *如果不调用setsockopt函数对fd进行设置，那么此函数就无法正常工作。
 *通常情况下，除非超时(超时时间一般是几分钟)，否则TCP的实现不允许绑定
 *同一个地址。我们使用套接字选项SO_REUSEADDR可以绕过这一限制
 *--P503
 */

#include "apue.h"
#include <sys/socket.h>
#include <errno.h>
#include "socket_api.h"
#include <arpa/inet.h>



int initserver(int type, const struct sockaddr *addr, socklen_t alen,
               int qlen)
{
    int fd, err;
    int reuse = 1;

    if((fd = socket(addr->sa_family, type, 0)) < 0) {
        printf("create socket failed\n");
        return(-1);
    }
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
        printf("set socket option failed\n");
        goto errout;
    }
    if(bind(fd, addr, alen) < 0) {
        printf("bind sockfd to addr failed\n");
        perror("bind");
        struct sockaddr_in *socktmp;
        socktmp = (struct sockaddr_in *)addr;
        printf("the port id is %d, the address is %s\n", ntohs(socktmp->sin_port),
                inet_ntoa(socktmp->sin_addr));
        goto errout;
    }
    if(type == SOCK_STREAM || type == SOCK_SEQPACKET) {
        if(listen(fd, qlen) < 0) {
            printf("listen on fd failed\n");
            goto errout;
        }
    }
    return(fd);

    errout:
    /*
     *我们想要保留引发错误的时候的errno的值，所以有必要将errno用临时变量保存
     *因为errno可能会因为别的系统调用发生错误而改变
     *所以，当被调用函数提示有错误发生时，需要立即检查errno的值，以防止被修改
     */
    err = errno;
    close(fd);
    errno = err;
    return(-1);
}
