#ifndef THREADED_H
#define THREADED_H
#include "def.h"

#define THREAD_NUM 16
void map_threaded(Function function, int* input, int* output, size_t size);
// int apply_serial(BinaryOpFunction function, int* begin, int* end);
#endif