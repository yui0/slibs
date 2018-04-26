/* public domain Simple, Minimalistic, random function
 *	©2017 Yuichiro Nakada
 * */

#ifndef RANDOM_H
#define RANDOM_H

#define XOR128_MAX	18446744073709551615.0

typedef unsigned long int	uint64_t;

// The state must be seeded so that it is not everywhere zero.
uint64_t xor128_seed[2];

void xor128_init(unsigned int s)
{
	for (int i=1; i<=2; i++) {
		xor128_seed[i-1] = s = 1812433253U * ( s ^ ( s >> 30 ) ) + i;
	}
}

uint64_t xor128()
{
	uint64_t s1 = xor128_seed[0];
	const uint64_t s0 = xor128_seed[1];
	xor128_seed[0] = s0;
	s1 ^= s1 << 23;
	return ( xor128_seed[1] = ( s1 ^ s0 ^ ( s1 >> 17 ) ^ ( s0 >> 26 ) ) ) + s0;
}

#define frand()		( xor128() / ((double)XOR128_MAX + 1.0f) )

#endif
