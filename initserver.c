/*
 *���������setsockopt������fd�������ã���ô�˺������޷�����������
 *ͨ������£����ǳ�ʱ(��ʱʱ��һ���Ǽ�����)������TCP��ʵ�ֲ������
 *ͬһ����ַ������ʹ���׽���ѡ��SO_REUSEADDR�����ƹ���һ����
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
     *������Ҫ�������������ʱ���errno��ֵ�������б�Ҫ��errno����ʱ��������
     *��Ϊerrno���ܻ���Ϊ���ϵͳ���÷���������ı�
     *���ԣ��������ú�����ʾ�д�����ʱ����Ҫ�������errno��ֵ���Է�ֹ���޸�
     */
    err = errno;
    close(fd);
    errno = err;
    return(-1);
}
