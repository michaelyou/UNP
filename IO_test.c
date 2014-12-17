#include <stdio.h>
#include <unistd.h>
int main(void)
{
    printf("call execl"); 
    sleep(1); 
    execl("/bin/ls", "", NULL); 
    printf("error!\n");
}

