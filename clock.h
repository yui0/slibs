/* public domain Simple, Minimalistic, clock library
 *	©2017 Yuichiro Nakada
 * */

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
	printf("Time spent on GPU: %f\n", realsec);
	printf("CPU utilization: %f\n", cpusec);
}
