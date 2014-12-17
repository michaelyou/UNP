#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <time.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int listenfd, connfd;
	struct sockaddr_in servaddr;
	char buf[4096];
	time_t ticks;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(17777);

	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	listen(listenfd, 10);

	for(;;) {
		connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
		printf("connfd is %d\n", connfd);
		ticks = time(NULL);		
		snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
		write(connfd, buf, strlen(buf));

		close(connfd);
	}
}
