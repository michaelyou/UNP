#include <sys/resource.h>

void daemonize(const char *cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit r1;
    struct sigaction sa;

    umask(0);

    if(getrlimit(RLIMIT_NOFILE, &r1) < 0)
        err_quit("%s: can't get file limit", cmd);

    if((pid = fork()) < 0)
        err_quit("%s: can't fork", cmd);
    else if(pid != 0)
        exit(0);
    setsid();

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGHUP, &sa, NULL) < 0)
        err_quit("%s : can't ignore SIGHUP", cmd);
    if((pid = fork()) < 0)
        err_quit("%s : can't fork", cmd);
    else if(pid != 0)
        exit(0);

    if(chdir("/") < 0)
        err_quit("%s : can't change directory /", cmd);

    if(r1.rlim_max == RLIM_INFINITY)
        r1.rlim_max = 1024;
    for(i = 0; i < r1.rlim_max; i++)
        close(i);    //close all the fd

    printf("fd0=%d, fd1=%d, fd2=%d\n", fd0, fd1, fd2);

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    printf("fd0=%d, fd1=%d, fd2=%d\n", fd0, fd1, fd2);


    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if(fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0, fd1, fd2);

        exit(1);
    }
    
}

