/* $Id: mod_guessbin.c,v 1.1 2007-01-23 13:29:04 tamentis Exp $
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

#include "tbclock.h"

#define	TIME_EASY	15
#define TIME_NORMAL	10
#define TIME_HARD	5
#define TICK		1000
#define SECOND		100000

#define QUESTION_LEFT	5
#define PROMPT_LEFT	21
#define SCORE_TOP	5
#define PROMPT_BLANK	"         "
#define COMMENT_BLANK	"                      "
#define COMMENT_TIMEOUT	"Time out! Next! Quick!"
#define COMMENT_BAD	"Incorrect! Try harder!"
#define COMMENT_GOOD	"Correct! Try this one!"
#define TXT_TITLE	"tbclock/guessbin: Choose your difficulty"
#define TXT_HELP1	"Use your keyboard arrows to select your difficulty,"
#define TXT_HELP2	"the harder it get, the faster it goes but the more"
#define TXT_HELP3	"points you receive! You will have to read 20 times"
#define TXT_HELP4	"the time in binary form, type it in decimal with"
#define TXT_HELP5	"the form HH:MM:SS."

#define SCORE_TITLE	"You scored %ld points!"
#define SCORE_TIME	"TIME BONUS!          +1000 pts"
#define SCORE_DIFF	"Difficulty bonus!    +%4u pts"
#define SCORE_GOOD	"Good shots           +%4u pts"
#define SCORE_OOPS	"Mistakes             -%4u pts"
#define SCORE_LINE	"================================"

#define ERR_TSIZE "The minimal allowed terminal size is 53x17 for 'guessbin'!"


static char s_diff[4][7] = { "", "Easy", "Normal", "Hard" };

struct guessbin {
	unsigned int h, m, s;
	time_t t_start, t_end, t_allowed, t_elapsed, t_used;
	unsigned int q_tot, q_cur, q_ok, q_err;
	unsigned int diff;
};

extern TBC tbc;


void
guessbin_score(struct guessbin *g)
{
	char c;
	char s_score[32];
	char s_bonus[32];
	int l, line = 0;
	long points = 0;

	if ((g->t_used / g->q_tot) <= (g->t_allowed / 2)) {
		points += 1000;
		mvwprintw(tbc.screen, tbc.height / 2 - SCORE_TOP + line,
				tbc.width / 2 - 15, SCORE_TIME);
		line++;
	}

	if (g->diff > 1 && g->q_ok > 0) {
		points += 1000 * g->diff * (g->q_tot/g->q_ok);
		snprintf(s_bonus, 32, SCORE_DIFF, 1000 * g->diff);
		mvwprintw(tbc.screen, tbc.height / 2 - SCORE_TOP + line,
				tbc.width / 2 - 15, s_bonus);
		line++;
	}

	if (g->q_ok > 0) {
		points += 200 * g->q_ok;
		snprintf(s_bonus, 32, SCORE_GOOD, 200 * g->q_ok);
		mvwprintw(tbc.screen, tbc.height / 2 - SCORE_TOP + line,
				tbc.width / 2 - 15, s_bonus);
		line++;
	}

	if (g->q_err > 0) {
		points -= 100 * g->q_err;
		if (points < 0) points = 0;
		snprintf(s_bonus, 32, SCORE_OOPS, 100 * g->q_err);
		mvwprintw(tbc.screen, tbc.height / 2 - SCORE_TOP + line,
				tbc.width / 2 - 15, s_bonus);
		line++;
	}

	mvwprintw(tbc.screen, tbc.height / 2 - SCORE_TOP + line, 
			tbc.width / 2 - 16, SCORE_LINE);
	l = snprintf(s_score, 32, SCORE_TITLE, points);
	wbkgdset(tbc.screen, COLOR_PAIR(TEXT_RED));
	mvwprintw(tbc.screen, tbc.height / 2 - SCORE_TOP + line + 1,
			tbc.width / 2 - l / 2, s_score);

	for (;;) {
		c = getch();

		if (c != -1)
			break;

		tbc_refresh();
		usleep(TICK);
	}

	free(g);
}


/* guessbin_next_digit - takes a look at the first two chars of the
 * provided string *c store (maybe) the 1 or 2 digit in *t */
unsigned int
guessbin_next_digit(unsigned char *c, unsigned int *t)
{
	unsigned int offset = 0;

	if (*(c+1) != ':' && *(c+1) != 0) {
		*t = (*(c) - '0') * 10 + *(c+1) - '0';
		offset = 3;
	} else {
		*t = *(c) - '0';
		offset = 2;
	}

	return (offset);
}


