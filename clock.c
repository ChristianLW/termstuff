#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>

const float fov = 90;
const float near = 0.1;
const float far = 10;

uint32_t *frameBuffer;
int fw, fh;

#define TAU 6.283185307179586

#define R(c) (c >> 16 & 0xFF)
#define G(c) (c >>  8 & 0xFF)
#define B(c) (c       & 0xFF)
#define RGB(c) R(c), G(c), B(c)

#define ERROR(msg) do { fputs(msg, stderr); return 1; } while (false)

#ifdef DEBUG_LOG
FILE *dbglog;
#define DEBUG(fmt, ...) fprintf(dbglog, fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

void putPixel(int x, int y, uint32_t c) {
	frameBuffer[x + y*fw] = c;
	//fprintf(stderr, "Put \e[38;2;%i;%i;%im\u2588\u2588\e[m #%06x at %i,%i\n", RGB(c), c, x, y);
}

float hueToChannel(float hue) {
	float value = fabs(3.0f - fmod(hue, 6.0f)) - 1.0f;
	return value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value;
}

uint32_t hueToColour(float hue) {
	uint8_t r = (uint8_t)(hueToChannel(hue + 0.0f) * 255.0f + 0.5f);
	uint8_t g = (uint8_t)(hueToChannel(hue + 4.0f) * 255.0f + 0.5f);
	uint8_t b = (uint8_t)(hueToChannel(hue + 2.0f) * 255.0f + 0.5f);
	return r << 16 | g << 8 | b;
}

void drawLine(int x0, int y0, int x1, int y1) {
	int dx = abs(x1 - x0);
	int dy = -abs(y1 - y0);
	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;
	int e = dx + dy;
	bool cx = dx >= -dy;
	int d = cx ? dx : -dy;
	while (true) {
		float t = (float)(d - abs(cx ? x1 - x0 : y1 - y0)) / d;
		//fprintf(stderr, "%5.1f%% \e[38;2;%i;%i;%im\u2588\u2588\e[m #%06x at %3i,%3i\n", t*100, RGB(hueToColour(t*6.0f)), hueToColour(t*6.0f), x0, y0);
		putPixel(x0, y0, hueToColour(t * 6.0f));
		int e2 = 2 * e;
		if (e2 >= dy) {
			if (x0 == x1) break;
			e += dy, x0 += sx;
		}
		if (e2 <= dx) {
			if (y0 == y1) break;
			e += dx, y0 += sy;
		}
	}
}

void drawRect(int x0, int y0, int x1, int y1) {
	drawLine(x0, y0, x1, y0);
	drawLine(x1, y0, x1, y1);
	drawLine(x1, y1, x0, y1);
	drawLine(x0, y1, x0, y0);
}

void drawClockLine(int r0, int r1, float theta) {
	float x = sin(theta), y = -cos(theta);
	int cx = fw/2, cy = fh/2;
	DEBUG("\e[36m%2i-%2i\e[m , \e[36;1m%3.0f°\e[m : \e[32;1m%6.2f,%6.2f\e[m - \e[32;1m%6.2f,%6.2f\e[m\n", r0, r1, theta*(360/TAU), (int)(cx+x*r0+0.5f), (int)(cy+y*r0+0.5f), (int)(cx+x*r1+0.5f), (int)(cy+y*r1+0.5f));
	drawLine((int)(cx + x * r0 + 0.5f), (int)(cy + y * r0 + 0.5f), (int)(cx + x * r1 + 0.5f), (int)(cy + y * r1 + 0.5f));
}

void col(float *out, int x, int y) {
	float u = (float)x/(fw-1);
	float v = (float)y/(fh-1);
	float r = (float)fw / fh;
	float cx = 2.5f * (u - 0.5f);
	float cy = 2.5f * (v - 0.5f);
	if (r > 1) cx *= r; else cy /= r;
	float zx = cx;
	float zy = cy;
	float tmp;
	out[0] = out[1] = out[2] = 0.0f;
	for (int i = 0; i < 16; i++) {
		tmp = zx;
		zx = zx*zx - zy*zy + cx;
		zy = 2.0f*tmp*zy + cy;
		if (zx*zx + zy*zy > 2*2) {
			out[2] = i / 15.0f;
			break;
		}
	}
}

int main(int argc, char **argv) {
	#ifdef DEBUG_LOG
	dbglog = fopen("term.log", "wb");
	#endif
	struct winsize ws = {};
	ioctl(0, TIOCGWINSZ, &ws);
	fw = ws.ws_col;
	fh = ws.ws_row * 2;
	if (!fw || !fh) ERROR("Could not get window size");

	frameBuffer = malloc(fw * fh * sizeof(uint32_t));
	if (!frameBuffer) ERROR("Could not allocate frame buffer");

	struct termios termios;
	tcgetattr(0, &termios);
	tcflag_t orig_lflag = termios.c_lflag;
	termios.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &termios);

	fputs("\e[?1049h\e[?25l", stdout);

	int cx = fw/2, cy = fh/2, r = (fw < fh ? cx : cy) - 1;
	float theta = 0.0f;
	while (true) {
		memset(frameBuffer, 0, fw * fh * sizeof(uint32_t));
		drawRect(0, 0, fw - 1, fh - 1);
		for (int i = 0; i < 12; i++) drawClockLine(r - 6, r - 2, TAU/12 * i);
		time_t t = time(NULL);
		struct tm *tm = localtime(&t);
		drawClockLine(0, r/2,   TAU/12 * tm->tm_hour);
		drawClockLine(0, 3*r/4, TAU/60 * tm->tm_min);
		drawClockLine(0, r,     TAU/60 * tm->tm_sec);
		//for (int i = 0; i < 60; i++) drawClockLine(0, r, TAU/60 * i);
		//drawLine(cx, cy, cx + (int)(cos(theta) * r + 0.5f), cy + (int)(sin(theta) * r + 0.5f));
		#ifdef DEBUG_LOG
		//fputs("────────────────────────────────────────────\n", dbglog);
		fflush(dbglog);
		#endif

		bool blank = true;
		for (int y = 0;;) {
			for (int x = 0; x < fw; x++) {
				uint32_t top = frameBuffer[x + y*fw];
				uint32_t bot = frameBuffer[x + (y+1)*fw];
				bool wasBlank = blank;
				blank = false;
				if (!top && !bot) { if (!wasBlank) fputs("\e[m ", stdout); else putchar(' '); blank = true; }
				else if (top && !bot) printf("\e[;38;2;%i;%i;%im\u2580", RGB(top));
				else if (!top && bot) printf("\e[;38;2;%i;%i;%im\u2584", RGB(bot));
				else if (top == bot) printf("\e[;38;2;%i;%i;%im\u2588", RGB(top));
				else printf("\e[48;2;%i;%i;%i;38;2;%i;%i;%im\u2584", RGB(top), RGB(bot));
			}
			if ((y += 2) >= fh) break;
			putchar('\n');
		}

		fputs("\e[m\e[H", stdout);
		if (getchar() == 'q') break;

		theta += TAU / 128;
	}

	fputs("\e[?25h\e[?1049l", stdout);

	termios.c_lflag = orig_lflag;
	tcsetattr(0, TCSAFLUSH, &termios);

	free(frameBuffer);

	#ifdef DEBUG_LOG
	fclose(dbglog);
	#endif
}
