#include "utils.h"
#include <stdio.h>
#include <sys/types.h>

size_t get_file_size(FILE *fp)
{
    size_t ret = 0;
    if (!fp)
        return ret;

    fseek(fp, 0, SEEK_END);
    ret = ftell(fp);
    rewind(fp);

    return ret;
}

char *read_file_to_buf(FILE *fp)
{
    char *buf = NULL;
    if (!fp)
        return buf;

    /* find file size */
    size_t length = get_file_size(fp);
    if (length < 1)
        return buf;

    /* try to read everything */
    buf = (char *)malloc(length * sizeof(*buf));
    if (!buf) {
        fprintf(stderr, "allocation for file read fail\n");
        return buf;
    }
    fread(buf, length, 1, fp);

    return buf;
}