/* $Id: main.c,v 1.3 2007-02-07 11:18:42 tamentis Exp $
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
#define USAGE_FMT	"usage: %s [-v] [-f] [-b] [-d] [-a] [-H] [-m name]\n"

TBC tbc;


/* tbc_refresh */
void
tbc_refresh()
{
	wrefresh(tbc.screen);
	refresh();
}


/* tbc_draw_dot - prints one dot if valid */
void
tbc_draw_dot(int valid, int x, int y, short color)
{
	int i;
	char *s, c = '#';


	if (!valid) {
		color = BLOCK_DEFAULT;
		if (tbc.opt_dots)
			c = '.';
		else
			c = ' ';
	}

	s = malloc(tbc.dot_w + 1);
	memset(s, c, tbc.dot_w);
	s[tbc.dot_w+1] = 0;

	wbkgdset(tbc.screen, COLOR_PAIR(color));
	for (i = 0; i < tbc.dot_h; i++) {
		mvwprintw(tbc.screen, y + i, x, s);
	}

	free(s);
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


/* tbc_draw_line_vert - prints 6 vertical lines (alternative display) */
void
tbc_draw_line_a(int hms, int x, short color, short max)
{
	int k;
	unsigned int y;

	for (k = 0; k < 4; k++) {
		y = tbc.top_margin + (tbc.dot_h + tbc.dot_sh) * (3 - k);
		if (k < max)
 			tbc_draw_dot( hms & (1<<k), x, y, color);
	}
}


/* tbc_next_help_value - rotate through different reading helps */
void
tbc_next_help_value()
{
	int space = tbc.dot_h + tbc.dot_sh;
	unsigned int tm, lm;
	unsigned bm = 3;

	if (tbc.height < 6)
		bm = 1;
	else if (tbc.height < 15)
		bm = 2;

	tm = tbc.top_margin + tbc.dot_h / 2;
	lm = tbc.width - tbc.left_margin + tbc.left_margin / 2 - 1;

	if (tbc.opt_helper > 2)
		tbc.opt_helper = 0;
	else    
		tbc.opt_helper++;

	wbkgdset(tbc.screen, COLOR_PAIR(TEXT_DEFAULT));
	tbc_clear_innerzone();

}


/* tbc_draw_helpers - draw the currently selected reading help */
void
tbc_draw_helpers(int res, int hour, int min, int sec, int dsec)
{
	int space;
	char st[12];

	/* display bottom (full) helper */
	if (tbc.height > 3 && tbc.opt_helper > 1) {
		unsigned bm = 3;
		unsigned i;
		
		/* worry about terminal size */
		if (tbc.height < 6)
			bm = 1;
		else if (tbc.height < 15)
			bm = 2;

		/* do we need to show tenth of sec ? */
		if (res > 3)
			i = snprintf(st, 12, "%02u:%02u:%02u:%02d", hour, min, 
					sec, dsec);
		else
			i = snprintf(st, 9, "%02u:%02u:%02u", hour, min, sec);

		wbkgdset(tbc.screen, COLOR_PAIR(TEXT_DEFAULT));
		mvprintw(tbc.height - bm, tbc.width / 2 - (i / 2), st);
	}

	/* display side (by line) helper (normal horizontal lines) */
	if (!tbc.opt_vertical && tbc.width > 11 && 
			(tbc.opt_helper == 1 || tbc.opt_helper > 2)) {
		unsigned int tm, lm;

		tm = tbc.top_margin + tbc.dot_h / 2;
		lm = tbc.width - tbc.left_margin + tbc.left_margin / 2 - 1;
		space = tbc.dot_h + tbc.dot_sh;

		wbkgdset(tbc.screen, COLOR_PAIR(TEXT_DEFAULT));
		snprintf(st, 3, "%02u", hour);
		mvwprintw(tbc.screen, tm, lm, st);
		snprintf(st, 3, "%02u", min);
		mvwprintw(tbc.screen, tm + space, lm, st);
		snprintf(st, 3, "%02u", sec);
		mvwprintw(tbc.screen, tm + space * 2, lm, st);
		if (res > 3) {
			snprintf(st, 3, "%02u", dsec);
			mvwprintw(tbc.screen, tm + space * 3, lm, st);
		}
	}

	/* display bottom (by digit) helper (alternative vertical lines) */
	if (tbc.opt_vertical && tbc.width > 11 && 
			(tbc.opt_helper == 1 || tbc.opt_helper > 2)) {
		unsigned int tm, lm;

		tm = tbc.top_margin + (tbc.dot_h + tbc.dot_sh) * 4;
		lm = tbc.left_margin;
		space = (tbc.dot_sw + tbc.dot_w) * 2;

		wbkgdset(tbc.screen, COLOR_PAIR(TEXT_DEFAULT));
		snprintf(st, 3, "%02u", hour);
		mvwprintw(tbc.screen, tm, lm, st);
		snprintf(st, 3, "%02u", min);
		mvwprintw(tbc.screen, tm, lm + space, st);
		snprintf(st, 3, "%02u", sec);
		mvwprintw(tbc.screen, tm, lm + space * 2, st);
		if (res > 3) {
			snprintf(st, 3, "%02u", dsec);
			mvwprintw(tbc.screen, tm, lm + space * 3, st);
		}
	}
}


/* tbc_draw_time - draw the time, with or without tenth of seconds */
void
tbc_draw_time(int res, int hour, int min, int sec, int dsec)
{
	int space;
	unsigned int ml = tbc.left_margin;

	if (tbc.opt_vertical) {
		space = tbc.dot_sw + tbc.dot_w;
		tbc_draw_line_a(hour / 10, ml,             BLOCK_BLUE,   2);
		tbc_draw_line_a(hour % 10, ml + space,     BLOCK_BLUE,   4);
		tbc_draw_line_a(min / 10,  ml + space * 2, BLOCK_RED,    3);
		tbc_draw_line_a(min % 10,  ml + space * 3, BLOCK_RED,    4);
		tbc_draw_line_a(sec / 10,  ml + space * 4, BLOCK_YELLOW, 3);
		tbc_draw_line_a(sec % 10,  ml + space * 5, BLOCK_YELLOW, 4);
		if (res > 3)
			tbc_draw_line_a(dsec, ml + space * 6, BLOCK_GREEN, 4);
	} else {
		space = tbc.dot_h + tbc.dot_sh;
		tbc_draw_line(hour, 0, BLOCK_BLUE);
		tbc_draw_line(min, space, BLOCK_RED);
		tbc_draw_line(sec, space * 2, BLOCK_YELLOW);
		if (res > 3)
			tbc_draw_line(dsec, space * 3, BLOCK_GREEN);
	}

	tbc_draw_helpers(res, hour, min, sec, dsec);
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
	tbc.opt_dots = 1;
	tbc.opt_vertical = 0;
	tbc.opt_helper = 0;
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
		//init_pair(TEXT_DEFAULT, -1, -1);
		init_pair(BLOCK_RED,	COLOR_RED, COLOR_RED);
		init_pair(BLOCK_GREEN,	COLOR_GREEN, COLOR_GREEN);
		init_pair(BLOCK_BLUE,	COLOR_BLUE, COLOR_BLUE);
		init_pair(BLOCK_YELLOW,	COLOR_YELLOW, COLOR_YELLOW);
		init_pair(TEXT_RED,	COLOR_RED, -1);
		init_pair(TEXT_GREEN,	COLOR_GREEN, -1);
		init_pair(TEXT_BLACK,	COLOR_BLACK, -1);
		init_pair(BACK_YELLOW,	COLOR_BLACK, COLOR_YELLOW);
	}
}


