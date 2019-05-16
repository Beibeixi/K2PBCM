# Generated automatically from Makefile.in by configure.
# $Id$
#
# Makefile for event-handling library
#
# Copyright 2002 Roaring Penguin Software Inc.
#
# This software may be distributed according to the terms of the GNU
# General Public License, version 2 or (at your option) any later version.
# LIC: GPL

OBJS=event.o event_tcp.o hash.o event_sig.o
SRCS=$(OBJS:.o=.c)
HDRS=event.h event_tcp.h eventpriv.h hash.h
CFLAGS=-g -O2 -DLINUX26 -DCONFIG_BCMWL5 -DDEBUG_NOISY -DDEBUG_RCTEST -pipe -DBCMWPA2 -DBCMARM -fno-strict-aliasing -marm -DRTCONFIG_NVRAM_64K -DLINUX_KERNEL_VERSION=132644  -idirafter /home/plp/k2p/asuswrt/release/src-rt-9.x/src/linux/linux-2.6.36/include -fno-strict-aliasing -Wall -Wstrict-prototypes -I.. $(DEFINES)
AR=ar

all: libevent.a

libevent.a: $(OBJS)
	rm -f libevent.a
	$(AR) -cq libevent.a $(OBJS)
	arm-brcm-linux-uclibcgnueabi-ranlib libevent.a

event.o: event.c $(HDRS)
	arm-brcm-linux-uclibcgnueabi-gcc $(CFLAGS) -c -o $@ $<

hash.o: hash.c $(HDRS)
	arm-brcm-linux-uclibcgnueabi-gcc $(CFLAGS) -c -o $@ $<

event_sig.o: event_sig.c $(HDRS)
	arm-brcm-linux-uclibcgnueabi-gcc $(CFLAGS) -c -o $@ $<

event_tcp.o: event_tcp.c $(HDRS)
	arm-brcm-linux-uclibcgnueabi-gcc $(CFLAGS) -c -o $@ $<

clean: FORCE
	rm -f *.a *.o *~

FORCE:

.phony: FORCE
