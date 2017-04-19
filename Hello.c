#include <stdio.h>
static int a = 15;
main() {
    /* scope check */ {
       int a;
        a = 16;
        printf("inner a == %d\n",a);
    }
    printf("global a == %d\n",a);
    printf("Hello world!\n");
}
