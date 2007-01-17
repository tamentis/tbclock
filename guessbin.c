/* $Id: guessbin.c,v 1.2 2007-01-17 08:24:58 tamentis Exp $
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

#define	TIME_EASY	1500000
#define TIME_NORMAL	1000000
#define TIME_HARD	 500000
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
#define TXT_HELP1	"Use your keyboard arrows to select your difficulty,"
#define TXT_HELP2	"the harder it get, the faster it goes but the more"
#define TXT_HELP3	"points you receive! You will have to read 20 times"
#define TXT_HELP4	"the time in binary form, type it in decimal with"
#define TXT_HELP5	"the form HH:MM:SS."

static char s_diff[4][7] = { "", "Easy", "Normal", "Hard" };


void line(WINDOW *, int, int, short);
void clear_innerzone(void);

struct guessbin {
	unsigned int h, m, s;
	unsigned long t_allowed, t_elapsed, t_used;
	unsigned int q_tot, q_cur, q_ok, q_err;
	unsigned int diff;
};



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
		mvwprintw(screen, height / 2 - SCORE_TOP + line, width / 2 - 15, 
				"TIME BONUS!          +1000 pts");
		line++;
	}

	if (g->diff > 1 && g->q_ok > 0) {
		points += 1000 * g->diff * (g->q_tot/g->q_ok);
		snprintf(s_bonus, 32, "Difficulty bonus!    +%u pts", 
				1000 * g->diff);
		mvwprintw(screen, height / 2 - SCORE_TOP + line, width / 2 - 15,
				s_bonus);
		line++;
	}

	if (g->q_ok > 0) {
		points += 200 * g->q_ok;
		snprintf(s_bonus, 32, "Good shots           +%4u pts", 
				200 * g->q_ok);
		mvwprintw(screen, height / 2 - SCORE_TOP + line, width / 2 - 15, 
				s_bonus);
		line++;
	}

	if (g->q_err > 0) {
		points -= 100 * g->q_err;
		if (points < 0) points = 0;
		snprintf(s_bonus, 32, "Mistakes             -%4u pts",
				100 * g->q_err);
		mvwprintw(screen, height / 2 - SCORE_TOP + line, width / 2 - 15,
				s_bonus);
		line++;
	}

	mvwprintw(screen, height / 2 - SCORE_TOP + line, width / 2 - 16, 
			"================================");
	l = snprintf(s_score, 32, "You scored %ld points!", points);
	wbkgdset(screen, COLOR_PAIR(4));
	mvwprintw(screen, height / 2 - SCORE_TOP + line + 1, width / 2 - l / 2,
			s_score);

	for (;;) {
		c = getch();

		if (c != -1)
			break;

		wrefresh(screen);
		refresh();
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

	line(screen, g->h, top_margin, 1);
	line(screen, g->m, top_margin + dot_h + dot_sh, 2);
	line(screen, g->s,  top_margin + (dot_h+dot_sh)*2, 3);
}


/* guessbin_menu_diff - draw the difficulty menu, highlighting the option
 * given by 'highlight' */
void
guessbin_menu_diff(int highlight)
{

	if (highlight == 1)
		wbkgdset(screen, COLOR_PAIR(6));
	else
		wbkgdset(screen, COLOR_PAIR(0));
	mvwprintw(screen, height/2+1, width/2-14, "[ Easy ]");

	wbkgdset(screen, COLOR_PAIR(0));
	mvwprintw(screen, height/2+1, width/2-6, " ");

	if (highlight == 2)
		wbkgdset(screen, COLOR_PAIR(6));
	else
		wbkgdset(screen, COLOR_PAIR(0));
	mvwprintw(screen, height/2+1, width/2-5, "[ Normal ]");

	wbkgdset(screen, COLOR_PAIR(0));
	mvwprintw(screen, height/2+1, width/2+5, " ");

	if (highlight == 3)
		wbkgdset(screen, COLOR_PAIR(6));
	else
		wbkgdset(screen, COLOR_PAIR(0));
	mvwprintw(screen, height/2+1, width/2+6, "[ Hard ]");

	wbkgdset(screen, COLOR_PAIR(0));

}


struct guessbin *
guessbin_init() 
{
	struct guessbin *g;
	char c;

	if (width < 50 || height < 15) {
		endwin();
		fprintf(stderr, "Your terminal is too small for this game.\n");
		exit(-1);
	}

	/* Set up what never moves... */
	wbkgdset(screen, COLOR_PAIR(0));
	mvwprintw(screen, height-1, width-18, "Ctrl-C to Quit");

	g = malloc(sizeof(struct guessbin));
	g->q_tot = 20;
	g->q_cur = 0;
	g->q_ok  = 0;
	g->q_err = 0;
	g->diff  = 1;

	/* Ask for difficulty */
	mvwprintw(screen, height / 2 - 1, width / 2 - 20, 
			"tbclock/guessbin: Choose your difficulty");
	mvwprintw(screen, height / 2 + 3, width / 2 - 25, TXT_HELP1);
	mvwprintw(screen, height / 2 + 4, width / 2 - 25, TXT_HELP2);
	mvwprintw(screen, height / 2 + 5, width / 2 - 25, TXT_HELP3);
	mvwprintw(screen, height / 2 + 6, width / 2 - 25, TXT_HELP4);
	mvwprintw(screen, height / 2 + 7, width / 2 - 25, TXT_HELP5);
	guessbin_menu_diff(g->diff);
	for (;;) {
		c = getch();

		if (c == 0x44)
			guessbin_menu_diff(g->diff > 1 ? --(g->diff) : g->diff);
		else if (c == 0x43)
			guessbin_menu_diff(g->diff < 3 ? ++(g->diff) : g->diff);
		else if (c == 0x0A) {
			clear_innerzone();
			break;
		}

		wrefresh(screen);
		refresh();

		usleep(TICK);
	}

	g->t_used = 0;
	g->t_elapsed = 0;
	if (g->diff == 3)
		g->t_allowed = TIME_HARD;
	else if (g->diff == 2)
		g->t_allowed = TIME_NORMAL;
	else
		g->t_allowed = TIME_EASY;

	return (g);
}


