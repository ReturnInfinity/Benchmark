// gcc -o l_bench1 l_bench1.c -lrt
#include <stdio.h>
#include <time.h>

int main()
{
	const int iterations = 1000000;
	struct timespec start, end;
	long long total_ns = 0;

	printf("Iterations: %d\n", iterations);

	// Record start time
	clock_gettime(CLOCK_MONOTONIC, &start);

	for (int i = 0; i < iterations; i++)
	{
		//-------------------------
		/* Code to benchmark */
		//-------------------------
		// Call a kernel function
		// getpid();
		//-------------------------
		// __asm__ volatile ("nop");
		//-------------------------
		__asm__ volatile (
			"xor %%eax, %%eax;"
			"xor %%ecx, %%ecx;"
			"cpuid"
			: // output
			: // input
			: "%eax", "%ebx", "%ecx", "%edx" // clobbered registers
		);
		//-------------------------
	}

	// Record end time
	clock_gettime(CLOCK_MONOTONIC, &end);

	// Calculate the difference in nanoseconds
	total_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);

	// Calculate average
	double avg_ns = (double)total_ns / iterations;

	// Display results
	printf("Average: %.2f ns\n", avg_ns);

	return 0;
}
