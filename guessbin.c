/* $Id: guessbin.c,v 1.1 2007-01-14 22:42:46 tamentis Exp $
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

#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <time.h>


extern WINDOW *screen;
extern int top_margin, left_margin;
extern int dot_w, dot_h;
extern int dot_sw, dot_sh;
extern int height, width;
extern int color;


void line(WINDOW *, int, int, short);


unsigned int
guessbin_next_digit(unsigned char *c, unsigned int *t)
{
	unsigned int offset = 0;

	if (*(c+1) != ':' && *(c+1) != 0) {
		*t = (*(c) - 48) * 10 + *(c+1) - 48;
		offset = 3;
	} else {
		*t = *(c) - 48;
		offset = 2;
	}

	return (offset);
}


int
guessbin_matches(unsigned char *is, int size, int h, int m, int s)
{
	unsigned int uh, um, us;

	if (size < 5)
		return (0);

	if (*is == ':')
		return (0);

	/* Hours */
	is += guessbin_next_digit(is, &uh);
	if (uh != h) return (0);

	/* Minutes */
	is += guessbin_next_digit(is, &um);
	if (um != m) return (0);

	/* Seconds */
	guessbin_next_digit(is, &us);
	if (us != s) return (0);

	return (1);
}


void
guessbin_shuffle(unsigned int *h, unsigned int *m, unsigned int *s)
{
	srand(time(NULL));

	*h = rand()%23;
	*m = rand()%59;
	*s = rand()%59;

	line(screen, *h, top_margin, 1);
	line(screen, *m, top_margin + dot_h + dot_sh, 2);
	line(screen, *s,  top_margin + (dot_h+dot_sh)*2, 3);
}


void
game_guessbin()
{
	char c;
	unsigned char is[8];
	unsigned int h, m, s;
	int size = 0;

	if (width < 30 || height < 9) {
		endwin();
		fprintf(stderr, "Your terminal is too small for this game.\n");
		exit(-1);
	}

	/* Set up what never moves... */
	wbkgdset(screen, COLOR_PAIR(0));
	mvwprintw(screen, height-1, width-17, "Ctrl-C to Quit");
	mvwprintw(screen, height-3, 3, "Guess the time: ");

	/* First random time */
	guessbin_shuffle(&h, &m, &s);

	/* l00p */
	for (;;) {
		c = getch();

		/* Enter is pressed, check if it matches... */
		if (c == 0x0A) {
			if (guessbin_matches(is, size, h, m, s)) {
				wbkgdset(screen, COLOR_PAIR(5));
				mvwprintw(screen, height-4, 3, 
						"Correct! Try this one...");
				guessbin_shuffle(&h, &m, &s);
			} else {
				wbkgdset(screen, COLOR_PAIR(4));
				mvwprintw(screen, height-4, 3, 
						"INCORRECT!              ");
			}

			wbkgdset(screen, COLOR_PAIR(0));
			mvwprintw(screen, height-3, 19, 
					"                        ");
			move(height-3, 19);
			size = 0;

		/* Backspace is pressed */
		} else if (c == 0x7F) {
			if (size < 1)
				continue;
			is[size-1] = 0;
			size--;
			wbkgdset(screen, COLOR_PAIR(0));
			mvwprintw(screen, height-3, 19, "        ");
			mvwprintw(screen, height-3, 19, (char*)is);

		/* ^U is pressed (erase everything) */
		} else if (c == 0x15) {
			wbkgdset(screen, COLOR_PAIR(0));
			mvwprintw(screen, height-3, 19, 
					"                        ");
			move(height-3, 19);
			size = 0;
	
		/* A good number or ':' is entered */
		} else if (c > 47 && c < 59 && size < 8) {
			is[size] = c;
			is[size+1] = 0;
			size++;
			wbkgdset(screen, COLOR_PAIR(0));
			mvwprintw(screen, height-4, 3, "          ");
			mvwprintw(screen, height-3, 19, (char*)is);
		}
		/* Debug keys...*/
		/*else if (c != -1) {
			char truc[10];
			snprintf(truc, 10, "%hhu", c);
			mvwprintw(screen, 10, 10, truc);
		}*/

		wrefresh(screen);
		refresh();

		usleep(100);
	}

}

