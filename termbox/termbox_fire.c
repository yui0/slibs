// gcc termbox_fire.c -o termbox_fire
#include "termbox.h"

struct term_buf {
	uint16_t width;
	uint16_t height;
	uint16_t init_width;
	uint16_t init_height;

	char* info_line;
	uint16_t labels_max_len;
	uint16_t box_x;
	uint16_t box_y;
	uint16_t box_width;
	uint16_t box_height;

	uint8_t* buf;
};

#define FIRE_STEPS 13
static void fire_init(struct term_buf* buf)
{
	buf->init_width = buf->width;
	buf->init_height = buf->height;

	uint16_t tmp_len = buf->width * buf->height;
	buf->buf = malloc(tmp_len);
	tmp_len -= buf->width;

	if (buf->buf == NULL) {
		return;        //dgn_throw(DGN_ALLOC);
	}

	memset(buf->buf, 0, tmp_len);
	memset(buf->buf + tmp_len, FIRE_STEPS - 1, buf->width);
}

static void fire_free(struct term_buf* buf)
{
	free(buf->buf);
}

static void fire(struct term_buf* term_buf)
{
/*	static struct tb_cell fire[FIRE_STEPS] = {
		{' ', 9, 0}, // default
		{0x2591, 2, 0}, // red
		{0x2592, 2, 0}, // red
		{0x2593, 2, 0}, // red
		{0x2588, 2, 0}, // red
		{0x2591, 4, 2}, // yellow
		{0x2592, 4, 2}, // yellow
		{0x2593, 4, 2}, // yellow
		{0x2588, 4, 2}, // yellow
		{0x2591, 8, 4}, // white
		{0x2592, 8, 4}, // white
		{0x2593, 8, 4}, // white
		{0x2588, 8, 4}, // white
	};*/
	static struct tb_cell fire[FIRE_STEPS] = {
		{' ', 16, 0}, // default
		{0x2591, 5*6*6, 0}, // red
		{0x2592, 5*6*6, 0}, // red
		{0x2593, 5*6*6, 0}, // red
		{0x2588, 5*6*6, 0}, // red
		{0x2591, 5*6*6 +5*6, 5*6*6}, // yellow
		{0x2592, 5*6*6 +5*6, 5*6*6}, // yellow
		{0x2593, 5*6*6 +5*6, 5*6*6}, // yellow
		{0x2588, 5*6*6 +5*6, 5*6*6}, // yellow
		{0x2591, 5*6*6 +5*6 +5, 5*6*6 +5*6}, // white
		{0x2592, 5*6*6 +5*6 +5, 5*6*6 +5*6}, // white
		{0x2593, 5*6*6 +5*6 +5, 5*6*6 +5*6}, // white
		{0x2588, 5*6*6 +5*6 +5, 5*6*6 +5*6}, // white
	};

	uint16_t src;
	uint16_t random;
	uint16_t dst;

	uint16_t w = term_buf->init_width;
	uint8_t* tmp = term_buf->buf;

	if ((term_buf->width != term_buf->init_width) || (term_buf->height != term_buf->init_height)) {
		return;
	}

	struct tb_cell* buf = tb_cell_buffer();

	for (uint16_t x = 0; x < w; ++x) {
		for (uint16_t y = 1; y < term_buf->init_height; ++y) {
			src = y * w + x;
			random = ((rand() % 7) & 3);
			dst = src - random + 1;

			if (w > dst) {
				dst = 0;
			} else {
				dst -= w;
			}

			tmp[dst] = tmp[src] - (random & 1);

			if (tmp[dst] > 12) {
				tmp[dst] = 0;
			}

			buf[dst] = fire[tmp[dst]];
			buf[src] = fire[tmp[src]];
		}
	}
}

int main()
{
	tb_init();
//	tb_select_output_mode(TB_OUTPUT_NORMAL);
	tb_select_output_mode(TB_OUTPUT_256);
	tb_clear();

	struct term_buf buf;
	buf.width = tb_width();
	buf.height = tb_height();

	fire_init(&buf);
	while (1) {
		struct tb_event ev;
		int t = tb_peek_event(&ev, 10);

		if (t == -1) {
			break;
		}
		if (t == TB_EVENT_KEY) {
			break;
		}

		tb_clear();
		fire(&buf);
//		tb_print(33, 1, TB_MAGENTA | TB_BOLD, TB_DEFAULT, "Fire demo!");
		tb_present();
	}
	fire_free(&buf);

	tb_shutdown();
	return 0;
}
