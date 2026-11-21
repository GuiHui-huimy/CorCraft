#include <stdio.h>

int main(void) {
    int unused_flag = 1;
    unsigned int total = 0;
    int delta = -5;

    total = delta;
    printf("total = %u\n", total);
    return 0;
}
