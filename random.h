/* public domain Simple, Minimalistic, random function
 *	©2017-2023 Yuichiro Nakada
 * */

#ifndef RANDOM_H
#define RANDOM_H

#define XOR128_MAX	18446744073709551615.0

typedef unsigned long int	uint64_t;

// The state must be seeded so that it is not everywhere zero.
uint64_t xor128_seed[2];

static void xor128_init(unsigned int s)
{
	for (int i=1; i<=2; i++) {
		xor128_seed[i-1] = s = 1812433253U * ( s ^ ( s >> 30 ) ) + i;
	}
}

static inline uint64_t xor128()
{
	uint64_t s1 = xor128_seed[0];
	const uint64_t s0 = xor128_seed[1];
	xor128_seed[0] = s0;
	s1 ^= s1 << 23;
	return ( xor128_seed[1] = ( s1 ^ s0 ^ ( s1 >> 17 ) ^ ( s0 >> 26 ) ) ) + s0;
}

// xoroshiro generator taken from http://vigna.di.unimi.it/xorshift/xoroshiro128plus.c
uint64_t xoroshiro_s[2] = {
    0X922AC4EB35B502D9L,
    0XDA3AA4832B8F1D27L
};
static void xoroshiro128plus_init(uint64_t s)
{
	for (int i=1; i<=2; i++) {
		xoroshiro_s[i-1] = s = 1812433253U * ( s ^ ( s >> 30 ) ) + i;
	}
}
static inline uint64_t rotl(const uint64_t x, int k)
{
	return (x << k) | (x >> (64 - k));
}
static uint64_t xoroshiro128plus()
{
	const uint64_t s0 = xoroshiro_s[0];
	uint64_t s1 = xoroshiro_s[1];
	const uint64_t result = s0 + s1;

	s1 ^= s0;
	xoroshiro_s[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
	xoroshiro_s[1] = rotl(s1, 36); // c
	return result;
}

// taken from https://github.com/svaarala/duktape/blob/master/misc/splitmix64.c
static uint64_t splitmix64_x; /* The state can be seeded with any value. */
static uint64_t splitmix64_next()
{
	uint64_t z = (splitmix64_x += UINT64_C(0x9E3779B97F4A7C15));
	z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	return z ^ (z >> 31);
}

// execute RDTSC instruction and return cycle counter value
static uint64_t rdtsc()
{
	uint64_t result;
	__asm__ volatile ("rdtsc" : "=A" (result));
	return result;
}
#define MATH_PI  3.14159265358979323846
#define MATH_E   2.7182818284590452354
#define CLK_TICK 16
// physical random number generator
static uint32_t prand32()
{
	uint32_t y;
	char     s1[80], s2[80];
	uint64_t start_clk;
	uint64_t stop_clk;
	unsigned rnd_bit;
	unsigned i;

	y = 0;
	for (i=0; i<32; i++) {
		start_clk = rdtsc();
		sprintf(s1, "%70.40lf", (double)y*MATH_PI);	// meaningless, waste time...
		sprintf(s2, "%lf", atof(s1)*MATH_E);		// meaningless, waste time...
		sprintf(s1, "%lf", atof(s2)/MATH_PI);		// meaningless, waste time...
		sprintf(s2, "%s", s1);				// meaningless, waste time...
		stop_clk = rdtsc();
		rnd_bit = ((stop_clk-start_clk)/CLK_TICK)%2;
		y = (y<<1)|rnd_bit;
		//printf("%u %u %u\n", (unsigned)(stop_clk-start_clk)/CLK_TICK, (unsigned)stop_clk, (unsigned)start_clk);
	}
	return y;
}

//#define CATS_USE_XOR128
#ifdef CATS_USE_XOR128
#define xrand()			xor128()
#define frand()			( xor128() / (XOR128_MAX+1.0) )
#else
#define xrand()			xoroshiro128plus()
#define frand()			( xoroshiro128plus() / (XOR128_MAX+1.0) )
#endif
#define _rand(max)		(int)( frand() * max)
#define random(min, max)	( frand() * (max -min) +min )
#define irand(min, max)		( (xrand() % (max -min +1)) +min )
// http://www.natural-science.or.jp/article/20110404234734.php (mt19937ar.h)
// https://omitakahiro.github.io/random/random_variables_generation.html

#endif
