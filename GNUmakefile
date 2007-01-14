# $Id: GNUmakefile,v 1.1 2007-01-14 22:42:46 tamentis Exp $
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

CC = cc
CFLAGS = -Wall -O
LIBFLAGS = -lncurses
OBJ = tbclock.o guessbin.o
MANDIR=		/usr/local/man

ifeq ($(OS),Linux)
MANDIR=		/usr/share/man
endif


all: tbclock

tbclock: tbclock.o guessbin.o
	$(CC) $(CFLAGS) $(LIBFLAGS) $(OBJ) -o tbclock

.c.o: 
	$(CC) -c $(CFLAGS) $<

install:
	install -m 755 tbclock /usr/local/bin/
	install -m 644 tbclock.1 $(MANDIR)/man1/

deinstall:
	rm -f /usr/local/bin/tbclock
	rm -f $(MANDIR)/man1/tbclock.1

clean:
	rm -f tbclock $(OBJ)
