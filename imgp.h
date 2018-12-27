/* public domain Simple, Minimalistic, Image processing library
 *	©2018 Yuichiro Nakada
 *
 * Basic usage:
 *
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
void imgp_dilate(uint8_t *s, int w, int h, int k, uint8_t *p)
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
	int size = 5;
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
