#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include <signal.h>

void sig_int(int signo)
{
    //void pr_cpu_time(void);

    printf("got a signal\n");

    exit(0);
}


int main()
{
    struct sigaction act;
    act.sa_handler = sig_int;
    act.sa_flags = 0;
    if(sigaction(SIGINT, &act, NULL))
        perror("sigaction\n");
    for(;;)
     ;
}

