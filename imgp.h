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
 *	imgp_filter(in, w, h, out, kernel, kernel_size, divisor, offset);	// only 24bit
 *	imgp_color_quant(pixels, w, h, color);	// only 24bit
 *	imgp_cq24to15(pixels, w, h, 3, pixels, 1);
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
/*void imgp_dilate(uint8_t *s, int w, int h, uint8_t *p)
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
}*/
void imgp_dilate(uint8_t *s, int w, int h, int bpp, uint8_t *p)
{
	for (int y=1; y<h-1; y++) {
		for (int x=1; x<w-1; x++) {
			for (int b=0; b<bpp; b++) {
				uint8_t uc = s[ ((y+0)*w + (x+0))*bpp+b ]; // centre
				uint8_t ua = s[ ((y-1)*w + (x+0))*bpp+b ]; // above
				uint8_t ub = s[ ((y+1)*w + (x+0))*bpp+b ]; // below
				uint8_t ul = s[ ((y+0)*w + (x-1))*bpp+b ]; // left
				uint8_t ur = s[ ((y+0)*w + (x+1))*bpp+b ]; // right

				uint8_t ux = 0;
				if (uc > ux) ux = uc;
				if (ua > ux) ux = ua;
				if (ub > ux) ux = ub;
				if (ul > ux) ux = ul;
				if (ur > ux) ux = ur;
				p[ (y*w + x)*bpp+b ] = ux;
			}
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

/*double magic_kernel[4*4] = {
	1/64.0, 3/64.0, 3/64.0, 1/64.0,
	3/64.0, 9/64.0, 9/64.0, 3/64.0,
	3/64.0, 9/64.0, 9/64.0, 3/64.0,
	1/64.0, 3/64.0, 3/64.0, 1/64.0,
};*/
void imgp_filter(uint8_t *im, int w, int h, uint8_t *o, double *K, int Ks, double divisor, double offset)
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


// https://www.petitmonte.com/math_algorithm/subtractive_color.html
// https://github.com/kornelski/mediancut-posterizer/blob/master/posterize.c
// https://rosettacode.org/wiki/Color_quantization/C
typedef struct oct_node_t oct_node_t, *oct_node;
struct oct_node_t {
	/* sum of all colors represented by this node. 64 bit in case of HUGE image */
	uint64_t r, g, b;
	int count, heap_idx;
	oct_node kids[8], parent;
	unsigned char n_kids, kid_idx, flags, depth;
};

typedef struct {
	int alloc, n;
	oct_node* buf;
} node_heap;

/* cmp function that decides the ordering in the heap.  This is how we determine
   which octree node to fold next, the heart of the algorithm. */
int cmp_node(oct_node a, oct_node b)
{
	if (a->n_kids < b->n_kids) return -1;
	if (a->n_kids > b->n_kids) return 1;

	int ac = a->count * (1 + a->kid_idx) >> a->depth;
	int bc = b->count * (1 + b->kid_idx) >> b->depth;
	return ac < bc ? -1 : ac > bc;
}

void down_heap(node_heap *h, oct_node p)
{
	int n = p->heap_idx, m;
	while (1) {
		m = n * 2;
		if (m >= h->n) break;
		if (m + 1 < h->n && cmp_node(h->buf[m], h->buf[m + 1]) > 0) m++;

		if (cmp_node(p, h->buf[m]) <= 0) break;

		h->buf[n] = h->buf[m];
		h->buf[n]->heap_idx = n;
		n = m;
	}
	h->buf[n] = p;
	p->heap_idx = n;
}

void up_heap(node_heap *h, oct_node p)
{
	int n = p->heap_idx;
	oct_node prev;

	while (n>1) {
		prev = h->buf[n / 2];
		if (cmp_node(p, prev) >= 0) break;

		h->buf[n] = prev;
		prev->heap_idx = n;
		n /= 2;
	}
	h->buf[n] = p;
	p->heap_idx = n;
}

#define ON_INHEAP	1
void heap_add(node_heap *h, oct_node p)
{
	if ((p->flags & ON_INHEAP)) {
		down_heap(h, p);
		up_heap(h, p);
		return;
	}

	p->flags |= ON_INHEAP;
	if (!h->n) h->n = 1;
	if (h->n >= h->alloc) {
		while (h->n >= h->alloc) h->alloc += 1024;
		h->buf = realloc(h->buf, sizeof(oct_node) * h->alloc);
	}

	p->heap_idx = h->n;
	h->buf[h->n++] = p;
	up_heap(h, p);
}

oct_node pop_heap(node_heap *h)
{
	if (h->n <= 1) return 0;

	oct_node ret = h->buf[1];
	h->buf[1] = h->buf[--h->n];

	h->buf[h->n] = 0;

	h->buf[1]->heap_idx = 1;
	down_heap(h, h->buf[1]);

	return ret;
}

static oct_node oct_pool = 0;
oct_node node_new(unsigned char idx, unsigned char depth, oct_node p)
{
	static int len = 0;
	if (len <= 1) {
		oct_node p = calloc(sizeof(oct_node_t), 2048);
		p->parent = oct_pool;
		oct_pool = p;
		len = 2047;
	}

	oct_node x = oct_pool + len--;
	x->kid_idx = idx;
	x->depth = depth;
	x->parent = p;
	if (p) p->n_kids++;
	return x;
}

void node_free()
{
	oct_node p;
	while (oct_pool) {
		p = oct_pool->parent;
		free(oct_pool);
		oct_pool = p;
	}
}

/* adding a color triple to octree */
#define OCT_DEPTH 8
/* 8: number of significant bits used for tree.  It's probably good enough
   for most images to use a value of 5.  This affects how many nodes eventually
   end up in the tree and heap, thus smaller values helps with both speed
   and memory. */
oct_node node_insert(oct_node root, unsigned char *pix)
{
	unsigned char i, bit, depth = 0;
	for (bit = 1 << 7; ++depth < OCT_DEPTH; bit >>= 1) {
		i = !!(pix[1] & bit) * 4 + !!(pix[0] & bit) * 2 + !!(pix[2] & bit);
		if (!root->kids[i]) {
			root->kids[i] = node_new(i, depth, root);
		}

		root = root->kids[i];
	}

	root->r += pix[0];
	root->g += pix[1];
	root->b += pix[2];
	root->count++;
	return root;
}

/* remove a node in octree and add its count and colors to parent node. */
oct_node node_fold(oct_node p)
{
	if (p->n_kids) abort();
	oct_node q = p->parent;
	q->count += p->count;

	q->r += p->r;
	q->g += p->g;
	q->b += p->b;
	q->n_kids --;
	q->kids[p->kid_idx] = 0;
	return q;
}

/* traverse the octree just like construction, but this time we replace the pixel
   color with color stored in the tree node */
void color_replace(oct_node root, unsigned char *pix)
{
	unsigned char i, bit;

	for (bit = 1 << 7; bit; bit >>= 1) {
		i = !!(pix[1] & bit) * 4 + !!(pix[0] & bit) * 2 + !!(pix[2] & bit);
		if (!root->kids[i]) break;
		root = root->kids[i];
	}

	pix[0] = root->r;
	pix[1] = root->g;
	pix[2] = root->b;
}

/* Building an octree and keep leaf nodes in a bin heap.  Afterwards remove first node
   in heap and fold it into its parent node (which may now be added to heap), until heap
   contains required number of colors. */
void imgp_color_quant(unsigned char *im, int w, int h, int n_colors)
{
	int i;
	unsigned char *pix = im;
	node_heap heap = { 0, 0, 0 };

	oct_node root = node_new(0, 0, 0);
	for (i=0; i < w * h; i++, pix += 3) {
		heap_add(&heap, node_insert(root, pix));
	}

	while (heap.n > n_colors + 1) {
		heap_add(&heap, node_fold(pop_heap(&heap)));
	}

	/*for (i=1; i < heap.n; i++) {
		oct_node got = heap.buf[i];
		double c = got->count;
		got->r = got->r / c + .5;
		got->g = got->g / c + .5;
		got->b = got->b / c + .5;
		printf("%2d | %3lu %3lu %3lu (%d pixels)\n",
		       i, got->r, got->g, got->b, got->count);
	}*/

	for (i=0, pix = im; i < w * h; i++, pix += 3) {
		color_replace(root, pix);
	}

	node_free();
	free(heap.buf);
}

// 24bit -> 15bit
uint8_t rndunit_24to15(uint8_t u, int dither)
{
	uint8_t unit = (u & 0b11111000)>>3;
	uint8_t weight = (u & 0b00000111);
	if (dither && (weight - (rand() % 7) >=0) && (unit < 31)) {
		unit++;
	}
	return unit<<3;
}
void imgp_cq24to15(uint8_t *s, int w, int h, int bps, uint8_t *p, int dither)
{
	for (int n=0; n<w*h*bps; n++) {
		*p++ = rndunit_24to15(*s++, dither);
	}
}
