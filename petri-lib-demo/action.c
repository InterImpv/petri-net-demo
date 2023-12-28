#include "action.h"
#include <stdint.h>

int add_one(void *data)
{
    int32_t *x = (int32_t *)data;
    *x = *x + 1;

    return 0;
}

int sub_one(void *data)
{
    int32_t *x = (int32_t *)data;
    *x = *x - 1;

    return 0;
}