// gcc termbox_matrix.c -o termbox_matrix
#include "termbox.h"

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

union anim_state
{
	struct matrix_state* matrix;
};

struct term_buf
{
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

	union anim_state astate;
};

static void matrix_init(struct term_buf* buf)
{
	buf->init_width = buf->width;
	buf->init_height = buf->height;
	buf->astate.matrix = malloc(sizeof(struct matrix_state));
	struct matrix_state* s = buf->astate.matrix;

	if (s == NULL) return; //dgn_throw(DGN_ALLOC);

	uint16_t len = buf->height + 1;
	s->grid = malloc(sizeof(struct matrix_dot*) * len);

	if (s->grid == NULL) return; //dgn_throw(DGN_ALLOC);

	len = (buf->height + 1) * buf->width;
	(s->grid)[0] = malloc(sizeof(struct matrix_dot) * len);

	if ((s->grid)[0] == NULL) return; //dgn_throw(DGN_ALLOC);

	for (int i = 1; i <= buf->height; ++i)
	{
		s->grid[i] = s->grid[i - 1] + buf->width;

		if (s->grid[i] == NULL) return; //dgn_throw(DGN_ALLOC);
	}

	s->length = malloc(buf->width * sizeof(int));

	if (s->length == NULL) return; //dgn_throw(DGN_ALLOC);

	s->spaces = malloc(buf->width * sizeof(int));

	if (s->spaces == NULL) return; //dgn_throw(DGN_ALLOC);

	s->updates = malloc(buf->width * sizeof(int));

	if (s->updates == NULL) return; //dgn_throw(DGN_ALLOC);

	// Initialize grid
	for (int i = 0; i <= buf->height; ++i)
	{
		for (int j = 0; j <= buf->width - 1; j += 2)
		{
			s->grid[i][j].val = -1;
		}
	}

	for (int j = 0; j < buf->width; j += 2)
	{
		s->spaces[j] = (int) rand() % buf->height + 1;
		s->length[j] = (int) rand() % (buf->height - 3) + 3;
		s->grid[1][j].val = ' ';
		s->updates[j] = (int) rand() % 3 + 1;
	}
}

static void matrix_free(struct term_buf* buf)
{
	free(buf->astate.matrix->grid[0]);
	free(buf->astate.matrix->grid);
	free(buf->astate.matrix->length);
	free(buf->astate.matrix->spaces);
	free(buf->astate.matrix->updates);
	free(buf->astate.matrix);
}

static void matrix(struct term_buf* buf)
{
	static int frame = 3;
	const int frame_delay = 8;
	static int count = 0;
	bool first_col;
	struct matrix_state* s = buf->astate.matrix;

	// Allowed codepoints
	const int randmin = 33;
	const int randnum = 123 - randmin;
	// Chars change mid-scroll
	const bool changes = true;

	if ((buf->width != buf->init_width) || (buf->height != buf->init_height))
	{
		return;
	}

	count += 1;
	if (count > frame_delay) {
		frame += 1;
		if (frame > 4) frame = 1;
		count = 0;

		for (int j = 0; j < buf->width; j += 2)
		{
			int tail;
			if (frame > s->updates[j])
			{
				if (s->grid[0][j].val == -1 && s->grid[1][j].val == ' ')
				{
					if (s->spaces[j] > 0)
					{
						s->spaces[j]--;
					} else {
						s->length[j] = (int) rand() % (buf->height - 3) + 3;
						s->grid[0][j].val = (int) rand() % randnum + randmin;
						s->spaces[j] = (int) rand() % buf->height + 1;
					}
				}

				int i = 0, seg_len = 0;
				first_col = 1;
				while (i <= buf->height)
				{
					// Skip over spaces
					while (i <= buf->height
							&& (s->grid[i][j].val == ' ' || s->grid[i][j].val == -1))
					{
						i++;
					}

					if (i > buf->height) break;

					// Find the head of this col
					tail = i;
					seg_len = 0;
					while (i <= buf->height
							&& (s->grid[i][j].val != ' ' && s->grid[i][j].val != -1))
					{
						s->grid[i][j].is_head = false;
						if (changes)
						{
							if (rand() % 8 == 0)
								s->grid[i][j].val = (int) rand() % randnum + randmin;
						}
						i++;
						seg_len++;
					}

					// Head's down offscreen
					if (i > buf->height)
					{
						s->grid[tail][j].val = ' ';
						continue;
					}

					s->grid[i][j].val = (int) rand() % randnum + randmin;
					s->grid[i][j].is_head = true;

					if (seg_len > s->length[j] || !first_col) {
						s->grid[tail][j].val = ' ';
						s->grid[0][j].val = -1;
					}
					first_col = 0;
					i++;
				}
			}
		}
	}

	uint32_t blank;
	utf8_char_to_unicode(&blank, " ");

	for (int j = 0; j < buf->width; j += 2) {
		for (int i = 1; i <= buf->height; ++i)
		{
			uint32_t c;
			int fg = TB_GREEN;
			int bg = TB_DEFAULT;

			if (s->grid[i][j].val == -1 || s->grid[i][j].val == ' ')
			{
				tb_change_cell(j, i - 1, blank, fg, bg);
				continue;
			}

			char tmp[2];
			tmp[0] = s->grid[i][j].val;
			tmp[1] = '\0';
			if(utf8_char_to_unicode(&c, tmp))
			{
				if (s->grid[i][j].is_head)
				{
					fg = TB_WHITE | TB_BOLD;
				}
				tb_change_cell(j, i - 1, c, fg, bg);
			}
		}
	}
}

int main()
{
	tb_init();
	tb_select_output_mode(TB_OUTPUT_NORMAL);
	tb_clear();

	struct term_buf buf;
	buf.width = tb_width();
	buf.height = tb_height();

	matrix_init(&buf);
	while (1) {
		struct tb_event ev;
		int t = tb_peek_event(&ev, 10);

		if (t == -1) break;
		if (t == TB_EVENT_KEY) break;

		tb_clear();
		matrix(&buf);
		tb_print("Matrix demo!", 33, 1, TB_MAGENTA | TB_BOLD, TB_DEFAULT);
		tb_present();
	}
	matrix_free(&buf);

	tb_shutdown();
	return 0;
}
