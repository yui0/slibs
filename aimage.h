// berry-dm
// Copyright © 2015-2022 Yuichiro Nakada

#ifndef uint8_t
typedef unsigned char 	uint8_t;
#endif

// for Animation Gif
typedef struct gif_result_t {
	int delay;
	uint8_t *data;
	struct gif_result_t *next;
} gif_result;

STBIDEF uint8_t *stbi_xload(char const *filename, int *x, int *y, int *frames)
{
	FILE *f;
	stbi__context s;
	uint8_t *result = 0;

	if (!(f = stbi__fopen(filename, "rb"))) {
		return stbi__errpuc("can't fopen", "Unable to open file");
	}

	stbi__start_file(&s, f);

	if (stbi__gif_test(&s)) {
		int c;
//		result = stbi__load_gif_main(&s, 0, x, y, frames, &c, 4);
		stbi__gif g;
		stbi_uc *data = 0;

		memset(&g, 0, sizeof(g)); // important!!
		*frames = 0;

		while ((data = stbi__gif_load_next(&s, &g, &c, 4, 0))) {
			if (data == (uint8_t*)&s) {
				// end of animated gif marker
				data = 0;
				break;
			}

			++(*frames);

			unsigned int size = 4 * g.w * g.h;
			uint8_t *pp = realloc(result, (*frames) * (size + 2));
			if (!pp) {
				printf("err at realloc!!\n");
				return 0;
			}
			result = pp;
			uint8_t *p = result + ((*frames)-1) * (size + 2);
			memcpy(p, data, size);
			p += size;
			*p++ = g.delay & 0xFF;
			*p++ = (g.delay & 0xFF00) >> 8;
		}
		STBI_FREE(data);

		if (*frames > 0) {
			*x = g.w;
			*y = g.h;
		}
	} else {
		stbi__result_info ri;
		result = stbi__load_main(&s, x, y, frames, 4, &ri, 0);
		*frames = !!result;
	}

	fclose(f);
	return result;
}

// 拡張パレット - RBGの変換テーブル
int pal2rgb[256][3] = {
	{0,0,0},      {192,0,0},    {0,192,0},    {192,192,0},  
	{64,64,192},  {192,0,192},  {0,192,192},  {192,192,192},
	{64,64,64},   {255,0,0},    {0,255,0},    {255,255,0},  
	{128,128,255},{255,0,255},  {0,255,255},  {255,255,255},
	{0,0,0},      {0,0,95},     {0,0,135},    {0,0,175},    
	{0,0,215},    {0,0,255},    {0,95,0},     {0,95,95},    
	{0,95,135},   {0,95,175},   {0,95,215},   {0,95,255},   
	{0,135,0},    {0,135,95},   {0,135,135},  {0,135,175},  
	{0,135,215},  {0,135,255},  {0,175,0},    {0,175,95},   
	{0,175,135},  {0,175,175},  {0,175,215},  {0,175,255},  
	{0,215,0},    {0,215,95},   {0,215,135},  {0,215,175},  
	{0,215,215},  {0,215,255},  {0,255,0},    {0,255,95},   
	{0,255,135},  {0,255,175},  {0,255,215},  {0,255,255},  
	{95,0,0},     {95,0,95},    {95,0,135},   {95,0,175},   
	{95,0,215},   {95,0,255},   {95,95,0},    {95,95,95},   
	{95,95,135},  {95,95,175},  {95,95,215},  {95,95,255},  
	{95,135,0},   {95,135,95},  {95,135,135}, {95,135,175}, 
	{95,135,215}, {95,135,255}, {95,175,0},   {95,175,95},  
	{95,175,135}, {95,175,175}, {95,175,215}, {95,175,255}, 
	{95,215,0},   {95,215,95},  {95,215,135}, {95,215,175}, 
	{95,215,215}, {95,215,255}, {95,255,0},   {95,255,95},  
	{95,255,135}, {95,255,175}, {95,255,215}, {95,255,255}, 
	{135,0,0},    {135,0,95},   {135,0,135},  {135,0,175},  
	{135,0,215},  {135,0,255},  {135,95,0},   {135,95,95},  
	{135,95,135}, {135,95,175}, {135,95,215}, {135,95,255}, 
	{135,135,0},  {135,135,95}, {135,135,135},{135,135,175},
	{135,135,215},{135,135,255},{135,175,0},  {135,175,95}, 
	{135,175,135},{135,175,175},{135,175,215},{135,175,255},
	{135,215,0},  {135,215,95}, {135,215,135},{135,215,175},
	{135,215,215},{135,215,255},{135,255,0},  {135,255,95}, 
	{135,255,135},{135,255,175},{135,255,215},{135,255,255},
	{175,0,0},    {175,0,95},   {175,0,135},  {175,0,175},  
	{175,0,215},  {175,0,255},  {175,95,0},   {175,95,95},  
	{175,95,135}, {175,95,175}, {175,95,215}, {175,95,255}, 
	{175,135,0},  {175,135,95}, {175,135,135},{175,135,175},
	{175,135,215},{175,135,255},{175,175,0},  {175,175,95}, 
	{175,175,135},{175,175,175},{175,175,215},{175,175,255},
	{175,215,0},  {175,215,95}, {175,215,135},{175,215,175},
	{175,215,215},{175,215,255},{175,255,0},  {175,255,95}, 
	{175,255,135},{175,255,175},{175,255,215},{175,255,255},
	{215,0,0},    {215,0,95},   {215,0,135},  {215,0,175},  
	{215,0,215},  {215,0,255},  {215,95,0},   {215,95,95},  
	{215,95,135}, {215,95,175}, {215,95,215}, {215,95,255}, 
	{215,135,0},  {215,135,95}, {215,135,135},{215,135,175},
	{215,135,215},{215,135,255},{215,175,0},  {215,175,95}, 
	{215,175,135},{215,175,175},{215,175,215},{215,175,255},
	{215,215,0},  {215,215,95}, {215,215,135},{215,215,175},
	{215,215,215},{215,215,255},{215,255,0},  {215,255,95}, 
	{215,255,135},{215,255,175},{215,255,215},{215,255,255},
	{255,0,0},    {255,0,95},   {255,0,135},  {255,0,175},  
	{255,0,215},  {255,0,255},  {255,95,0},   {255,95,95},  
	{255,95,135}, {255,95,175}, {255,95,215}, {255,95,255}, 
	{255,135,0},  {255,135,95}, {255,135,135},{255,135,175},
	{255,135,215},{255,135,255},{255,175,0},  {255,175,95}, 
	{255,175,135},{255,175,175},{255,175,215},{255,175,255},
	{255,215,0},  {255,215,95}, {255,215,135},{255,215,175},
	{255,215,215},{255,215,255},{255,255,0},  {255,255,95}, 
	{255,255,135},{255,255,175},{255,255,215},{255,255,255},
	{8,8,8},      {18,18,18},   {28,28,28},   {38,38,38},   
	{48,48,48},   {58,58,58},   {68,68,68},   {78,78,78},   
	{88,88,88},   {98,98,98},   {108,108,108},{118,118,118},
	{128,128,128},{138,138,138},{148,148,148},{158,158,158},
	{168,168,168},{178,178,178},{188,188,188},{198,198,198},
	{208,208,208},{218,218,218},{228,228,228},{238,238,238},
};

