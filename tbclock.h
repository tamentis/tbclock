/* $Id: tbclock.h,v 1.4 2007-02-27 09:28:53 tamentis Exp $
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


/* some text constants */
#define TBCVER		"tbclock 1.10"
#define TBCCOPY TBCVER	" - Tamentis Binary Clock (c) 2007 Bertrand Janin\n"
#define ERR_TSIZE	"The minimal terminal size for this module is %hhux%hhu."
#define MSG_UNKNOWNMOD	"I don't know this module (man tbclock).\n"
#define USAGE_FMT	"usage: %s [-abdefhv] [-HMST color] [-m name]\n"


/* color definitions */
#define BLOCK_DEFAULT	0
#define BLOCK_RED	1
#define BLOCK_GREEN	2
#define BLOCK_BLUE	3
#define BLOCK_YELLOW	4
#define TEXT_DEFAULT	0
#define TEXT_RED	10
#define TEXT_GREEN	11
#define TEXT_BLACK	12
#define BACK_DEFAULT	0
#define BACK_YELLOW	20
#define BLOCK_HOUR	30
#define BLOCK_MINUTE	31
#define BLOCK_SECOND	32
#define BLOCK_TENTH	33

/* keyboard definitions */
#define KB_SPACE	0x20
#define KB_BACKSPACE	0x7F
#define KB_RETURN	0x0A
#define KB_CLEAR	0x15
#define KB_LEFT		0x44
#define KB_RIGHT	0x43
#define KB_A		0x61
#define KB_H		0x68
#define KB_R		0x72

/* main and only data type */
typedef struct _tbclock_data {
	WINDOW *screen;
	int now_sec;
	int now_min;
	int now_hour;
	int top_margin;
	int left_margin;
	int dot_w;
	int dot_h;
	int res_x;
	int res_y;
	int height;
	int width;
	int color;
	time_t bigbang;
	int org_frame;
	int opt_frame;
	int org_border;
	int opt_border;
	int opt_dots;
	int opt_vertical;
	int opt_helper;
	int col_h;
	int col_m;
	int col_s;
	int col_t;
} TBC;

/* prototypes for main.c */
void tbc_configure(void);
void tbc_next_help_value();

/* prototypes for draw.c */
void tbc_display_init(void);
void tbc_clear(void);
void tbc_draw_time(int, int, int, int, int);

/* prototypes for mod_*.c */
void mod_guessbin();
void mod_clock();
void mod_chrono();

