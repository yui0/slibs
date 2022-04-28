// gcc termbox_fire.c -o termbox_fire
#include "termbox.h"

struct box
{
	uint32_t left_up;
	uint32_t left_down;
	uint32_t right_up;
	uint32_t right_down;
	uint32_t top;
	uint32_t bot;
	uint32_t left;
	uint32_t right;
};

struct matrix_dot
{
	int val;
	bool is_head;
};

struct matrix_state
{
	struct matrix_dot** grid;
	int* length;
	int* spaces;
	int* updates;
};

struct doom_state
{
	uint8_t* buf;
};

union anim_state
{
	struct doom_state* doom;
	struct matrix_state* matrix;
};

struct term_buf
{
	uint16_t width;
	uint16_t height;
	uint16_t init_width;
	uint16_t init_height;

	struct box box_chars;
	char* info_line;
	uint16_t labels_max_len;
	uint16_t box_x;
	uint16_t box_y;
	uint16_t box_width;
	uint16_t box_height;

	union anim_state astate;
};

#define DOOM_STEPS 13
static void doom_init(struct term_buf* buf)
{
	buf->init_width = buf->width;
	buf->init_height = buf->height;
	buf->astate.doom = malloc(sizeof(struct doom_state));

	if (buf->astate.doom == NULL) return; //dgn_throw(DGN_ALLOC);

	uint16_t tmp_len = buf->width * buf->height;
	buf->astate.doom->buf = malloc(tmp_len);
	tmp_len -= buf->width;

	if (buf->astate.doom->buf == NULL) return; //dgn_throw(DGN_ALLOC);

	memset(buf->astate.doom->buf, 0, tmp_len);
	memset(buf->astate.doom->buf + tmp_len, DOOM_STEPS - 1, buf->width);
}

static void doom_free(struct term_buf* buf)
{
	free(buf->astate.doom->buf);
	free(buf->astate.doom);
}

static void doom(struct term_buf* term_buf)
{
	static struct tb_cell fire[DOOM_STEPS] =
	{
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
	};

	uint16_t src;
	uint16_t random;
	uint16_t dst;

	uint16_t w = term_buf->init_width;
	uint8_t* tmp = term_buf->astate.doom->buf;

	if ((term_buf->width != term_buf->init_width) || (term_buf->height != term_buf->init_height))
	{
		return;
	}

	struct tb_cell* buf = tb_cell_buffer();

	for (uint16_t x = 0; x < w; ++x)
	{
		for (uint16_t y = 1; y < term_buf->init_height; ++y)
		{
			src = y * w + x;
			random = ((rand() % 7) & 3);
			dst = src - random + 1;

			if (w > dst)
			{
				dst = 0;
			}
			else
			{
				dst -= w;
			}

			tmp[dst] = tmp[src] - (random & 1);

			if (tmp[dst] > 12)
			{
				tmp[dst] = 0;
			}

			buf[dst] = fire[tmp[dst]];
			buf[src] = fire[tmp[src]];
		}
	}
}

/*void printf_tb(int x, int y, uint32_t fg, uint32_t bg, const char* fmt, ...)
{
	char buf[4096];
	va_list vl;
	va_start(vl, fmt);
	vsnprintf(buf, sizeof(buf), fmt, vl);
	va_end(vl);
	print_tb(buf, x, y, fg, bg);
}*/

int main()
{
	tb_init();
	tb_select_output_mode(TB_OUTPUT_NORMAL);
	tb_clear();

/*	tb_select_output_mode(TB_OUTPUT_TRUECOLOR);
	int w = tb_width();
	int h = tb_height();
	uint32_t bg = 0x000000, fg = 0x000000;
	tb_clear();
	int z = 0;

	tb_present();*/

	struct term_buf buf;
	buf.width = tb_width();
	buf.height = tb_height();

	//switch_tty(&buf);
	doom_init(&buf);
	while (1) {
		struct tb_event ev;
//		int t = tb_poll_event(&ev);
		int t = tb_peek_event(&ev, 10);

		if (t == -1) break;
		if (t == TB_EVENT_KEY) break;

		tb_clear();
		doom(&buf);
		tb_print("Fire demo!", 33, 1, TB_MAGENTA | TB_BOLD, TB_DEFAULT);
		tb_present();
//		usleep(10000);
	}
	doom_free(&buf);

	tb_shutdown();
	return 0;
}
