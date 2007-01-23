/* $Id: main.c,v 1.1 2007-01-23 13:29:04 tamentis Exp $
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

#include <err.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <time.h>

#include "tbclock.h"

#define MSG_UNKNOWNMOD	"I don't know this module (man tbclock).\n"
#define MSG_THANKS	"Thank you for using tbclock!\n"

TBC tbc;


/* tbc_draw_dot - prints one dot if valid */
void
tbc_draw_dot(int valid, int x, int y, short color)
{
	int i, j;
	char c[2] = "#";

	if (!valid) {
		color = 0;
		c[0] = ' ';
	}

	wbkgdset(tbc.screen, COLOR_PAIR(color));
	for (i = 0; i < tbc.dot_h; i++) {
		for (j = 0; j < tbc.dot_w; j++) {
			mvwprintw(tbc.screen, y + i, x + j, c);
		}
	}
}


/* tbc_draw_line - prints 6 dots with the same color */
void
tbc_draw_line(int hms, int y, short color)
{
	int k;
	unsigned int x;

	for (k = 0; k < 6; k++) {
		x = tbc.left_margin + (tbc.dot_w + tbc.dot_sw) * (5 - k);
		tbc_draw_dot(hms & (1<<k), x, y + tbc.top_margin, color);
	}
}


/* tbc_refresh */
void
tbc_refresh()
{
	wrefresh(tbc.screen);
	refresh();
}


/* tbc_draw_time - draw the time, with or without tenth of seconds */
void
tbc_draw_time(int res, int hour, int min, int sec, int dsec)
{
	tbc_draw_line(hour, 0, BLOCK_BLUE);
	tbc_draw_line(min, tbc.dot_h + tbc.dot_sh, BLOCK_RED);
	tbc_draw_line(sec, (tbc.dot_h + tbc.dot_sh) * 2, BLOCK_YELLOW);
	if (res > 3)
		tbc_draw_line(dsec, (tbc.dot_h + tbc.dot_sh) * 3, BLOCK_GREEN);
}


/* tbc_fatal - stop execution and print error message */
void
tbc_fatal(char *msg)
{
	endwin();
	errx(-1, "%s", msg);
}
	

/* resize - called when the terminal is resized... broken for the moment */
void
resize(int signal)
{
	tbc_fatal("Terminal resize is not supported.");
}


/* TODO: resize will need to be implemented... one day */
#if 0
void
resize(int signal)
{
	struct winsize ws;
	int nw, nh;

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
}
#endif


/* tbc_clear_innerzone - clear everything inside the borders */
void
tbc_clear_innerzone(void)
{
	char *spc;
	int i;

	/* Create a string long enough to clean one line */
	spc = malloc(tbc.width - 1);
	memset(spc, ' ', tbc.width - 2);
	*(spc + tbc.width - 1) = 0;

	/* Repeat that all over the place */
	wbkgdset(tbc.screen, COLOR_PAIR(BACK_DEFAULT));
	for (i = 1; i < tbc.height - 1; i++)
		mvwprintw(tbc.screen, i, 1, spc);

	free(spc);
}


/* tbc_set_default - set the starting vars, default options */
void
tbc_set_default()
{
	tbc.bigbang = time(NULL);
	tbc.width = 80;
	tbc.height = 24;
	tbc.opt_frame = 1;
	tbc.opt_border = 1;
}


/* tbc_display_init - starts curses, obtains term size, set colors */
void
tbc_display_init()
{
	struct winsize ws;
	WINDOW *mainwnd;

	/* terminal size stuff */
	signal(SIGWINCH, resize);
	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1) {
		tbc.width = ws.ws_col;
		tbc.height = ws.ws_row;
	}

	/* curses screen init */
	mainwnd = initscr();
	noecho();
	cbreak();
	curs_set(0);
	nodelay(mainwnd, TRUE);

	/* prepare colors */
	start_color();
        if (use_default_colors() != ERR) {
		init_pair(TEXT_DEFAULT, -1, -1);
		init_pair(BLOCK_RED,	COLOR_RED, COLOR_RED);
		init_pair(BLOCK_GREEN,	COLOR_GREEN, COLOR_GREEN);
		init_pair(BLOCK_BLUE,	COLOR_BLUE, COLOR_BLUE);
		init_pair(BLOCK_YELLOW,	COLOR_YELLOW, COLOR_YELLOW);
		init_pair(TEXT_RED,	COLOR_RED, -1);
		init_pair(TEXT_GREEN,	COLOR_GREEN, -1);
		init_pair(BACK_YELLOW,	COLOR_BLACK, COLOR_YELLOW);
	}
}


