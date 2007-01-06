/*
 * This is tbclock (Tamentis Binary Clock).
 * $Id: tbclock.c,v 1.1.1.1 2007-01-06 09:45:42 tamentis Exp $
 *
 * Compile this with :
 * 	gcc -lncurses tbclock.c -Wall -O -o tbclock
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
 */

#include <sys/ioctl.h>

#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <stdlib.h>

static WINDOW *mainwnd;
static WINDOW *screen;
int now_sec, now_min, now_hour;

void
check_res()
{
	struct winsize ws;

	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1) {
		if (ws.ws_row != 24 || ws.ws_col != 80) {
			fprintf(stderr, "Only works on 80x24 terminals.\n");
			exit(-1);
		}
	}
}


void
screen_init(void)
{
	mainwnd = initscr();
	noecho();
	cbreak();
	nodelay(mainwnd, TRUE);
	refresh();
	wrefresh(mainwnd);
	screen = newwin(24, 80, 0, 0);
	box(screen, ACS_VLINE, ACS_HLINE);
}


void
print_one_dot(int x, int y, WINDOW *screen, short color)
{
	wbkgdset(screen, COLOR_PAIR(color));
	mvwprintw(screen, y, x,"      ");
	mvwprintw(screen, y+1, x,"      ");
	mvwprintw(screen, y+2, x,"      ");
}

void
dot(WINDOW *screen, int valid, int x, int y, short color)
{
	if (valid) 
		print_one_dot(x, y, screen, color);
	else
		print_one_dot(x, y, screen, 0);
}


void
update_display(void)
{
	check_res();

	/* Hour ... in blue */
	init_pair(1, COLOR_BLUE, COLOR_BLUE);
	dot(screen, now_hour&16, 22, 5, 1);
	dot(screen, now_hour&8,  32, 5, 1);
	dot(screen, now_hour&4,  42, 5, 1);
	dot(screen, now_hour&2,  52, 5, 1);
	dot(screen, now_hour&1,  62, 5, 1);

	/* Minutes ... in red */
	init_pair(2, COLOR_RED, COLOR_RED);
	dot(screen, now_min&32, 12, 10, 2);
	dot(screen, now_min&16, 22, 10, 2);
	dot(screen, now_min&8,  32, 10, 2);
	dot(screen, now_min&4,  42, 10, 2);
	dot(screen, now_min&2,  52, 10, 2);
	dot(screen, now_min&1,  62, 10, 2);

	/* Seconds... yellow */
	init_pair(3, COLOR_YELLOW, COLOR_YELLOW);
	dot(screen, now_sec&32, 12, 15, 3);
	dot(screen, now_sec&16, 22, 15, 3);
	dot(screen, now_sec&8,  32, 15, 3);
	dot(screen, now_sec&4,  42, 15, 3);
	dot(screen, now_sec&2,  52, 15, 3);
	dot(screen, now_sec&1,  62, 15, 3);

	wrefresh(screen);
	refresh();
}


void screen_end(void) {
   endwin();
}


void
maketime(void)
{
	time_t now;
	struct tm *now_tm;

	now = time (NULL);
	now_tm = localtime (&now);
	now_sec = now_tm->tm_sec;
	now_min = now_tm->tm_min;
	now_hour = now_tm->tm_hour;
}


int
main(int ac, char **av)
{
	screen_init();
	start_color();

	for (;;) {
		if (getch() != -1)
			break;
		maketime();
		update_display();
		sleep(1);
	}

	screen_end();

	return (0);
}
