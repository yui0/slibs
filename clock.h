/* public domain Simple, Minimalistic, clock library
 *	©2017 Yuichiro Nakada
 * */

#define ANSI_RESET		"\x1b[0m"
#define ANSI_BLACK		"\033[30m"
#define ANSI_RED		"\x1b[31m"
#define ANSI_GREEN		"\x1b[32m"
#define ANSI_YELLOW		"\x1b[33m"
#define ANSI_BLUE		"\x1b[34m"
#define ANSI_MAGENTA		"\x1b[35m"
#define ANSI_CYAN		"\x1b[36m"
#define ANSI_WHITE		"\033[37m"
#define ANSI_BOLD_BLACK		"\033[1m\033[30m"
#define ANSI_BOLD_RED		"\033[1m\x1b[31m"
#define ANSI_BOLD_GREEN		"\033[1m\x1b[32m"
#define ANSI_BOLD_YELLOW	"\033[1m\x1b[33m"
#define ANSI_BOLD_BLUE		"\033[1m\x1b[34m"
#define ANSI_BOLD_MAGENTA	"\033[1m\x1b[35m"
#define ANSI_BOLD_CYAN		"\033[1m\x1b[36m"
#define ANSI_BOLD_WHITE		"\033[1m\033[37m"

#ifdef _WIN32
#define clock_start()
#define clock_end()
#else

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

struct timeval __t0;
clock_t __startClock;

void clock_start()
{
	gettimeofday(&__t0, NULL);
	__startClock = clock();
}

void clock_end()
{
	struct timeval t1;
	clock_t endClock;

	gettimeofday(&t1, NULL);
	endClock = clock();
	time_t diffsec = difftime(t1.tv_sec, __t0.tv_sec);
	suseconds_t diffsub = t1.tv_usec - __t0.tv_usec;
	double realsec = diffsec+diffsub*1e-6;
	double cpusec = (endClock - __startClock)/(double)CLOCKS_PER_SEC;
	double percent = 100.0*cpusec/realsec;
	printf(ANSI_BOLD_MAGENTA "Time spent on GPU: %f\n" ANSI_RESET, realsec);
	printf(ANSI_BOLD_GREEN "CPU utilization: %f\n" ANSI_RESET, cpusec);
}

#endif
