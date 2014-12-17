#include <stdio.h>

void main()
{
    int i = sizeof(size_t);
    switch(i) {
        case 4:
            printf("this computer is 32 bit\n");
            break;
        case 8:
            printf("this computer is 64 bit\n");
            break;
        default:
            printf("other\n");
            break;
    }
}