/* guessbin_time - this displays the two progressbar! */
void
guessbin_timeline(unsigned long current, unsigned long total)
{
	int i, sh;
	float ratio;

	ratio = (float) current / (float)total;
	sh = (height - 2) * (1 - ratio);

	for (i = 1; i < height - 1; i++) {
		if (i < sh + 1)
			wbkgdset(screen, COLOR_PAIR(0));
		else
			wbkgdset(screen, COLOR_PAIR(7));
		mvwprintw(screen, i, width - 3, "  ");
		mvwprintw(screen, i, 1, "  ");
	}

	wbkgdset(screen, COLOR_PAIR(0));
}


void
guessbin_timeout(struct guessbin *g)
{
	wbkgdset(screen, COLOR_PAIR(4));
	mvwprintw(screen, height - 4, QUESTION_LEFT, COMMENT_TIMEOUT);
	guessbin_shuffle(g);

	g->t_used += g->t_allowed;
	g->t_elapsed = 0;
	g->q_cur++;
}


void
guessbin_incorrect(struct guessbin *g)
{
	wbkgdset(screen, COLOR_PAIR(4));
	mvwprintw(screen, height - 4, QUESTION_LEFT, COMMENT_BAD);
	g->q_err++;
}


void
guessbin_correct(struct guessbin *g)
{
	wbkgdset(screen, COLOR_PAIR(5));
	mvwprintw(screen, height - 4, QUESTION_LEFT, COMMENT_GOOD);
	guessbin_shuffle(g);

	g->t_used += g->t_elapsed;
	g->t_elapsed = 0;
	g->q_cur++;
	g->q_ok++;
}

void
guessbin_clearprompt(int *size)
{
	wbkgdset(screen, COLOR_PAIR(0));
	mvwprintw(screen, height-3, PROMPT_LEFT, PROMPT_BLANK);
	move(height-3, 19);
	*size = 0;
}


/* game_guessbin - main part, setup and loop */
void
game_guessbin()
{
	char c;
	struct guessbin *g;
	unsigned char is[8];
	unsigned char status[32];
	int size = 0;

	g = guessbin_init();

	/* Game loop */
	mvwprintw(screen, height-3, QUESTION_LEFT, "Guess the time: ");
	guessbin_shuffle(g);
	for (;;) {
		c = getch();

		snprintf(status, 32, "%s [%u/%u]", s_diff[g->diff], g->q_cur, g->q_tot);
		mvwprintw(screen, height-1, 4, status);

		guessbin_timeline(g->t_allowed - g->t_elapsed, g->t_allowed);
		g->t_elapsed += TICK;

		if ((g->t_allowed - g->t_elapsed) < 2 * TICK) {
			guessbin_timeout(g);
			guessbin_clearprompt(&size);
		}

		if (g->q_cur >= g->q_tot)
			break;

		if (c == 0x0A) { /* Enter, matches ? */
			if (guessbin_matches(is, size, g->h, g->m, g->s)) {
				guessbin_correct(g);
			} else {
				guessbin_incorrect(g);
			}

			guessbin_clearprompt(&size);

		} else if (c == 0x7F) { /* Backspace */
			if (size < 1)
				continue;
			is[size-1] = 0;
			size--;
			wbkgdset(screen, COLOR_PAIR(0));
			mvwprintw(screen, height-3, PROMPT_LEFT, "        ");
			mvwprintw(screen, height-3, PROMPT_LEFT, (char*)is);

		} else if (c == 0x15) { /* ^U */
			guessbin_clearprompt(&size);
	
		} else if (c > 47 && c < 59 && size < 8) { /* Number or ':' */
			is[size] = c;
			is[size+1] = 0;
			size++;
			wbkgdset(screen, COLOR_PAIR(0));
			mvwprintw(screen, height - 4, QUESTION_LEFT, COMMENT_BLANK);
			mvwprintw(screen, height - 3, PROMPT_LEFT, (char*)is);
		}
		/* Debug keys...*/
		/*else if (c != -1) {
			char truc[10];
			snprintf(truc, 10, "%hhu", c);
			mvwprintw(screen, 10, 10, truc);
		}*/

		wrefresh(screen);
		refresh();

		usleep(TICK);
	}
	mvwprintw(screen, 3, 3, "PR00T");

	/* Show score */
	clear_innerzone();
	guessbin_score(g);

}

