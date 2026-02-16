// l_ethernet_bench.c
//
// Measure nanoseconds spent inside the kernel for a single non-blocking packet read.
// Uses AF_PACKET/SOCK_RAW and recvfrom(MSG_DONTWAIT). Times only the syscall path.
//
// Build:
//   gcc l_ethernet_bench.c -o l_ethernet_bench
//
// Run (needs root or CAP_NET_RAW):
//   sudo ./ethernet_bench eth0 -n 2000000 -b 2048
//
// Notes:
// - This measures recvfrom() latency (kernel entry->return) for a non-blocking socket.
// - "packet" means a successful recvfrom() returning >0 bytes.
// - "no packet" means EAGAIN/EWOULDBLOCK (nothing available).
// - Other errors are counted separately and excluded from success/empty stats by default.

#define _GNU_SOURCE
#include <inttypes.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static void pin_to_cpu0(void)
{
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(0, &set);
	if (sched_setaffinity(0, sizeof(set), &set) != 0)
	{
        	// Not fatal; just reduces jitter if it works.
        	perror("sched_setaffinity");
	}
}

static void try_realtime_priority(void)
{
	struct sched_param sp;
	memset(&sp, 0, sizeof(sp));
	sp.sched_priority = 1; // low RT priority
	if (sched_setscheduler(0, SCHED_FIFO, &sp) != 0)
	{
        	// Not fatal; requires CAP_SYS_NICE or root.
        	// Keep quiet unless you want to see it:
        	// perror("sched_setscheduler");
	}
}

static void try_lock_memory(void)
{
	if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0)
	{
		// Not fatal; reduces major jitter if it works.
		// perror("mlockall");
	}
}

static void usage(const char *prog)
{
	fprintf(stderr,
		"Usage: %s <ifname> [-n iterations] [-b buflen] [-w warmup]\n"
		"  <ifname>         Interface name (e.g., eth0)\n"
		"  -n iterations    Total recvfrom() attempts (default 1000000)\n"
		"  -b buflen        Receive buffer size (default 2048)\n",
		prog);
	exit(2);
}

int main(int argc, char **argv)
{
	struct timespec start, end;
	long long total_ns = 0;
	if (argc < 2)
	{
		usage(argv[0]);
	}
	const char *ifname = argv[1];

	uint64_t iterations = 1000000;
	uint64_t bytes = 0;
	size_t buflen = 2048;

	printf("Iterations: %d\n", (int)iterations);

	for (int i = 2; i < argc; i++)
	{
		if (!strcmp(argv[i], "-n") && i + 1 < argc)
		{
			iterations = strtoull(argv[++i], NULL, 10);
		}
		else if (!strcmp(argv[i], "-b") && i + 1 < argc)
		{
			buflen = (size_t)strtoull(argv[++i], NULL, 10);
		}
		else
		{
			usage(argv[0]);
		}
	}

	// Process tuning to reduce jitter.
	pin_to_cpu0();
	try_realtime_priority();
	try_lock_memory();

	// Allocate and fault-in receive buffer to avoid first-touch page faults during timing.
	uint8_t *buf = NULL;
	if (posix_memalign((void **)&buf, 64, buflen) != 0 || !buf)
	{
		fprintf(stderr, "posix_memalign failed\n");
		return 1;
	}
	memset(buf, 0, buflen);

	// Open raw packet socket (L2). Requires root/CAP_NET_RAW.
	int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd < 0)
	{
		perror("socket(AF_PACKET,SOCK_RAW)");
		fprintf(stderr, "Need root or CAP_NET_RAW.\n");
		return 1;
	}

	// Bind to interface.
	unsigned int ifindex = if_nametoindex(ifname);
	if (ifindex == 0)
	{
		perror("if_nametoindex");
		return 1;
	}

	struct sockaddr_ll sll;
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETH_P_ALL);
	sll.sll_ifindex = (int)ifindex;

	if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) != 0)
	{
		perror("bind(AF_PACKET)");
		return 1;
	}

	// Optional: increase socket rcvbuf to reduce drops (not strictly required for timing).
	// Keep modest to avoid privilege failures; ignore errors.
	int rcvbuf = 4 * 1024 * 1024;
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

	struct sockaddr_ll from;
	socklen_t fromlen = sizeof(from);

	// Record start time
	clock_gettime(CLOCK_MONOTONIC, &start);

	// Main loop: recvfrom() with MSG_DONTWAIT. Measure only syscall.
	for (uint64_t i = 0; i < iterations; i++)
	{
		fromlen = sizeof(from);
		ssize_t r = recvfrom(fd, buf, buflen, MSG_DONTWAIT, (struct sockaddr *)&from, &fromlen);
		if (r > 0)
		{
			bytes += (uint64_t)r;
		}
	}

	// Record end time
	clock_gettime(CLOCK_MONOTONIC, &end);

	// Calculate the difference in nanoseconds
	total_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);

	// Calculate average
	double avg_ns = (double)total_ns / iterations;

	// Display results
	printf("Average: %.2f ns\n", avg_ns);
	printf("Bytes received: %lu\n", bytes);

	close(fd);
	free(buf);
	return 0;
}
