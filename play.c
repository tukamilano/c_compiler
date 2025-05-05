#include <stdio.h>

struct point1{
    int x;
    int y;
};

struct point2{
    int x;
    int y;
    char c;
};

int main() {
    printf("%zu\n", sizeof(struct point1));
    printf("%zu\n", sizeof(struct point2));
}