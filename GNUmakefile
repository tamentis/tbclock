# $Id: GNUmakefile,v 1.5 2007-02-27 09:28:53 tamentis Exp $
#
# Copyright (c) 2007 Bertrand Janin <tamentis@neopulsar.org>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
# 

CC=		cc
OS:=		$(shell uname)
CFLAGS=		-Wall -O
LIBFLAGS=	-lncurses
BEAST=		tbclock
OBJ=		main.o draw.o mod_clock.o mod_chrono.o mod_guessbin.o
MANDIR=		/usr/local/man

ifeq ($(OS),Linux)
MANDIR=		/usr/share/man
endif


all: $(BEAST)

tbclock: $(OBJ)
	$(CC) $(CFLAGS) $(LIBFLAGS) $(OBJ) -o $(BEAST)

.c.o: tbclock.h
	$(CC) -c $(CFLAGS) $<

install:
	install -m 755 $(BEAST) /usr/local/bin/
	install -m 644 $(BEAST).1 $(MANDIR)/man1/

deinstall:
	rm -f /usr/local/bin/$(BEAST)
	rm -f $(MANDIR)/man1/$(BEAST).1

clean:
	rm -f $(BEAST) $(OBJ)
