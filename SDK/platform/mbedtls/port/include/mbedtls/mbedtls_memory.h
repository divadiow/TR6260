
#pragma once

#include <stdlib.h>

void *trs_mbedtls_calloc_func(size_t n, size_t size);
void trs_mbedtls_free_func(void *ptr);