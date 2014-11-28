#include "apue.h"
#include <netdb.h>
#include <syslog.h>
#include <sys/socket.h>
#include "socket_api.h"
#include "error.h"
#include <fcntl.h>
#include <arpa/inet.h>
#include "daemonize.h"

#define BUFLEN 128
#define QLEN 10

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

int
set_cloexec(int fd)
{
	int		val;
    //F_GETFDȡ��close-on-exec��ꡣ
    //��������FD_CLOEXECλΪ0�������ڵ���exec()��غ���ʱ�ļ�������ر�
	if ((val = fcntl(fd, F_GETFD, 0)) < 0) 
		return(-1);

	val |= FD_CLOEXEC;		/* enable close-on-exec */

	return(fcntl(fd, F_SETFD, val));
}

extern int initserver(int type,const struct sockaddr * addr,
                      socklen_t alen,int qlen);

void serve(int sockfd)
{
    int clfd;
    FILE *fp;
    char buf[BUFLEN];
    char opt[BUFLEN];
    int n;

    set_cloexec(sockfd);
    for(;;) {
        if((clfd = accept(sockfd, NULL, NULL)) < 0) {  //�����Ŀͻ��˱�ʶ
            syslog(LOG_ERR, "ruptimed: accept error: %s", 
                   strerror(errno));
            exit(-1);
        }
        while(1) {
            if((n = recv(clfd, opt, BUFLEN, 0)) > 0) {
                printf("the string i got is %s", opt);
                //������ַ����Ƚϲ�ָ��λ���Ļ��Ƚϻ����
                //unp P8�н��ͣ�TCP��һ��û�м�¼�߽���ֽ���Э��
                if(strncmp(opt, "uptime", 6) == 0) {    
                    //�ùܵ��ķ�ʽִ��һ������
                    if((fp = popen("/usr/bin/uptime", "r")) == NULL) {
                        //��������Ϣ���͵��ͻ���
                        sprintf(buf, "error: %s\n", strerror(errno));
                        send(clfd, buf, strlen(buf), 0);
                    }
                    else {
                        while(fgets(buf, BUFLEN, fp) != NULL)  //�п���һ�η����꣬��Ҫһ��ѭ��
                            send(clfd, buf, strlen(buf), 0);
                        pclose(fp);
                    }
                    close(clfd);  //�رղ�����׽���
                    break;
                    }
                else {
                    strcpy(buf, "we don not provide the service");
                    send(clfd, buf, strlen(buf), 0);
                }
            }
            else
                continue;
        }
     }
}

int main(int argc, char *argv[])
{
    struct addrinfo *ailist, *aip;
    struct addrinfo hint;
    int sockfd, err, n;
    char *host;

    if(argc != 1)
        err_quit("usage: %s", argv[0]);
    //ȡ��ϵͳ����
    //Max length of a hostname, not including the terminating null byte
    if((n = sysconf(_SC_HOST_NAME_MAX)) < 0)
        n = HOST_NAME_MAX;
    if((host = malloc(n)) == NULL) 
        err_sys("malloc error");
    if(gethostname(host, n) < 0) 
        err_sys("gethostname error");
    //daemonize("ruptimed");

    memset(&hint, 0, sizeof(hint));
    hint.ai_flags = AI_CANONNAME;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;
    if((err = getaddrinfo(host, "ruptime", &hint, &ailist)) != 0) {
        syslog(LOG_ERR, "ruptimed: getaddrinfo error %s", 
               gai_strerror(err));  //�˺�����������getaddrinfo�����Ĵ����뷵�ص���Ϣ
        exit(1);
    }
    for(aip = ailist; aip != NULL; aip = aip->ai_next) {
        #if 0
        struct sockaddr_in *socktmp;
        socktmp = (struct sockaddr_in *)(aip->ai_addr);
        printf("the port id is %d, the address is %s\n", ntohs(socktmp->sin_port),
                inet_ntoa(socktmp->sin_addr));
        #endif
        if((sockfd = initserver(SOCK_STREAM, aip->ai_addr,
                                aip->ai_addrlen, QLEN)) >= 0) {
            serve(sockfd);
            exit(0);
        }
        
    }
    exit(0);
}
