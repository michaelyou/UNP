#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>


int main()
{
    char buf[100];
    strcpy(buf, "hello\0\nworld");
    write(STDOUT_FILENO, buf, strlen(buf));
}

