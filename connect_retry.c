/*
 *�˺���ʹ����ָ�������㷨���������connectʧ�ܣ����̻�����
 *һС��ʱ�䣬ÿ��ѭ�����ߵ�ʱ�����ָ�������ӡ��ڻ���BSD��
 *�׽���ʵ���У������һ�����ӳ���ʧ�ܣ���ô��TCP�м���ʹ��
 *ͬһ���׽�������������������Ȼ��ʧ�ܡ�����һ����ʷ��������
 *Ϊ�����ӱ���������ֲ�ԡ�������connectʧ�ܺ󣬻�ر��׽���
 *���µ�ʱ�򣬴�һ���µ��׽���
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
