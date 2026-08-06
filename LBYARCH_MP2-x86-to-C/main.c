// main.c
// MP2 - Dot Product Kernel (x86 to C)
// calls both versions of the kernel, times them, then check for match

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h> // need for QueryPerformanceCounter
#define NUM_RUNS 20

// my PC cant hadle 2^30
// using 2^28 instead
#define NUM_SIZES 3
static const long long testSizes[NUM_SIZES] = {
    1LL << 20,
    1LL << 24,
    1LL << 28
};

// C version of the kernel
// Compare with the the kernel
double dot_product_c(double* A, double* B, long long n) {
    double sdot = 0.0;
    for (long long i = 0; i < n; i++) {
        sdot += A[i] * B[i];
    }
    return sdot;
}

// asm version in kernel.asm
extern double dot_product_asm(double* A, double* B, long long n);

// fill vectors with random numbers, not part of the timed kernel
void init_vectors(double* A, double* B, long long n) {
    for (long long i = 0; i < n; i++) {
        A[i] = (double)(rand() % 100) / 10.0;
        B[i] = (double)(rand() % 100) / 10.0;
    }
}

int main(void) {
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);

    srand((unsigned int)time(NULL));

    printf("Dot Product Kernel Benchmark (C vs x86-64)\n\n");

    for (int s = 0; s < NUM_SIZES; s++) {
        long long n = testSizes[s];
        int exponent = (s == 0) ? 20 : (s == 1) ? 24 : 28;

        printf("-------------------\n");
        printf("Vector size n = %lld (2^%d)\n", n, exponent);

        double* A = (double*)malloc(n * sizeof(double));
        double* B = (double*)malloc(n * sizeof(double));

        if (A == NULL || B == NULL) {
            printf("malloc failed for n = %lld, skipping\n", n);
            free(A);
            free(B);
            continue;
        }

        init_vectors(A, B, n);

        // time C kernel, run it 20x and get avg
        double sdot_c = 0.0;
        double totalTime_c = 0.0;

        for (int run = 0; run < NUM_RUNS; run++) {
            QueryPerformanceCounter(&start);
            sdot_c = dot_product_c(A, B, n); // TIME THIS ONLY
            QueryPerformanceCounter(&end);

            totalTime_c += (double)(end.QuadPart - start.QuadPart) * 1000.0 / (double)freq.QuadPart;
        }
        double avgTime_c = totalTime_c / NUM_RUNS;

        // time asm kernel the same way
        double sdot_asm = 0.0;
        double totalTime_asm = 0.0;

        for (int run = 0; run < NUM_RUNS; run++) {
            QueryPerformanceCounter(&start);
            sdot_asm = dot_product_asm(A, B, n);
            QueryPerformanceCounter(&end);

            totalTime_asm += (double)(end.QuadPart - start.QuadPart) * 1000.0 / (double)freq.QuadPart;
        }
        double avgTime_asm = totalTime_asm / NUM_RUNS;

        // asm result should = c result
        // using a tiny epsilon instead of == because floating point rounding
        double diff = sdot_c - sdot_asm;
        if (diff < 0) diff = -diff;

        double sdot_c_abs = sdot_c;
        if (sdot_c_abs < 0) sdot_c_abs = -sdot_c_abs;
        double epsilon = 1e-6 * (sdot_c == 0.0 ? 1.0 : sdot_c_abs);

        int correct = (diff <= epsilon);

        printf("  [C]   sdot = %.6f   avg time = %.6f ms\n", sdot_c, avgTime_c);
        printf("  [ASM] sdot = %.6f   avg time = %.6f ms\n", sdot_asm, avgTime_asm);
        printf("  Correctness check (ASM vs C): %s\n", correct ? "PASS" : "FAIL");
        printf("  Speedup (C time / ASM time): %.2fx\n", avgTime_c / avgTime_asm);
        printf("\n");

        free(A);
        free(B);
    }

    printf("===== Done =====\n");
    return 0;
}