/* public domain Simple, Minimalistic, Image processing library
 *	©2018-2020 Yuichiro Nakada
 *
 * Basic usage:
 *	imgp_gray(pixels, w, h, w, gray, w);		// 24bit -> 8bit
 *	imgp_dilate(gray, w, h, dilated);		// only 8bit
 *	imgp_absdiff(gray, dilated, w, h, diff);	// only 8bit
 *	imgp_reverse(diff, w, h, contour);		// only 8bit
 *
 *	uint8_t ahash[AHASH_SIZE*AHASH_SIZE/8];
 *	imgp_ahash(gray, w, h, ahash);		// only 8bit
 *
 *	imgp_filter(out, in, w, h, kernel, kernel_size, divisor, offset);	// only 24bit
 * */

void imgp_gray(uint8_t *s, int sx, int sy, int stride, uint8_t *p, int gstride)
{
	for (int y=0; y<sy; y++) {
		for (int x=0; x<sx; x++) {
			p[x] = 0.3 * s[x*3+0] + 0.59 * s[x*3+1] + 0.11*s[x*3+2];
		}

		s += stride*3;
		p += gstride;
	}
}

#if 1
void imgp_dilate(uint8_t *s, int w, int h, uint8_t *p)
{
	for (int y=1; y<h-1; y++) {
		for (int x=1; x<w-1; x++) {
			uint8_t uc = s[ (y+0)*w + (x+0) ]; // centre
			uint8_t ua = s[ (y-1)*w + (x+0) ]; // above
			uint8_t ub = s[ (y+1)*w + (x+0) ]; // below
			uint8_t ul = s[ (y+0)*w + (x-1) ]; // left
			uint8_t ur = s[ (y+0)*w + (x+1) ]; // right

			uint8_t ux = 0;
			if (uc > ux) ux = uc;
			if (ua > ux) ux = ua;
			if (ub > ux) ux = ub;
			if (ul > ux) ux = ul;
			if (ur > ux) ux = ur;
			p[ y*w + x ] = ux;
		}
	}
}
#else
void imgp_dilate(uint8_t *s, int w, int h, int k, uint8_t *p)
{
	int size = k;//5;
	uint8_t max, clrValue;
	int radius = size / 2;
	int ir, jr;
	/*uint8_t m = {
		{0,0,1,0,0},
		{0,1,1,1,0},
		{1,1,1,1,1},
		{0,1,1,1,0},
		{0,0,1,0,0}
	};*/

	// Loop for Columns.
	for (int colm = radius; colm < h - radius; colm++) {
		// Initialise pointers to at row start.
		uint8_t *ptr = s + (colm * w);
		uint8_t *dstPtr = p + (colm * w);

		// Loop for Row item.
		for (int row = radius; row < w - radius; row++) {
			max = 0;
			clrValue = 0;

			// Loops for element array.
			for (int eleColm = 0; eleColm < 5; eleColm++) {
				ir = eleColm - radius;
				uint8_t *tempPtr = s + ((colm + ir) * w);

				for (int eleRow = 0; eleRow < 5; eleRow++) {
					jr = eleRow - radius;

					// Get neightbour element color value.
					clrValue = (tempPtr[row * 3 + jr] + tempPtr[row * 3 + jr + 1] + tempPtr[row * 3 + jr + 2]) / 3;

					if (max < clrValue) {
						/*if (m[eleColm + eleRow*5] != 0)*/ max = clrValue;
					}
				}
			}

			dstPtr[0] = dstPtr[1] = dstPtr[2] = max;

			ptr += 3;
			dstPtr += 3;
		}
	}
}
#endif

void imgp_absdiff(uint8_t *s, uint8_t *s2, int w, int h, uint8_t *p)
{
	for (int n=0; n<w*h; n++) {
		*p++ = abs(*s++ - *s2++);
	}
}

void imgp_reverse(uint8_t *s, int w, int h, uint8_t *p)
{
	for (int n=0; n<w*h; n++) {
		*p++ = 255 - *s++;
	}
}

#ifdef STBIR_INCLUDE_STB_IMAGE_RESIZE_H
#define AHASH_SIZE	16
void imgp_ahash(uint8_t *s, int w, int h, uint8_t *ahash)
{
	uint8_t resize[AHASH_SIZE*AHASH_SIZE];
	stbir_resize_uint8(s, w, h, 0, resize, AHASH_SIZE, AHASH_SIZE, 0, 1);

	double avg = 0;
	for (int i=0; i<AHASH_SIZE*AHASH_SIZE; i++) {
		avg += resize[i];
	}
	avg /= AHASH_SIZE*AHASH_SIZE;
	//printf("avg: %f\n", avg);

	for (int i=0; i<AHASH_SIZE*AHASH_SIZE; i++) {
		ahash[i/8] <<= 1;
		if (avg <= (double)resize[i]) ahash[i/8] |= 1;
		else ahash[i/8] &= ~1;
	}

	/*for (int i=0; i<AHASH_SIZE*AHASH_SIZE/8; i++) {
		printf("%02x", ahash[i]);
	}
	printf("\n");*/
}
#endif

void imgp_filter(uint8_t *o, uint8_t *im, int w, int h, double *K, int Ks, double divisor, double offset)
{
	unsigned int ix, iy, x, y;
	int kx, ky;
	double r, g, b, p;

	for (ix=0; ix<w; ix++) {
		for (iy=0; iy<h; iy++) {
			r = g = b = 0.0;
			for (kx=-Ks; kx<=Ks; kx++) {
				for (ky=-Ks; ky<=Ks; ky++) {
					x = ix+kx;
					y = iy+ky;
					p = ((x<0) || (x>=w) || (y<0) || (y>=h)) ? 0 : im[(ix+kx + (iy+ky)*w)*3];
					r += (K[(kx+Ks) + (ky+Ks)*(2*Ks+1)]/divisor) * p + offset;
					p = ((x<0) || (x>=w) || (y<0) || (y>=h)) ? 0 : im[(ix+kx + (iy+ky)*w)*3 +1];
					g += (K[(kx+Ks) + (ky+Ks)*(2*Ks+1)]/divisor) * p + offset;
					p = ((x<0) || (x>=w) || (y<0) || (y>=h)) ? 0 : im[(ix+kx + (iy+ky)*w)*3 +2];
					b += (K[(kx+Ks) + (ky+Ks)*(2*Ks+1)]/divisor) * p + offset;
				}
			}
			r = (r>255.0) ? 255.0 : ((r<0.0) ? 0.0 : r);
			g = (g>255.0) ? 255.0 : ((g<0.0) ? 0.0 : g);
			b = (b>255.0) ? 255.0 : ((b<0.0) ? 0.0 : b);
			o[(ix + iy*w)*3] = r;
			o[(ix + iy*w)*3 +1] = g;
			o[(ix + iy*w)*3 +2] = b;
		}
	}
}
