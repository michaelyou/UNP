#include <stdio.h>
#include <stdlib.h>

void strstr1(char *s, char *t)
{
    int i, j, k;
    for(i = 0; s[i] != '\0'; i++) {
        for(j = i, k = 0; t[k] != 0 && s[j] == t[k]; j++, k++);
        if(k > 0 && t[k] == '\0') {
            printf("it is a substring of the source string\n");
            exit(0);
        }
    }
    printf("no!\n");
}

int main(int argc, char **argv)
{
    if(argc != 3)
        printf("usage: %s <string><string>");
    strstr1(argv[1], argv[2]);
    return(0);
}
