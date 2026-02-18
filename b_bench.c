#include "libBareMetal.h"

u64 string_len(char* str)
{
	int len = 0;
	while (str[len] != '\0')
		len++;
	return len;
}

void reverse(char* str)
{
	u64 len = string_len(str);
	for (int i = 0; i < len / 2; i++)
	{
		char temp = str[i];
		str[i] = str[len - i - 1];
		str[len - i - 1] = temp;
	}
}

void int_to_string(u64 num, char* str)
{
	int i = 0;
	do
	{
		str[i++] = num % 10 + '0';
		num /= 10;
	} while (num);

	str[i] = '\0';
	reverse(str);
}



void print_u64(u64 num)
{
	char str[21];
	int_to_string(num, str);
	b_output(str, string_len(str));
}

int main()
{
	const int iterations = 1000000;
	u64 start, end, total_ns, avg_ns;

	b_output("Iterations: 1000000\n", 20);

	// Record start time
	start = b_system(TIMECOUNTER, 0, 0);

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
	end = b_system(TIMECOUNTER, 0, 0);

	// Calculate the difference in nanoseconds
	total_ns = end - start;

	// Calculate average
	avg_ns = total_ns / iterations;

	// Display results
	b_output("Average: ", 9);
	print_u64(avg_ns);
	b_output (" ns", 3);

	return 0;
}
