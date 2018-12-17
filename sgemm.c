// clang sgemm.c -o sgemm -mavx
/*
$ ./sgemm 
1.000 2.000 
3.000 4.000 
5.000 6.000 
x
1.000 2.000 3.000 4.000 
5.000 6.000 7.000 8.000 
=
11.000 14.000 17.000 20.000 
23.000 30.000 37.000 44.000 
35.000 46.000 57.000 68.000 
*/
#include <stdio.h>
#include "sgemm_sse.h"

void print_matrix(float *data, int row, int col)
{
	for (int i=0; i<row; i++) {
		for (int j=0; j<col; j++) {
			printf("%.3f ", data[i*col + j]);
		}
		printf("\n");
	}
}

int main()
{
	float A[6] = {1.0,2.0,  3.0,4.0,  5.0,6.0};
	float B[8] = {1.0,2.0,3.0,4.0,  5.0,6.0,7.0,8.0};
	float C[12];
	int M = 3, N = 4, K = 2;
	double alpha = 1.0, beta = 0.0;

	print_matrix(A, M, K);
	printf("x\n");
	print_matrix(B, K, N);
	printf("=\n");

	sgemm_sse('R', 'N', 'N', M, N, K, alpha, A, K, B, N, beta, C, N);

	print_matrix(C, M, N);

	return 0;
}
