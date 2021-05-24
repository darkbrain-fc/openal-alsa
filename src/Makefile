# Copyright (C) 2004 Christopher John Purnell
#                    cjp@lost.org.uk
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

SONAME = libopenal.so.0
SUFFIX = 2.0 

LIB=$(SONAME).$(SUFFIX)
PIC=-fPIC

CC=gcc
CFLAGS=-Os -Wall -W -m32 -Wmissing-prototypes -pedantic $(PIC) 
LIBS=-lasound -lpthread -lm
SHARED=-shared -Wl,-soname,$(SONAME)
CPPFLAGS=-I../include

OFILES= al_listener.o al_source.o al_buffer.o al_play.o al_able.o al_state.o \
	al_doppler.o al_distance.o al_error.o al_ext.o al_vector.o \
	alc_context.o alc_speaker.o alc_device.o alc_state.o alc_error.o \
	alc_ext.o alut_main.o alut_wav.o
CFILES= al_listener.c al_source.c al_buffer.c al_play.c al_able.c al_state.c \
	al_doppler.c al_distance.c al_error.c al_ext.c al_vector.c \
	alc_context.c alc_speaker.c alc_device.c alc_state.c alc_error.c \
	alc_ext.c alut_main.c alut_wav.c

all: $(LIB)

$(LIB): $(OFILES)
	$(CC) $(SHARED) -o $(LIB) $(OFILES) $(LIBS)
	ar cru libopenal.a *.o

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

distclean: clean
	rm -f $(LIB) $(PROGS) .depend

clean: tidy
	rm -f *.o

install: $(LIB)
	cp $(LIB) /usr/local/lib

tidy:
	rm -f *~ *.bak *.orig

depend:
	$(CC) $(CPPFLAGS) -M $(CFILES) > .depend

ifeq (.depend, $(wildcard .depend))
include .depend
endif
