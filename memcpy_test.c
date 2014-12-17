#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

struct test {
    
    char *ptr;
    int a;
};

int main()
{
    struct test s1;
    char haha[] = "hello";
    s1.a = 8;
    s1.ptr = haha;

    struct test *s11 = &s1;

    struct test *s2 = malloc(sizeof(struct test));
    memcpy(s2, s11, sizeof(struct test));
    s2->a = 9;
    strncpy(s2->ptr, "world", 2);
    printf("number is %d\n", s2->a);
    printf("string is %s\n", s2->ptr);
    printf("number is %d\n", s11->a);
    printf("string is %s\n", s11->ptr);
}
