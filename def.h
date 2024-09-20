#ifndef DEF_H
#define DEF_H
#include <stddef.h>
#define THREAD_NUM 16
// #define DEBUG
#ifndef DEBUG
#define fprintf(...) ;
#endif
typedef int (*Function)(int);
typedef int (*BinaryFunction)(int, int);
#endif