/* $Id: tbclock.c,v 1.5 2007-01-14 22:42:46 tamentis Exp $
 *
 * Copyright (c) 2007 Bertrand Janin <tamentis@neopulsar.org>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/ioctl.h>

#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <time.h>

#define TBCVER "tbclock 1.5"
#define TBCCOPY TBCVER " - Tamentis Binary Clock (c) 2007 Bertrand Janin\n"

WINDOW *screen;
int now_sec, now_min, now_hour;
int top_margin, left_margin;
int dot_w, dot_h;
int dot_sw, dot_sh;
int height = 24, width = 80;
int color;

void game_guessbin();

void
dot(WINDOW *screen, int valid, int x, int y, short color)
{
	int i, j;
	char c[2] = "#";

	if (!valid) {
		color = 0;
		c[0] = ' ';
	}

	wbkgdset(screen, COLOR_PAIR(color));
	for (i = 0; i < dot_h; i++) {
		for (j = 0; j < dot_w; j++) {
			mvwprintw(screen, y + i, x + j, c);
		}
	}
}


void
line(WINDOW *screen, int hms, int y, short color)
{
	dot(screen, hms&32, left_margin,                    y, color);
	dot(screen, hms&16, left_margin + (dot_w+dot_sw),   y, color);
	dot(screen, hms&8,  left_margin + (dot_w+dot_sw)*2, y, color);
	dot(screen, hms&4,  left_margin + (dot_w+dot_sw)*3, y, color);
	dot(screen, hms&2,  left_margin + (dot_w+dot_sw)*4, y, color);
	dot(screen, hms&1,  left_margin + (dot_w+dot_sw)*5, y, color);
}


void
resize(int signal)
{
	//struct winsize ws;
	//int nw, nh;

	endwin();
	fprintf(stderr, "I don't like when you resize your term... sorry\n");
	exit(-1);

#if 0
	// This is unstable code to handle resizing... it hangs sometimes
	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1) {
		nw = ws.ws_col;
		nh = ws.ws_row;

		if (nw == width && nh == height) 
			return;
	} else {
		return;
	}


	delwin(screen);
	screen = newwin(nh, nw, 0, 0);
	box(screen, ACS_VLINE, ACS_HLINE);
#endif
}


/* Main loop for the 'clock-only' version */
void
justclock()
{
	time_t now;
	struct tm *tm;

	for (;;) {
		if (getch() != -1)
			break;

		now = time(NULL);
		tm = localtime(&now);

		line(screen, tm->tm_hour, top_margin, 1);
		line(screen, tm->tm_min,  top_margin + dot_h + dot_sh, 2);
		line(screen, tm->tm_sec,  top_margin + (dot_h+dot_sh)*2, 3);

		wrefresh(screen);
		refresh();

		sleep(1);
	}
}


int
main(int ac, char **av)
{
	struct winsize ws;
	WINDOW *mainwnd;
	int frame = 1, ch, border = 1;
	char *gamename = NULL;
	void (*gamecallback)() = NULL;

	while ((ch = getopt(ac, av, "hvfbg:")) != -1) {
		switch (ch) {
		case 'v':
			fprintf(stderr, TBCCOPY);
			fprintf(stderr, "%s\n", TBCVER);
			exit(-1);
		case 'f':
			frame = 0;
			break;
		case 'b':
			border = 0;
			break;
		case 'g':
			gamename = optarg;
			break;
		case 'h':
		default:
			fprintf(stderr, "Usage: %s [-v] [-f] [-b] [-g name]\n", 
					av[0]);
			exit(-1);
		}
	}
	ac -= optind;
	av += optind;

	/* Check if we have a gamename */
	if (gamename != NULL) {
		frame = 1;
		border = 1;
		if (strncmp(gamename, "guessbin", 9) == 0) {
			gamecallback = game_guessbin;
		} else {
			fprintf(stderr, "I never heard of this game!\n");
			exit(-1);
		}
	}

	signal(SIGWINCH, resize);

	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1) {
		width = ws.ws_col;
		height = ws.ws_row;
	}

	/* If terminal is really small, disable frame and borders */
	if (height < 5 || width < 8)
		frame = border = 0;

	/* If terminal is too small, quit */
	if (height < 3 || width < 6) {
		fprintf(stderr, "The minimal allowed terminal size is 6x3 !\n");
		exit(-1);
	}


	/* Screen Initialization */
	mainwnd = initscr();
	noecho();
	cbreak();
	curs_set(0);
	nodelay(mainwnd, TRUE);


	/* Prepare the 3 colors */
	start_color();
        if (use_default_colors() != ERR) {
		init_pair(COLOR_BLACK, -1, -1);
		init_pair(1, COLOR_BLUE, COLOR_BLUE);
		init_pair(2, COLOR_RED, COLOR_RED);
		init_pair(3, COLOR_YELLOW, COLOR_YELLOW);
		init_pair(4, COLOR_RED, -1);
		init_pair(5, COLOR_GREEN, -1);
	}

	/* Calculate sizes and margins... */
	if (frame) {
		dot_h  = height / 8; if (dot_h < 1) dot_h = 1;
		dot_w  = width / 12; if (dot_w < 1) dot_w = 1;
		if (border) {
			dot_sh = height / 12;
			dot_sw = width / 20;
		} else
			dot_sh = dot_sw = 0;
		top_margin = (height - 3 * dot_h - dot_sh * 2) / 2;
		left_margin = (width - 6 * dot_w - dot_sw * 5) / 2;
	} else {
		dot_h  = height / 3; if (dot_h < 1) dot_h = 1;
		dot_w  = width / 7; if (dot_w < 1) dot_w = 1;
		if (border)
			dot_sh = dot_sw = 1;
		else
			dot_sh = dot_sw = 0;
		top_margin = (height - dot_h * 3 - dot_sh * 2) / 2;
		left_margin = (width - dot_w * 6 - dot_sw * 5) / 2;
	}

	/* Prepare inside frame */
	screen = newwin(height, width, 0, 0);
	if (frame) {
		box(screen, ACS_VLINE, ACS_HLINE);
		mvwprintw(screen, 0, width-15, TBCVER);
	}

	/* Display a game or just the clock */
	if (gamecallback != NULL)
		gamecallback();
	else
		justclock();

	endwin();
	printf("Thank you for using tbclock!\n");

	return (0);
}

