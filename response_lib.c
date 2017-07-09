#include <stdlib.h>

void response(int result)
{
    if (result == 0) {
	printf("check failred\n");
        abort();
    } else {
      printf("checks passed\n");
    }
}
