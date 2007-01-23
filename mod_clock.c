/* $Id: mod_clock.c,v 1.1 2007-01-23 13:29:04 tamentis Exp $
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
#include <time.h>

#include "tbclock.h"

#define ERR_TSIZE "The minimal allowed terminal size is 6x3 for 'clock'!"

extern TBC tbc;

/* mod_clock */
void
mod_clock()
{
	time_t now;
	struct tm *tm;

	/* terminal is too small, removing frame & border */
	if (tbc.height < 5 || tbc.width < 8)
		tbc.opt_frame = tbc.opt_border = 0;

	/* term too small for mod_clock */
	if (tbc.height < 3 || tbc.width < 6)
		tbc_fatal(ERR_TSIZE);

	tbc_configure(3, 0);

	for (;;) {
		if (getch() != -1)
			break;

		now = time(NULL);
		tm = localtime(&now);

		tbc_draw_time(3, tm->tm_hour, tm->tm_min, tm->tm_sec, 0);

		tbc_refresh();

		usleep(10000);
	}
}

