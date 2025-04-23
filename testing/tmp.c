#include <stdio.h>


int main(int argc, char** argv) {
    if (argc > 1) {
        printf("argc equals %d\n",argc);
        return 1;
    }
    printf("argc less than 1\n");
    return 2;
}
