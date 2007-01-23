/* $Id: mod_chrono.c,v 1.2 2007-01-23 14:08:41 tamentis Exp $
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
#include <sys/time.h>

#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <time.h>

#include "tbclock.h"

#define ERR_TSIZE "The minimal allowed terminal size is 6x4 for 'chrono'!"

extern TBC tbc;

/* returns the number of tenth of seconds since start of program (bigbang) */
unsigned long
chrono_nds()
{
	struct timeval tp;
	struct timezone tzp;
	unsigned long dsec = 0;

	gettimeofday(&tp, &tzp);

	dsec = (tp.tv_sec - tbc.bigbang) * 10 + (tp.tv_usec / 100000);

	return (dsec);
}


/* mod_chrono */
void
mod_chrono()
{
	unsigned long start, elapsed, paused = 0;
	int pause = 0;

	if (tbc.height < 6 || tbc.width < 8)
		tbc.opt_frame = tbc.opt_border = 0;

	if (tbc.height < 4 || tbc.width < 6)
		tbc_fatal(ERR_TSIZE);

	tbc_configure(4, 0);

	start = chrono_nds();

	for (;;) {
		unsigned int hour, min, sec, dsec;
		signed char c;

		c = getch();

		if (c == KB_SPACE) { /* space bar */
			if (pause == 0) { /* pause */
				pause = 1;
				paused = chrono_nds();
			} else { /* unpause */
				pause = 0;
				wbkgdset(tbc.screen, COLOR_PAIR(TEXT_DEFAULT));
				mvwprintw(tbc.screen, 2, tbc.width / 2 - 6, 
						"             ");
				start = start + chrono_nds() - paused;
			}
		} else if (c == KB_BACKSPACE) { /* backspace */
			if (pause) {
				tbc_clear_innerzone();
				tbc_refresh();
				paused = start;
			} else {
				start = chrono_nds();
			}
		} else if (c != -1)
			return;

		if (!pause) {
			elapsed = chrono_nds() - start;

			hour = elapsed / 36000;
			min  = (elapsed % 36000) / 600;
			sec  = (elapsed % 36000) % 600 / 10;
			dsec = (elapsed % 36000) % 600 % 10;

			tbc_draw_time(4, hour, min, sec, dsec);
		} else {
			wbkgdset(tbc.screen, COLOR_PAIR(BACK_YELLOW));
			mvwprintw(tbc.screen, 2, tbc.width / 2 - 6, "--- PAUSE ---");
		}

		tbc_refresh();

		usleep(10000);
	}
}


