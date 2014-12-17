#include <stdio.h>
#include <math.h>

void main()
{
    int i = ~0;
    if((unsigned int)i == 65535)
        printf("this is a 16bit computer\n");
    else if(unsigned int)i == (pow(2, 32) - 1))
        printf("this computer is at least 32 bit\n");
}
