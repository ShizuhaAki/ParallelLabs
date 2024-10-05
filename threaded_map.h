#ifndef THREADED_H
#define THREADED_H
#include "def.h"
void map_threaded(Function function, int* input, int* output, size_t size);
// int apply_serial(BinaryOpFunction function, int* begin, int* end);
#endif