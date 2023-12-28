#ifndef __UTILS_H
#define __UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

size_t get_file_size(FILE *fp);
char *read_file_to_buf(FILE *fp);

#endif