// 三次空間から距離を得る
int distance(int r0, int g0, int b0, int r1, int g1, int b1)
{
	int r, g, b;
	r = abs(r0 - r1);
	g = abs(g0 - g1);
	b = abs(b0 - b1);
	return sqrt(r * r + g * g + b * b);
}

// RGB から 拡張カラーへの近似色を探す
int near(int r0, int g0, int b0)
{
	int dmin = 1000; // > 441.672
	int d[256] = {0};
	for (int i=0; i<256; i++) {
		d[i] = distance(r0, g0, b0, pal2rgb[i][0], pal2rgb[i][1], pal2rgb[i][2]);
		if (dmin > d[i]) dmin = d[i];
	}
	for (int i=0; i<256; i++) {
		if (d[i] == dmin) return i;
	}
	return 255;
}

#if 0
void putImage(uint8_t *pix, int width, int height, int rx, int ry, int sx, int sy)
{
	/*float rx = (float)width / sx;
	float ry = (float)width / sy;
	printf("%f,%f\n", rx, ry);*/

	// カラーコード rgb = 000:black 001:blue 010:green 011:cyan 100:red 101:magenta 110:yellow 111:white
//	char clrcode[8] = {'0', '4', '2', '6', '1', '5', '3', '7'};

	for (int y=0; y</*sy*/height/ry; y++) {
		for (int x=0; x</*sx*/width/rx; x++) {
			int r, g, b, s;
			int clr;

			// 1文字で表される文のピクセルのRGB値の平均を求める
			int p = y*ry*width + x*rx;
			r = g = b = 0;
			for (int m=0; m<ry; m++) {
				for (int n=0; n<rx; n++) {
					r += pix[(p + m*width + n)*4];
					g += pix[(p + m*width + n)*4+1];
					b += pix[(p + m*width + n)*4+2];
				}
			}
			// 平均
			s = ry * rx;
			r = r / s;
			g = g / s;
			b = b / s;
			/*if (color256) {
				if (fullcolor) {
					// 拡張 2
					printf("\x1b[0;48;2;%2u:%2u:%2um ", r, g, b);
				} else*/ {
					// 拡張 5
					clr = near(r, g, b);
					printf("\x1b[0;48;5;%um ", clr);
				}
			/*} else {
				// RGB各値を2bit化して、RGBを3bit(8通り)で表す
				r = (r < cbmp->threshold_r) ? 0 : 1;
				g = (g < cbmp->threshold_g) ? 0 : 1;
				b = (b < cbmp->threshold_b) ? 0 : 1;
				clr = (r << 2) + (g << 1) + b;
				printf("\x1b[3%cm\x1b[4%cm%u", clrcode[clr], clrcode[clr], clr);
			}*/
		}
		printf("\x1b[39m\x1b[49m"); // デフォルトに戻す
		printf("\n");
	}
}
#endif