/* tbc_configure - setup layout and all those things, will be called from
 * modules, res is the time resolution 3 = seconds, 4 = tenth of seconds */
void
tbc_configure(unsigned short resy, unsigned short resx, short y_offset,
		unsigned short min_w, unsigned short min_h, 
		unsigned short pmin_w, unsigned short pmin_h)
{
	/* terminal is too small, removing frame & border */
	if (tbc.height < pmin_h || tbc.width < pmin_w)
		tbc.opt_frame = tbc.opt_border = 0;

	/* term too small for mod_clock */
	if (tbc.height < min_h || tbc.width < min_w) {
		char s_err[72];
		snprintf(s_err, 72, ERR_TSIZE, min_w, min_h);
		tbc_fatal(s_err);
	}

	/* Calculate sizes and margins... */
	if (tbc.opt_frame) {
		tbc.dot_h = tbc.height / (resy * 2);
		if (tbc.dot_h < 1)
			tbc.dot_h = 1;

		tbc.dot_w = tbc.width / (resx * 2); 
		if (tbc.dot_w < 1)
			tbc.dot_w = 1;

		if (tbc.opt_border) {
			tbc.dot_sh = tbc.height / (resy * 4);
			tbc.dot_sw = tbc.width / (resx * 3);
		} else
			tbc.dot_sh = tbc.dot_sw = 0;

		tbc.top_margin = (tbc.height - resy * tbc.dot_h 
				- tbc.dot_sh * (resy - 1)) / 2 + y_offset;
		tbc.left_margin = (tbc.width - resx * tbc.dot_w 
				- tbc.dot_sw * (resx - 1)) / 2;
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

	while ((ch = getopt(ac, av, "hHvfbdag:m:")) != -1) {
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
		case 'd':
			tbc.opt_dots = 0;
			break;
		case 'a':
			tbc.opt_vertical = 1;
			break;
		case 'H':
			tbc.opt_helper++;
			break;
		case 'g':
		case 'm':
			modulename = optarg;
			break;
		case 'h':
		default:
			fprintf(stderr, USAGE_FMT, av[0]);
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