/* tbc_configure - setup layout and all those things, will be called from
 * modules, res is the time resolution 3 = seconds, 4 = tenth of seconds */
void
tbc_configure(unsigned short res, short y_offset)
{

	/* Calculate sizes and margins... */
	if (tbc.opt_frame) {
		tbc.dot_h = tbc.height / (res * 2);
		if (tbc.dot_h < 1)
			tbc.dot_h = 1;

		tbc.dot_w = tbc.width / 12; 
		if (tbc.dot_w < 1)
			tbc.dot_w = 1;

		if (tbc.opt_border) {
			tbc.dot_sh = tbc.height / (res * 4);
			tbc.dot_sw = tbc.width / 20;
		} else
			tbc.dot_sh = tbc.dot_sw = 0;

		tbc.top_margin = (tbc.height - res * tbc.dot_h 
				- tbc.dot_sh * (res - 1)) / 2 + y_offset;
		tbc.left_margin = (tbc.width - 6 * tbc.dot_w 
				- tbc.dot_sw * 5) / 2;
	} else {
		tbc.dot_h = tbc.height / 3;
		if (tbc.dot_h < 1)
			tbc.dot_h = 1;

		tbc.dot_w = tbc.width / 7;
		if (tbc.dot_w < 1)
			tbc.dot_w = 1;

		if (tbc.opt_border)
			tbc.dot_sh = tbc.dot_sw = 1;
		else
			tbc.dot_sh = tbc.dot_sw = 0;

		tbc.top_margin = (tbc.height - tbc.dot_h * 3
				- tbc.dot_sh * 2) / 2;
		tbc.left_margin = (tbc.width - tbc.dot_w * 6
				- tbc.dot_sw * 5) / 2;
	}

	/* Prepare inside frame */
	tbc.screen = newwin(tbc.height, tbc.width, 0, 0);
	if (tbc.opt_frame) {
		box(tbc.screen, ACS_VLINE, ACS_HLINE);
		mvwprintw(tbc.screen, 0, tbc.width-15, TBCVER);
	}
}



int
main(int ac, char **av)
{
	int ch;
	char *modulename = NULL;
	void (*modulecallback)() = NULL;

	tbc_set_default();

	while ((ch = getopt(ac, av, "hvfbg:m:")) != -1) {
		switch (ch) {
		case 'v':
			fprintf(stderr, TBCCOPY);
			exit(-1);
		case 'f':
			tbc.opt_frame = 0;
			break;
		case 'b':
			tbc.opt_border = 0;
			break;
		case 'g':
		case 'm':
			modulename = optarg;
			break;
		case 'h':
		default:
			fprintf(stderr, "usage: %s [-v] [-f] [-b] [-m name]\n", 
					av[0]);
			exit(-1);
		}
	}
	ac -= optind;
	av += optind;

	tbc_display_init();

	/* module selection */
	if (modulename != NULL) {
		if (strncmp(modulename, "clock", 6) == 0) {
			modulecallback = mod_clock;
		} else if (strncmp(modulename, "guessbin", 9) == 0) {
			tbc.opt_frame = 1;
			tbc.opt_border = 1;
			modulecallback = mod_guessbin;
		} else if (strncmp(modulename, "chrono", 7) == 0) {
			modulecallback = mod_chrono;
		} else {
			fprintf(stderr, MSG_UNKNOWNMOD);
			exit(-1);
		}
	}

	/* run the selected module */
	if (modulecallback == NULL)
		mod_clock();
	else
		modulecallback();

	endwin();
	fprintf(stderr, MSG_THANKS);

	return (0);
}

