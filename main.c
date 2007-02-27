/* $Id: main.c,v 1.5 2007-02-27 09:28:53 tamentis Exp $
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


#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <curses.h>

#include "tbclock.h"


TBC tbc;


/* tbc_next_help_value - rotate through different reading helps */
void
tbc_next_help_value()
{
	unsigned int tm, lm;

	tm = tbc.top_margin + tbc.dot_h / 2;
	lm = tbc.width - tbc.left_margin + tbc.left_margin / 2 - 1;

	if (tbc.opt_helper > 2)
		tbc.opt_helper = 0;
	else    
		tbc.opt_helper++;

}


/* tbc_set_default - set the starting vars, default options */
void
tbc_set_default()
{
	tbc.bigbang = time(NULL);

	tbc.width = 80;
	tbc.height = 24;

	tbc.opt_frame = tbc.org_frame = 1;
	tbc.opt_border = tbc.org_border = 1;
	tbc.opt_dots = 1;
	tbc.opt_vertical = 0;
	tbc.opt_helper = 0;

	tbc.col_h = COLOR_BLUE;
	tbc.col_m = COLOR_RED;
	tbc.col_s = COLOR_YELLOW;
	tbc.col_t = COLOR_GREEN;
}



/* tbc_configure - setup layout and all those things, could be called from
 * anywhere, when term is resized, at the beginning of modules... it
 * deals with weither something is tolerated or not depending of
 * a size. */
void
tbc_configure()
{

	/* terminal is too small, removing frame & border */
	if (tbc.height < 10 || tbc.width < 19)
		tbc.opt_frame = tbc.opt_border = 0;
	else {
		tbc.opt_frame = tbc.org_frame;
		tbc.opt_border = tbc.org_border;
	}

	/* term not wide enough for horizontal helper */
	if (!tbc.opt_vertical && tbc.width < 8 && tbc.opt_helper & 1)
		tbc.opt_helper --;

	/* term is not tall enough for bottom help in vertical mode */
	if (tbc.opt_vertical && tbc.height < 5 && tbc.opt_helper > 0)
		tbc.opt_helper = 0;

	/* term is not tall enough for bottom help */
	if (tbc.height < 4 && tbc.opt_helper > 1)
		tbc.opt_helper -= 2;

	/* height of a dot (term height - frame - spaces - helper) */
	tbc.dot_h = (tbc.height
			- tbc.opt_frame * 2
			- (tbc.res_y - 1) * tbc.opt_border
			- (!tbc.opt_vertical && tbc.opt_helper > 1 ? 1 : 0)
			- (tbc.opt_vertical && tbc.opt_helper ? 1 : 0)
		    ) / tbc.res_y;

	/* width of a dot */
	tbc.dot_w = (tbc.width
			- tbc.opt_frame * 2
			- (tbc.res_x - 1) * tbc.opt_border
			- (!tbc.opt_vertical && tbc.opt_helper & 1 ? 2 : 0)
		    ) / tbc.res_x;

	/* top margin */
	tbc.top_margin = tbc.opt_frame +
		(tbc.height - tbc.opt_frame * 2 - tbc.res_y * tbc.dot_h
		 - (tbc.res_y - 1) * tbc.opt_border
		 - (!tbc.opt_vertical && tbc.opt_helper & 1 ? 2 : 0)
		) / 2;

	/* left margin */
	tbc.left_margin = tbc.opt_frame + 
		(tbc.width - tbc.opt_frame * 2 - tbc.res_x * tbc.dot_w
		 - (tbc.res_x - 1) * tbc.opt_border
		 - (!tbc.opt_vertical && tbc.opt_helper & 1 ? 2 : 0)
		) / 2;

	tbc_clear();

}


/* this doesn't do anything, I was thinking about removing it. */
#define SET_COLOR(x) i = atoi(optarg); if (i >= 0 && i < 8) x = i;
int
main(int ac, char **av)
{
	int ch, i;
	char modulename[9] = "clock";

	tbc_set_default();

	while ((ch = getopt(ac, av, "abdefg:hvm:H:M:S:T:")) != -1) {
		switch (ch) {
		case 'a':
			tbc.opt_vertical = 1;
			break;
		case 'b':
			tbc.opt_border = tbc.org_frame = 0;
			break;
		case 'd':
			tbc.opt_dots = 0;
			break;
		case 'e':
			tbc.opt_helper++;
			break;
		case 'f':
			tbc.opt_frame = tbc.org_frame = 0;
			break;
		case 'g':
		case 'm':
			strlcpy(modulename, optarg, 9);
			break;
		case 'H':
			SET_COLOR(tbc.col_h);
			break;
		case 'M':
			SET_COLOR(tbc.col_m);
			break;
		case 'S':
			SET_COLOR(tbc.col_s);
			break;
		case 'T':
			SET_COLOR(tbc.col_t);
			break;
		case 'v':
			printf(TBCCOPY);
			exit(-1);
		case 'h':
		default:
			printf(USAGE_FMT, av[0]);
			exit(-1);
		}
	}
	ac -= optind;
	av += optind;

	tbc_display_init();

	/* module selection */
	if (strncmp(modulename, "guessbin", 9) == 0) {
		tbc.opt_frame = 1;
		tbc.opt_border = 1;
		mod_guessbin();
	} else if (strncmp(modulename, "chrono", 7) == 0) {
		mod_chrono();
	} else {
		mod_clock();
	}

	endwin();
	printf("Thank you for using tbclock!\n");

	return (0);
}

