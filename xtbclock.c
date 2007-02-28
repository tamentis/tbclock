/* $Id: xtbclock.c,v 1.1 2007-02-28 12:47:35 tamentis Exp $
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

/* THIS FILE IS WORK IN PROGRESS... */

#include <X11/Xlib.h>

#include <err.h>

//#include "tbclock.h"


int
main(int ac, char **av)
{
	Display *dpy;
	Window root_win;
	GC gc;

	dpy = XOpenDisplay(":0");
	if (dpy == NULL) {
		errx(-1, "Unable to connect to X server.");
	}
	
	root_win = DefaultRootWindow(dpy);

	GC green_gc;
	XColor green_col;
	Colormap colormap;
	char green[] = "#00FF00";

	colormap = DefaultColormap(dpy, 0);
	green_gc = XCreateGC(dpy, root_win, 0, 0);

	XParseColor(dpy, colormap, green, &green_col);
	XAllocColor(dpy, colormap, &green_col);
	XSetForeground(dpy, green_gc, green_col.pixel);

	XDrawRectangle(dpy, root_win, green_gc, 1, 1, 497, 497);
	XDrawRectangle(dpy, root_win, green_gc, 50, 50, 398, 398);
	XFillRectangle(dpy, root_win, green_gc, 60, 150, 50, 60);

	XFlush(dpy);

	sleep(5);
	
	XCloseDisplay(dpy);

	return (0);
}

