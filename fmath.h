/* public domain Simple, Minimalistic, Fast math library
 *	©2018 Yuichiro Nakada
 *
 * Basic usage:
 *	printf("sin(3.14/2.0) = %f\n", sin_fast(3.14/2.0));
 *	printf("cos(3.14/2.0) = %f\n", cos_fast(3.14/2.0));
 *	printf("log(2.7) = %f\n", log_fast(2.7));
 *	printf("exp(2.0) = %f\n", exp_fast(2.0));
 *
 * */

#define PAI	3.14159265358979323846
// http://www.geocities.jp/midarekazu/cos.pdf
double cos_fast(double x)
{
	static double div[5] = {1.0/(3*4*5*6*7*8*9*10), -1.0/(3*4*5*6*7*8), 1.0/(3*4*5*6), -1.0/(3*4), 1.0};
	double y, *p = div;
	//int q = (int)(x/(2.0*PAI));
	//x = x-q*(2.0*PAI);
	x = x/32.0;
	x = x*x;
	y = -1.0/(3*4*5*6*7*8*9*10*11*12);
	do {
		y = y*x+(*p);
		p++;
	} while (p<div+5);
	y = y*x;
	for (int i=0; i<5; i++) y = y*(4.0-y);
	return 1.0-y/2.0;
}
double sin_fast(double r) {return cos_fast(r-PAI/2);}
double tan_fast(double r) {return cos_fast(r-PAI/2)/cos_fast(r);}
#undef PAI

void sincos(double rad, double *x, double *y)
{
	static double div[8] = {-1.0/(3*4*5*6*7*8), -1.0/(2*3*4*5*6*7), 1.0/(3*4*5*6), 1.0/(2*3*4*5), -1.0/(3*4), -1.0/(2*3), 1.0, 1.0};
	double c = 1.0/(3*4*5*6*7*8*9*10), s = 1.0/(2*3*4*5*6*7*8*9), *p=div, z;
	rad = rad/32.0;
	z = rad*rad;
	do {
		c = c*z+(*p);
		p++;
		s = s*z+(*p);
		p++;
	} while (p<div+8);
	c = c*z;
	s = s*rad;
	for (int i=0; i<5; i++) {
		s = s*(2.0-c);
		c = c*(4.0-c);
	}
	*x = 1.0-c/2.0;
	*y = s;  
}

#define LOG2	0.693147180559945309417		// log(2)
double log_fast(double x)
{
	static double table[17]={
		.0                       ,  // log( 16 /16)
		.0606246218164348425806  ,  // log( 17 /16)
		.1177830356563834545387  ,  // log( 18 /16)
		.17185025692665922234    ,  // log( 19 /16)
		.2231435513142097557662  ,  // log( 20 /16)
		.2719337154836417588316  ,  // log( 21 /16)
		.3184537311185346158102  ,  // log( 22 /16)
		.3629054936893684531378  ,  // log( 23 /16)
		.405465108108164381978   ,  // log( 24 /16)
		.4462871026284195115325  ,  // log( 25 /16)
		.4855078157817008078017  ,  // log( 26 /16)
		.5232481437645478365168  ,  // log( 27 /16)
		.5596157879354226862708  ,  // log( 28 /16)
		.5947071077466927895143  ,  // log( 29 /16)
		.6286086594223741377443  ,  // log( 30 /16)
		.6613984822453650082602  ,  // log( 31 /16)
		.6931471805599453094172  ,  // log( 32 /16)
	};  
	unsigned long long w, significand16;
	int q;
	double y, h, z;
	w = *(unsigned long long*)&x;
	q = (((int)(w>>47)&0x1F)+1)>>1;
	significand16 = (w & 0xFFFFFFFFFFFFFULL)^0x4030000000000000ULL;	// 仮数*16 16<=significand16<32
	h = *(double*)&significand16;
	z = (double)(q+16);
	h = (h-z)/(h+z);
	z = h*h;
	y = (2.0/9)*z+2.0/7;
	y = y*z+2.0/5;
	y = y*z+2.0/3;
	y = y*z+2.0;
	y = y*h;
	return ((int)(w>>52)-1023)*LOG2+table[q]+y;
}
double exp_fast(double x)
{
	static double div[7] = {1.0/(2*3*4*5*6), 1.0/(2*3*4*5), 1.0/(2*3*4), 1.0/(2*3), 1.0/2, 1.0, 1.0};
	static unsigned long long table[16]={
		0x059B0D3158574ull,  // 2^( 1 /32)-1
		0x11301D0125B51ull,  // 2^( 3 /32)-1
		0x1D4873168B9AAull,  // 2^( 5 /32)-1
		0x29E9DF51FDEE1ull,  // 2^( 7 /32)-1
		0x371A7373AA9CBull,  // 2^( 9 /32)-1
		0x44E086061892Dull,  // 2^( 11 /32)-1
		0x5342B569D4F82ull,  // 2^( 13 /32)-1
		0x6247EB03A5585ull,  // 2^( 15 /32)-1
		0x71F75E8EC5F74ull,  // 2^( 17 /32)-1
		0x82589994CCE13ull,  // 2^( 19 /32)-1
		0x93737B0CDC5E5ull,  // 2^( 21 /32)-1
		0xA5503B23E255Dull,  // 2^( 23 /32)-1
		0xB7F76F2FB5E47ull,  // 2^( 25 /32)-1
		0xCB720DCEF9069ull,  // 2^( 27 /32)-1
		0xDFC97337B9B5Full,  // 2^( 29 /32)-1
		0xF50765B6E4540ull,  // 2^( 31 /32)-1
	};
	double y = 1.0/(2*3*4*5*6*7), *p = div, z, r;
	int q;
	unsigned long long w;
	z = x*(16.0/LOG2);
	q = (int)z-(x<0);
	r = x-((q<<1)+1)*(LOG2/32.0);
	w = (unsigned long long)(1023+(q>>4))<<52 ^ table[q & 0xF];
	z = *(double*)&w;
	do {
		y=y*r+(*p);
		p++;
	} while (p<div+7);
	return y*z;
}
#undef LOG2

#if 0
#include <stdio.h>
int main()
{
	printf("sin(3.14/2.0) = %f\n", sin_fast(3.14/2.0));
	printf("cos(3.14/2.0) = %f\n", cos_fast(3.14/2.0));
	printf("log(2.7) = %f\n", log_fast(2.7));
	printf("exp(2.0) = %f\n", exp_fast(2.0));
}
#endif
