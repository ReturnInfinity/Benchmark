#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define TICK_RATE 64
#define TARGET_MS (1000.0 / TICK_RATE) // 15.625ms
#define SAMPLES 10000                  // Need 10k for valid P99.9

// Comparison function for qsort
int compare(const void *a, const void *b) {
    double diff = *(double *)a - *(double *)b;
    return (diff > 0) - (diff < 0);
}

int main() {
    struct timespec start, end;
    double *latencies = malloc(SAMPLES * sizeof(double));
    
    printf("Benchmarking %d ticks at %.3fms target...\n", SAMPLES, TARGET_MS);

    for (int i = 0; i < SAMPLES; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);

        // --- Simulate Game Work + Sleep ---
        // In a real server, work happens here. 
        // We use usleep to see how precisely the OS wakes us up.
        usleep((int)(TARGET_MS * 1000)); 

        clock_gettime(CLOCK_MONOTONIC, &end);

        double elapsed = (end.tv_sec - start.tv_sec) * 1000.0 +
                         (end.tv_nsec - start.tv_nsec) / 1000000.0;
        latencies[i] = elapsed;
    }

    // Sort to find percentiles
    qsort(latencies, SAMPLES, sizeof(double), compare);

    // Calculate Metrics
    double p50 = latencies[(int)(SAMPLES * 0.50)];
    double p90 = latencies[(int)(SAMPLES * 0.90)];
    double p99 = latencies[(int)(SAMPLES * 0.99)];
    double p999 = latencies[(int)(SAMPLES * 0.999)];
    double max = latencies[SAMPLES - 1];

    printf("\n--- Results ---\n");
    printf("P50 (Median): %.3f ms\n", p50);
    printf("P90:         %.3f ms\n", p90);
    printf("P99:         %.3f ms (Worst 1 in 100)\n", p99);
    printf("P99.9:       %.3f ms (Worst 1 in 1000)\n", p999);
    printf("Max:         %.3f ms\n", max);

    if (p99 > TARGET_MS) {
        printf("\nWARNING: P99 exceeds tick budget! Players will feel lag.\n");
    }

    free(latencies);
    return 0;
}
