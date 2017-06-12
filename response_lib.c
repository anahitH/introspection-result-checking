#include <stdlib.h>

void response(int result)
{
    if (result == 1) {
        printf("abort\n");
        abort();
    }
}