/* guessbin_matches - check wether the string in *is (HH:MM:SS) is
 * equal to the three integer provided as a time. */
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


/* guessbin_shuffle - get a new time and draw the new blocks */
void
guessbin_shuffle(struct guessbin *g)
{
	srand(time(NULL));

	g->h = rand()%23;
	g->m = rand()%59;
	g->s = rand()%59;

	tbc_draw_time(3, g->h, g->m, g->s, 0);
}


/* guessbin_menu_diff - draw the difficulty menu, highlighting the option
 * given by 'highlight' */
void
guessbin_menu_diff(int highlight)
{

	if (highlight == 1)
		wbkgdset(tbc.screen, COLOR_PAIR(BACK_YELLOW));
	else
		wbkgdset(tbc.screen, COLOR_PAIR(BACK_DEFAULT));
	mvwprintw(tbc.screen, tbc.height/2+1, tbc.width/2-14, "[ Easy ]");

	wbkgdset(tbc.screen, COLOR_PAIR(0));
	mvwprintw(tbc.screen, tbc.height/2+1, tbc.width/2-6, " ");

	if (highlight == 2)
		wbkgdset(tbc.screen, COLOR_PAIR(BACK_YELLOW));
	else
		wbkgdset(tbc.screen, COLOR_PAIR(BACK_DEFAULT));
	mvwprintw(tbc.screen, tbc.height/2+1, tbc.width/2-5, "[ Normal ]");

	wbkgdset(tbc.screen, COLOR_PAIR(0));
	mvwprintw(tbc.screen, tbc.height/2+1, tbc.width/2+5, " ");

	if (highlight == 3)
		wbkgdset(tbc.screen, COLOR_PAIR(BACK_YELLOW));
	else
		wbkgdset(tbc.screen, COLOR_PAIR(BACK_DEFAULT));
	mvwprintw(tbc.screen, tbc.height/2+1, tbc.width/2+6, "[ Hard ]");

	wbkgdset(tbc.screen, COLOR_PAIR(BACK_DEFAULT));

}


struct guessbin *
guessbin_init() 
{
	struct guessbin *g;
	char c;
	int hh, hw;

	if (tbc.width < 53 || tbc.height < 17)
		tbc_fatal(ERR_TSIZE);

	/* Set up what never moves... */
	wbkgdset(tbc.screen, COLOR_PAIR(TEXT_DEFAULT));
	mvwprintw(tbc.screen, tbc.height-1, tbc.width-18, "Ctrl-C to Quit");

	g = malloc(sizeof(struct guessbin));
	g->q_tot = 2;
	g->q_cur = 0;
	g->q_ok  = 0;
	g->q_err = 0;
	g->diff  = 1;

	/* Ask for difficulty */
	hh = tbc.height / 2;
	hw = tbc.width / 2;
	mvwprintw(tbc.screen, hh - 1, hw - 20, TXT_TITLE);
	mvwprintw(tbc.screen, hh + 3, hw - 25, TXT_HELP1);
	mvwprintw(tbc.screen, hh + 4, hw - 25, TXT_HELP2);
	mvwprintw(tbc.screen, hh + 5, hw - 25, TXT_HELP3);
	mvwprintw(tbc.screen, hh + 6, hw - 25, TXT_HELP4);
	mvwprintw(tbc.screen, hh + 7, hw - 25, TXT_HELP5);
	guessbin_menu_diff(g->diff);
	for (;;) {
		c = getch();

		if (c == 0x44)
			guessbin_menu_diff(g->diff > 1 ? --(g->diff) : g->diff);
		else if (c == 0x43)
			guessbin_menu_diff(g->diff < 3 ? ++(g->diff) : g->diff);
		else if (c == 0x0A) {
			tbc_clear_innerzone();
			break;
		}

		tbc_refresh();

		usleep(TICK);
	}

	g->t_start = time(NULL);
	g->t_used = 0;
	g->t_elapsed = 0;
	if (g->diff == 3) {
		g->t_allowed = TIME_HARD;
		g->t_end = g->t_start + TIME_HARD;
	} else if (g->diff == 2) {
		g->t_allowed = TIME_NORMAL;
		g->t_end = g->t_start + TIME_NORMAL;
	} else {
		g->t_allowed = TIME_EASY;
		g->t_end = g->t_start + TIME_EASY;
	}

	return (g);
}


