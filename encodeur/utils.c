
#include "utils.h"

void *smalloc(size_t size)
{
    void *ptr = malloc (size);
    if (!ptr) {
       fprintf (stderr, "error: OUT OF MEMORY\n");
       exit (EXIT_FAILURE);
    }

    return ptr;
}
