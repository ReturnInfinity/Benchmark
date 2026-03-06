#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define TARGET_TICK_MS 15.62  // 64-tick budget
#define TEST_DURATION 1000    // Test for 1000 ticks

int main() {
    struct timespec start, end;
    double latencies[TEST_DURATION];
    double total_ms = 0;

    for (int i = 0; i < TEST_DURATION; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        // --- Simulate Game Logic Here ---
        usleep(10000); // Simulate 10ms of work
        // --------------------------------
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        double elapsed = (end.tv_sec - start.tv_sec) * 1000.0 +
                         (end.tv_nsec - start.tv_nsec) / 1000000.0;
        latencies[i] = elapsed;
        total_ms += elapsed;
    }

    printf("Average MSPT: %.2f ms\n", total_ms / TEST_DURATION);
    // Note: To find P99, sort 'latencies' and pick the 990th value.
    return 0;
}
