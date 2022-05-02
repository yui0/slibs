// gcc termbox_pixel.c -o termbox_pixel
#include "termbox.h"

int main()
{
	tb_init();
	tb_select_output_mode(TB_OUTPUT_TRUECOLOR);
	tb_clear();

	int width = tb_width();
	int height = tb_height();

	while (1) {
		struct tb_event ev;
		int t = tb_peek_event(&ev, 10);

		if (t == -1) break;
		if (t == TB_EVENT_KEY) break;

		tb_clear();
		for (int y=0; y < height; ++y) {
			for (int x=0; x < width; ++x) {
				tb_pixel(x, y, (x*256*256 + (rand() % 64)*256 + (rand() % 96 - y * 8)));
			}
		}
		tb_present();
	}

	tb_shutdown();
	return 0;
}
