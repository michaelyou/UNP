#include "apue.h"
#include <sys/socket.h>
#include <errno.h>


int connect_retry(int domain,int type,int protocol,
                  const struct sockaddr * addr,socklen_t alen);

int initserver(int type, const struct sockaddr *addr, socklen_t alen, int qlen);

