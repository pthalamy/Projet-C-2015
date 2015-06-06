
#include "utils.h"

void *smalloc(size_t size)
{
    void *v = malloc(size);
    if (!v) {
       fprintf(stderr, "out of mem\n");
       exit(EXIT_FAILURE);
    }

    return v;
}