void simage(uint8_t *str, uint8_t *pix, int width, int height, int rx, int ry, int sx, int sy)
{
	for (int y=0; y</*sy*/height/ry; y++) {
		for (int x=0; x</*sx*/width/rx; x++) {
			int r, g, b, s;
			int clr;

			// 1文字で表される文のピクセルのRGB値の平均を求める
			int p = y*ry*width + x*rx;
			r = g = b = 0;
			for (int m=0; m<ry; m++) {
				for (int n=0; n<rx; n++) {
					r += pix[(p + m*width + n)*4];
					g += pix[(p + m*width + n)*4+1];
					b += pix[(p + m*width + n)*4+2];
				}
			}
			// 平均
			s = ry * rx;
			r = r / s;
			g = g / s;
			b = b / s;

			// 拡張 5
			clr = near(r, g, b);
			*str++ = clr;
		}
	}
}

void pimage(uint8_t *pix, int cx, int cy, int width, int height)
{
#ifdef H_TERMBOX
	for (int y=0; y<height; y++) {
		for (int x=0; x<width; x++) {
			// true color
/*			int col = *pix++;
			int c = pal2rgb[col][0]*256*256 +pal2rgb[col][1]*256 +pal2rgb[col][2];
			tb_change_cell(x, y, ' ', 0, c);*/
#ifdef USE_HALF_BLOCKS
			tb_pixel(cx+x, cy+y, *pix++); // half-blocks
#else
			tb_change_cell(cx+x, cy+y, ' ', 0, *pix++); // 256
#endif
		}
	}
#else
	printf("\033[%d;%dH", cx, cy);
	printf("\033[1;1H");
	for (int y=0; y<height; y++) {
		for (int x=0; x<width; x++) {
			printf("\x1b[0;48;5;%um ", *pix++);
		}
		printf("\x1b[39m\x1b[49m\n");
	}
#endif
}

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define ICEIL(dividend, divisor) \
	(((dividend) + ((divisor) - 1)) / (divisor))

uint8_t *aviewer_init(char *name, int sx, int sy, int *w, int *h, int *frames)
{
	uint8_t *pixels;
	int width, height;

	pixels = stbi_xload(name, &width, &height, frames);
	if (!pixels) {
		*w = *h = 0;
		return 0;
	}
	int rx = MAX(ICEIL(width, sx), 1);
	int ry = MAX(ICEIL(height, sy), 1);
//	printf("%s %dx%d r[%d,%d] / Screen %d,%d\n", name, width, height, rx, ry, sx, sy);
//	printf("frames:%d\n", *frames);

	*w = width/rx;
	*h = height/ry;
	uint8_t *screen = malloc((*frames)*(*w)*(*h));
	for (int i=0; i<*frames; i++) {
		simage(screen+i*(*w)*(*h), &pixels[width*height*4*i+2*i], width, height, rx, ry, sx, sy);
	}
	stbi_image_free(pixels);

	return screen;
}

void aviewer(char *name, int sx, int sy)
{
	/*uint8_t *pixels;
	int width, height;, frames;

	pixels = stbi_xload(name, &width, &height, &frames);
	int rx = MAX(ICEIL(width, sx), 1);
	int ry = MAX(ICEIL(height, sy), 1);
	printf("%s %dx%d r[%d,%d] / Screen %d,%d\n", name, width, height, rx, ry, sx, sy);
	printf("frames:%d\n", frames);

	uint8_t screen[frames][width/rx*height/ry];
	for (int i=0; i<frames; i++) {
		simage(screen[i], &pixels[width*height*4*i+2*i], width, height, rx, ry, sx, sy);
	}
	stbi_image_free(pixels);*/
	int w, h, frames;
	uint8_t *screen = aviewer_init(name, sx, sy, &w, &h, &frames);

	for (int i=0; i<frames; i++) {
#ifdef H_TERMBOX
		tb_clear();
#endif
//		pimage(screen[i], width/rx, height/ry);
		pimage(screen+i*w*h, 0, 0, w, h);
#ifdef H_TERMBOX
		tb_present();
#endif

		struct timespec req;
		req.tv_sec  = 0;
		req.tv_nsec = 50000000;
		nanosleep(&req, NULL);
	}
	free(screen);
}
