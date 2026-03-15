/*
 * mem_benchmark.c
 * ---------------
 * Benchmarks memory bandwidth (MB/s) at increasing dataset sizes to reveal
 * CPU cache hierarchy effects (L1 → L2 → L3 → DRAM).
 *
 * Operations per iteration:
 *   1. Sequential write  – fill buffer with computed values
 *   2. Sequential read   – accumulate sum (prevents dead-code elimination)
 *   3. Copy             – src → dst (classic STREAM copy)
 *   4. Random read      – pointer-chase through shuffled index array
 *
 * Compile:  gcc -O2 -o mem_benchmark mem_benchmark.c
 * Run:      ./mem_benchmark
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* ── tunables ──────────────────────────────────────────────────────────── */
#define START_SIZE_KB   16          /* first dataset: 16 KB                */
#define END_SIZE_MB     128         /* last  dataset: 128 MB               */
#define REPEAT_SECONDS  2         /* run each test for at least 2 s    */
/* ──────────────────────────────────────────────────────────────────────── */

/* Portable high-resolution timer → seconds */
static double now_sec(void)
{
    struct timespec ts;
#ifdef CLOCK_MONOTONIC
    clock_gettime(CLOCK_MONOTONIC, &ts);
#else
    clock_gettime(CLOCK_REALTIME, &ts);
#endif
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* Fisher-Yates shuffle of an index array (for random-read test) */
static void shuffle(size_t *arr, size_t n)
{
    for (size_t i = n - 1; i > 0; --i) {
        size_t j = (size_t)rand() % (i + 1);
        size_t tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

/* ── per-size benchmark ────────────────────────────────────────────────── */
static void bench_size(size_t bytes)
{
    const size_t n = bytes / sizeof(uint64_t);   /* element count          */

    /* allocate buffers */
    uint64_t *buf  = (uint64_t *)malloc(bytes);
    uint64_t *dst  = (uint64_t *)malloc(bytes);
    size_t   *idx  = (size_t   *)malloc(n * sizeof(size_t));

    if (!buf || !dst || !idx) {
        fprintf(stderr, "  [ERROR] malloc failed for %zu bytes\n", bytes);
        free(buf); free(dst); free(idx);
        return;
    }

    /* build shuffled index array for random-read test */
    for (size_t i = 0; i < n; ++i) idx[i] = i;
    shuffle(idx, n);

    /* warm up: touch all pages so we measure cache, not page-fault time */
    memset(buf, 0, bytes);
    memset(dst, 0, bytes);

    /* ── 1. Sequential WRITE ─────────────────────────────────────────── */
    double t0, t1, elapsed;
    long   iters;
    volatile uint64_t sink = 0;   /* prevents optimising away side-effects */

    iters = 0; t0 = now_sec();
    do {
        for (size_t i = 0; i < n; ++i)
            buf[i] = (uint64_t)i ^ 0xDEADBEEFCAFEULL;
        ++iters;
        t1 = now_sec();
    } while ((t1 - t0) < REPEAT_SECONDS);
    elapsed = t1 - t0;
    double write_bw = ((double)bytes * iters) / elapsed / (1024.0 * 1024.0);

    /* ── 2. Sequential READ ──────────────────────────────────────────── */
    uint64_t acc = 0;
    iters = 0; t0 = now_sec();
    do {
        for (size_t i = 0; i < n; ++i)
            acc += buf[i];
        ++iters;
        t1 = now_sec();
    } while ((t1 - t0) < REPEAT_SECONDS);
    sink = acc;                         /* keep acc alive */
    elapsed = t1 - t0;
    double read_bw = ((double)bytes * iters) / elapsed / (1024.0 * 1024.0);

    /* ── 3. Copy (read + write) ──────────────────────────────────────── */
    iters = 0; t0 = now_sec();
    do {
        for (size_t i = 0; i < n; ++i)
            dst[i] = buf[i];
        ++iters;
        t1 = now_sec();
    } while ((t1 - t0) < REPEAT_SECONDS);
    sink ^= dst[0];
    elapsed = t1 - t0;
    /* copy touches 2× bytes (one read, one write) */
    double copy_bw = ((double)bytes * 2 * iters) / elapsed / (1024.0 * 1024.0);

    /* ── 4. Random READ (pointer-chase via shuffled indices) ─────────── */
    acc = 0;
    iters = 0; t0 = now_sec();
    do {
        for (size_t i = 0; i < n; ++i)
            acc += buf[idx[i]];
        ++iters;
        t1 = now_sec();
    } while ((t1 - t0) < REPEAT_SECONDS);
    sink ^= acc;
    elapsed = t1 - t0;
    double rand_bw = ((double)bytes * iters) / elapsed / (1024.0 * 1024.0);

    /* ── print results ───────────────────────────────────────────────── */
    /* format dataset size */
    char size_str[16];
    if (bytes >= 1024 * 1024)
        snprintf(size_str, sizeof(size_str), "%4u MiB", (unsigned int)(bytes / (1024 * 1024)));
    else
        snprintf(size_str, sizeof(size_str), "%4u KiB", (unsigned int)(bytes / 1024));

    printf("│ %-8s │ %10.0f │ %10.0f │ %10.0f │ %10.0f │\n",
           size_str, write_bw, read_bw, copy_bw, rand_bw);

    (void)sink;   /* suppress unused-variable warning */
    free(buf); free(dst); free(idx);
}

/* ── main ──────────────────────────────────────────────────────────────── */
int main(void)
{
    srand((unsigned)time(NULL));

    printf("\n");
    printf("  Memory Bandwidth Benchmark\n");
    printf("  (Higher MB/s = data fits in faster cache level)\n\n");

    printf("┌──────────┬────────────┬────────────┬────────────┬────────────┐\n");
    printf("│ Dataset  │ Seq Write  │  Seq Read  │    Copy    │ Rand Read  │\n");
    printf("│   Size   │   (MB/s)   │   (MB/s)   │   (MB/s)   │   (MB/s)   │\n");
    printf("├──────────┼────────────┼────────────┼────────────┼────────────┤\n");

    size_t size = (size_t)START_SIZE_KB * 1024;
    size_t end  = (size_t)END_SIZE_MB   * 1024 * 1024;

    while (size <= end) {
        bench_size(size);
        size *= 2;
    }

    printf("└──────────┴────────────┴────────────┴────────────┴────────────┘\n");

    printf("\n  Interpretation guide:\n");
    printf("  • Sharp drop in MB/s between two sizes → data spilled into\n");
    printf("    the next (slower) cache level or main memory.\n");
    printf("  • Seq Write ≈ Seq Read   → write-allocate policy in effect.\n");
    printf("  • Copy ≈ 2× Seq Read     → bandwidth-bound (as expected).\n");
    printf("  • Rand Read ≪ Seq Read   → cache-line prefetcher can't help;\n");
    printf("    reveals true latency-bound random-access bandwidth.\n\n");

    return 0;
}
