extern int main();

extern char __bss_start;
extern char __bss_stop;

static void zero_bss(void);

__attribute__((naked)) void _start(void)
{
	__asm__ volatile (
		"pushq %%rbp\n\t"        /* save rbp (callee-saved)     */
		"movq %%rsp, %%rbp\n\t"  /* remember original RSP       */
		"andq $-16, %%rsp\n\t"   /* ensure 16-byte alignment    */
		"call _start_c\n\t"      /* CALL so RSP is 8-mod-16 inside _start_c */
		"movq %%rbp, %%rsp\n\t"  /* restore original RSP        */
		"popq %%rbp\n\t"         /* restore rbp                 */
		"ret\n\t"                /* return to BareMetal OS       */
		::: "memory"
	);
}

static void _start_c(void)
{
	zero_bss();

	int retval = main();
}

static void zero_bss(void)
{
	for (char *c = &__bss_start; c < &__bss_stop; c++)
		*c = 0;
}
