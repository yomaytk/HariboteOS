int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_boxfillwin(int win, int x0, int y0, int x1, int y1, int col);
void api_initmalloc();
char *api_malloc(int size);
void api_free(int addr, int size);
void api_point(int win, int x, int y, int col);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);
void api_end(void);

void main(void)
{
	char *buf;
	int win;

	api_initmalloc();
	buf = api_malloc(150 * 100);
	win = api_openwin(buf, 150, 100, -1, "star1");
	api_boxfillwin(win,  6, 26, 143, 93, 0);
	for(int i = 0;i < 50;i++){
		int x, y;
		x = (i * i)%137 + 6;
		y = (i * (i+3))%67 + 3;
		api_point(win + 1, x, y, 3);
	}
	api_refreshwin(win, 6, 26, 144, 94);
	api_end();
}
