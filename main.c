#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "threaded_apply.h"
#include "threaded_map.h"

int fs(int x) { return x * 2; }
int f(int x) { return (int)exp(x); }
int g(int x, int y) { return x + y; }

int gg(int x, int y) {
    for (int i = 0; i < y; i++) x = (x + 1) % 998244353;
    return x;
}

int n;
#define N 51100000
int a[N];
int ans[N];
int out[N];
int dump_testcase;

void create_test(int n) {
    for (int i = 0; i < n; i++) a[i] = rand() % 10000 + 1;
}

void map_serial(Function function, int* input, int* output, size_t size) {
    for (size_t i = 0; i < size; i++) {
        output[i] = function(input[i]);
    }
}

void map_test(int n, int max_tests) {
    for (int test_id = 1; (max_tests == -1 ? 1 : test_id <= max_tests);
         test_id++) {
        printf("Test %d:\n", test_id);
        struct timeval stop, start;
        create_test(n);
        gettimeofday(&start, NULL);
        map_serial(&f, a, ans, n);
        gettimeofday(&stop, NULL);
        unsigned long time_serial = (stop.tv_sec - start.tv_sec) * 1000000 +
                                    stop.tv_usec - start.tv_usec;
        printf("\tSerial map took %lu us\n", time_serial);
        gettimeofday(&start, NULL);
        map_threaded(&f, a, out, n);
        gettimeofday(&stop, NULL);
        unsigned long time_threaded = (stop.tv_sec - start.tv_sec) * 1000000 +
                                      stop.tv_usec - start.tv_usec;
        printf("\tThreaded map took %lu us\n", time_threaded);
        printf("\tSpeedup = %.6lfx\n", time_serial / (double)time_threaded);
        int cmp = memcmp(ans, out, n * sizeof(int));
        if (cmp != 0) {
            printf("\tWrong Answer\n");
        } else {
            printf("\tCorrect Answer\n");
        }
        sleep(1);
    }
}

int apply_serial(BinaryFunction function, int* input, size_t size) {
    int result = input[0];
    for (size_t i = 1; i < size; i++) {
        result = function(result, input[i]);
    }
    return result;
}

void apply_test(int n, int max_tests) {
    for (int test_id = 1; (max_tests == -1 ? 1 : test_id <= max_tests);
         test_id++) {
        printf("Test %d:\n", test_id);
        struct timeval stop, start;
        create_test(n);
        if (dump_testcase) {
            for (int i = 0; i < n; i++) printf("%d ", a[i]);
            printf("\n");
        }
        gettimeofday(&start, NULL);
        int apply_ans = apply_serial(&gg, a, n);
        gettimeofday(&stop, NULL);
        unsigned long time_serial = (stop.tv_sec - start.tv_sec) * 1000000 +
                                    stop.tv_usec - start.tv_usec;
        printf("\tSerial apply took %lu us\n", time_serial);
        gettimeofday(&start, NULL);
        int apply_output = apply_threaded(&gg, a, n);
        gettimeofday(&stop, NULL);
        unsigned long time_threaded = (stop.tv_sec - start.tv_sec) * 1000000 +
                                      stop.tv_usec - start.tv_usec;
        printf("\tThreaded apply took %lu us\n", time_threaded);
        printf("\tSpeedup = %.6lfx\n", time_serial / (double)time_threaded);
        if (apply_output != apply_ans) {
            printf("\tWrong Answer, read %d, expected %d\n", apply_output,
                   apply_ans);
        } else {
            printf("\tCorrect Answer\n");
        }
        sleep(1);
    }
}

int main(int argc, char* argv[]) {
    int n = 1000;
    int max_tests = -1;
    int t = 0;  // 0 = map, 1 = apply

    int opt;
    while ((opt = getopt(argc, argv, "n:m:t:s:d")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'm':
                max_tests = atoi(optarg);
                break;
            case 't':
                t = atoi(optarg);
                break;
            case 's':
                srand(atoi(optarg));
                break;
            case 'd':
                dump_testcase = 1;
                break;
            default:
                fprintf(stderr,
                        "Usage: %s -n <n> -m <max_tests> -t <test_type>\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (t == 0) {
        map_test(n, max_tests);  // Run map test if t == 0
    } else if (t == 1) {
        apply_test(n, max_tests);  // Run apply test if t == 1
    } else {
        fprintf(stderr, "Invalid test type. Use 0 for map, 1 for apply.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
