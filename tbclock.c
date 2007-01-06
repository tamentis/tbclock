/*
 * $Id: tbclock.c,v 1.2 2007-01-06 14:10:42 tamentis Exp $
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
 * tbclock (Tamentis Binary Clock)
 *
 * Compile with :
 * 	gcc -lncurses tbclock.c -Wall -O -o tbclock
 *
 * Changelog since 1.1.1.1
 * -----------------------
 *	* Should work on any terminal resolution above 8x5
 *	* Should also work on monochrome terminals.
 *	* Blocks will scale depending of the size of the terminal.
 *	* Stop if the terminal is resized (to be fixed...)
 *	* Reorganized functions.
 *
 */

#include <sys/ioctl.h>

#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <stdlib.h>

static WINDOW *screen;
int now_sec, now_min, now_hour;
int top_margin, left_margin;
int dot_w, dot_h;
int dot_sw, dot_sh;
int height = 24, width = 80;
int color;


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
	dot(screen, hms&32, left_margin     , y, color);
	dot(screen, hms&16, left_margin + (dot_w+dot_sw), y, color);
	dot(screen, hms&8,  left_margin + (dot_w+dot_sw)*2, y, color);
	dot(screen, hms&4,  left_margin + (dot_w+dot_sw)*3, y, color);
	dot(screen, hms&2,  left_margin + (dot_w+dot_sw)*4, y, color);
	dot(screen, hms&1,  left_margin + (dot_w+dot_sw)*5, y, color);
}


void
update_display(void)
{
	time_t now;
	struct tm *tm;

	now = time(NULL);
	tm = localtime(&now);

	line(screen, tm->tm_hour, top_margin, 1);
	line(screen, tm->tm_min,  top_margin + dot_h + dot_sh, 2);
	line(screen, tm->tm_sec,  top_margin + (dot_h+dot_sh)*2, 3);

	wrefresh(screen);
	refresh();
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


int
main(int ac, char **av)
{
	struct winsize ws;
	WINDOW *mainwnd;

	signal(SIGWINCH, resize);

	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1) {
		width = ws.ws_col;
		height = ws.ws_row;
	}

	/* Deny too small terminals */
	if (height < 5 || width < 8) {
		endwin();
		fprintf(stderr, "The smallest allowed terminal is 8x5\n");
		exit(-1);
	}

	/* Screen Initialization */
	mainwnd = initscr();
	noecho();
	cbreak();
	nodelay(mainwnd, TRUE);

	/* Prepare the 3 colors */
	start_color();
	init_pair(1, COLOR_BLUE, COLOR_BLUE);
	init_pair(2, COLOR_RED, COLOR_RED);
	init_pair(3, COLOR_YELLOW, COLOR_YELLOW);

	/* Dot sizes */
	dot_h  = height / 8; if (dot_h < 1) dot_h = 1;
	dot_sh = height / 12;
	dot_w  = width / 12; if (dot_w < 1) dot_w = 1;
	dot_sw = width / 20;

	/* Calculate margins... */
	top_margin = (height - 3 * dot_h - dot_sh * 2) / 2;
	left_margin = (width - 6 * dot_w - dot_sw * 5) / 2;

	/* Prepare inside frame */
	screen = newwin(height, width, 0, 0);
	box(screen, ACS_VLINE, ACS_HLINE);
	mvwprintw(screen, 0, width-10, "tbclock");

	/* Main loop */
	for (;;) {
		if (getch() != -1)
			break;
		update_display();
		sleep(1);
	}

	endwin();
	printf("Thanks for using tbclock!\n");

	return (0);
}