/* guessbin_time - this displays the two progressbar! */
void
guessbin_timeline(unsigned long current, unsigned long total)
{
	int i, sh;
	float ratio;

	ratio = (float) current / (float)total;
	sh = (tbc.height - 2) * (1 - ratio);

	for (i = 1; i < tbc.height - 1; i++) {
		if (i < sh + 1)
			wbkgdset(tbc.screen, COLOR_PAIR(BLOCK_DEFAULT));
		else
			wbkgdset(tbc.screen, COLOR_PAIR(BLOCK_GREEN));
		mvwprintw(tbc.screen, i, tbc.width - 3, "  ");
		mvwprintw(tbc.screen, i, 1, "  ");
	}

	wbkgdset(tbc.screen, COLOR_PAIR(BACK_DEFAULT));
}


void
guessbin_timeout(struct guessbin *g)
{
	wbkgdset(tbc.screen, COLOR_PAIR(TEXT_RED));
	mvwprintw(tbc.screen, tbc.height - 4, QUESTION_LEFT, COMMENT_TIMEOUT);
	guessbin_shuffle(g);

	g->t_used += g->t_allowed;
	g->t_start = time(NULL);
	g->t_end = g->t_start + g->t_allowed;
	g->t_elapsed = 0;
	g->q_cur++;
}


void
guessbin_incorrect(struct guessbin *g)
{
	wbkgdset(tbc.screen, COLOR_PAIR(TEXT_RED));
	mvwprintw(tbc.screen, tbc.height - 4, QUESTION_LEFT, COMMENT_BAD);
	g->q_err++;
}


void
guessbin_correct(struct guessbin *g)
{
	time_t now;

	now = time(NULL);

	wbkgdset(tbc.screen, COLOR_PAIR(TEXT_GREEN));
	mvwprintw(tbc.screen, tbc.height - 4, QUESTION_LEFT, COMMENT_GOOD);
	guessbin_shuffle(g);

	g->t_used += now - g->t_start;
	g->t_elapsed = 0;
	g->t_start = now;
	g->t_end = g->t_start + g->t_allowed;
	g->q_cur++;
	g->q_ok++;
}

void
guessbin_clearprompt(int *size)
{
	wbkgdset(tbc.screen, COLOR_PAIR(TEXT_DEFAULT));
	mvwprintw(tbc.screen, tbc.height-3, PROMPT_LEFT, PROMPT_BLANK);
	move(tbc.height-3, 19);
	*size = 0;
}


/* game_guessbin - main part, setup and loop */
void
mod_guessbin()
{
	char c;
	struct guessbin *g;
	unsigned char is[8];
	unsigned char status[32];
	int size = 0;

	tbc_configure(3, -1);
	g = guessbin_init();

	/* Game loop */
	mvwprintw(tbc.screen, tbc.height - 3, QUESTION_LEFT, 
			"Guess the time: ");
	guessbin_shuffle(g);
	for (;;) {
		time_t now;

		c = getch();
		now = time(NULL);

		snprintf((char*)status, 32, "%s [%u/%u]", s_diff[g->diff], 
				g->q_cur, g->q_tot);
		mvwprintw(tbc.screen, tbc.height-1, 4, (char*)status);

		guessbin_timeline(g->t_end - now, g->t_end - g->t_start);

		if ( g->t_end <= now) {
			guessbin_timeout(g);
			guessbin_clearprompt(&size);
		}

		if (g->q_cur >= g->q_tot)
			break;

		if (c == KB_RETURN) { /* Enter, matches ? */
			if (guessbin_matches(is, size, g->h, g->m, g->s)) {
				guessbin_correct(g);
			} else {
				guessbin_incorrect(g);
			}

			guessbin_clearprompt(&size);

		} else if (c == KB_BACKSPACE) { /* Backspace */
			if (size < 1)
				continue;
			is[size-1] = 0;
			size--;
			wbkgdset(tbc.screen, COLOR_PAIR(TEXT_DEFAULT));
			mvwprintw(tbc.screen, tbc.height - 3, PROMPT_LEFT, 
					"        ");
			mvwprintw(tbc.screen, tbc.height - 3, PROMPT_LEFT, 
					(char*)is);

		} else if (c == KB_CLEAR) { /* ^U */
			guessbin_clearprompt(&size);
	
		} else if (c > 47 && c < 59 && size < 8) { /* Number or ':' */
			is[size] = c;
			is[size+1] = 0;
			size++;
			wbkgdset(tbc.screen, COLOR_PAIR(TEXT_DEFAULT));
			mvwprintw(tbc.screen, tbc.height - 4, QUESTION_LEFT, 
					COMMENT_BLANK);
			mvwprintw(tbc.screen, tbc.height - 3, PROMPT_LEFT, 
					(char*)is);
		}

		tbc_refresh();

		usleep(TICK);
	}

	/* Show score */
	tbc_clear_innerzone();
	guessbin_score(g);

}

