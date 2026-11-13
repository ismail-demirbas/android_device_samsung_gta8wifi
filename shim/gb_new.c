#include <stdlib.h>

void *_Znw0(unsigned int n)
{
    return malloc((n == 0x98u || n == 0xa0u) ? 0x6f8u : n);
}
