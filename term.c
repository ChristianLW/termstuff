#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifndef USE_STTY_TPUT
#include <sys/ioctl.h>
#include <termios.h>
#endif

#define DEBUG_CIRCLE

const char *modes[] = {"rainbow", "grey", "grid", "noise", "colournoise", "mandelbrot", "circle"};
const int NUM_MODES = sizeof(modes) / sizeof(modes[0]);

#ifdef DEBUG_CIRCLE
float cminx = INFINITY, cmaxx = -INFINITY, cminy = INFINITY, cmaxy = -INFINITY;
#endif

void printModes() {
		fputs("The mode can be one of the following: ", stdout);
		fputs(modes[0], stdout);
		for (int i = 1; i < NUM_MODES; i++) {
			fputs(", ", stdout);
			fputs(modes[i], stdout);
		}
		fputc('\n', stdout);
		exit(1);
}

int getMode(int argc, char **argv) {
	if (argc <= 1) {
		puts("You have to specify an extra argument to tell the program what to draw.");
		printModes();
	}
	for (int i = 0;;) {
		if (strcmp(argv[1], modes[i]) == 0) return i;
		if (++i < NUM_MODES) continue;
		printf("%s is not a valid mode.\n", argv[1]);
		printModes();
	}
}

float hueToChannel(float hue, float offset) {
	float value = fabs(3.0f - fmod(hue - offset + 6.0f, 6.0f)) - 1.0f;
	return value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value;
}

void col(float *out, int mode, int x, int y, int w, int h) {
	if (mode == 0) { // rainbow
		float hue = ((float)x/w + (float)y/h) * 3.0;
		out[0] = hueToChannel(hue, 0.0f);
		out[1] = hueToChannel(hue, 2.0f);
		out[2] = hueToChannel(hue, 4.0f);
	} else if (--mode == 0) { // grey
		float value = ((float)x/w + (float)y/h) * 0.5;
		out[0] = out[1] = out[2] = value;
	} else if (--mode == 0) { // grid
		float value = !((x + y) % 2);
		out[0] = out[1] = out[2] = value;
	} else if (--mode == 0) { // noise
		float value = (float)rand() / RAND_MAX;
		out[0] = out[1] = out[2] = value;
	} else if (--mode == 0) { // colournoise
		float hue = rand() * 6.0f / RAND_MAX;
		out[0] = hueToChannel(hue, 0.0f);
		out[1] = hueToChannel(hue, 2.0f);
		out[2] = hueToChannel(hue, 4.0f);
	} else if (--mode == 0) { // mandelbrot
		float u = (float)x/w;
		float v = (float)y/h;
		float r = (float)(w + 1) / (h + 1);
		float cx = 2.5f * (u - 0.5f);
		float cy = 2.5f * (v - 0.5f);
		if (r > 0) cx *= r; else cy /= r;
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
	} else if (--mode == 0) { // circle
		float u = (float)x/w;
		float v = (float)y/h;
		float r = (float)(w + 1) / (h + 1);
		float cx = 2.5f * (u - 0.5f);
		float cy = 2.5f * (v - 0.5f);
		if (r > 0) cx *= r; else cy /= r;
		float value = cx*cx + cy*cy <= 1.0f ? 1.0f : 0.0f;
#ifdef DEBUG_CIRCLE
		if (value == 1.0f) { if (cx < cminx) cminx = cx; if (cx > cmaxx) cmaxx = cx; if (cy < cminy) cminy = cy; if (cy > cmaxy) cmaxy = cy; }
#endif
		out[0] = out[1] = out[2] = value;
	}
}

void rgb(int *out, int mode, int x, int y, int w, int h) {
	float temp[6];
	col(temp+0, mode, x, 2*y+0, w, 2*h);
	col(temp+3, mode, x, 2*y+1, w, 2*h);
	for (int i = 0; i < 6; i++) out[i] = (int)(temp[i] * 255.0f + 0.5f);
}

#ifdef USE_STTY_TPUT
char *stty(const char *data) {
	if (!data) {
		FILE *p = popen("stty -g", "r");
		char *result = malloc(256);
		fgets(result, 256, p);
		pclose(p);
		return result;
	}
	char *str = strndup("stty ", 5 + strlen(data));
	strcpy(str + 5, data);
	system(str);
	free(str);
	return NULL;
}

int tput(const char *arg) {
	char *str = strndup("tput ", 5 + strlen(arg));
	strcpy(str + 5, arg);
	FILE *p = popen(str, "r");
	int result;
	fscanf(p, "%d", &result);
	pclose(p);
	free(str);
	return result;
}
#else
void getttysize(int *w, int *h) {
	struct winsize ws;
	ioctl(0, TIOCGWINSZ, &ws);
	*w = ws.ws_col;
	*h = ws.ws_row;
}
#endif

void main(int argc, char **argv) {
	int mode = getMode(argc, argv);

#ifdef USE_STTY_TPUT
	char *ttydata = stty(NULL);
	stty("-icanon -echo");
#else
	struct termios termios;
	tcgetattr(0, &termios);
	tcflag_t orig_lflag = termios.c_lflag;
	termios.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &termios);
#endif

	fputs("\e[?1049h\e[?25l", stdout);

#ifdef USE_STTY_TPUT
	int w = tput("cols");
	int h = tput("lines");
#else
	int w, h;
	getttysize(&w, &h);
#endif

	int rgbValues[6];
	for (int y = 0;;) {
		for (int x = 0; x < w; x++) {
			rgb(rgbValues, mode, x, y, w - 1, h - 1);
			printf(
				"\e[48;2;%i;%i;%i;38;2;%i;%i;%im\u2584",
				rgbValues[0], rgbValues[1], rgbValues[2],
				rgbValues[3], rgbValues[4], rgbValues[5]
			);
		}
		if (++y >= h) break;
		putchar('\n');
	}

	fputs("\e[m\e[H", stdout);
	getchar();

#ifdef USE_STTY_TPUT
	stty(ttydata);
	free(ttydata);
#else
	termios.c_lflag = orig_lflag;
	tcsetattr(0, TCSAFLUSH, &termios);
#endif

	fputs("\e[?25h\e[?1049l", stdout);
	// Lua flushes IO here, don't know if necessary
	// print(tostring(minx).." - "..tostring(maxx).."\n"..tostring(miny).." - "..tostring(maxy))
#ifdef DEBUG_CIRCLE
	if (mode == 6) printf("x \u2208 [%g, %g]\ny \u2208 [%g, %g]\nRatio = %g\n", cminx, cmaxx, cminy, cmaxy, (cmaxx - cminx) / (cmaxy - cminy));
#endif
}
