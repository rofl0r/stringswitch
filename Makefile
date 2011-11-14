LINKLIBS=

FNAME=stringswitch_gen

MAINFILE=$(FNAME).c
OUTFILE=$(FNAME).out

CFLAGS_OWN=-Wall -Wextra -static -std=c99
CFLAGS_DBG=-g -O0
CFLAGS_OPT=-s -Os

-include config.mak

CFLAGS_RCB_OPT=${CFLAGS_OWN} ${CFLAGS_OPT} ${CFLAGS}
CFLAGS_RCB_DBG=${CFLAGS_OWN} ${CFLAGS_DBG} ${CFLAGS}

all: debug

optimized:
	CFLAGS="${CFLAGS_RCB_OPT}" rcb --force $(RCBFLAGS) ${MAINFILE} $(LINKLIBS)
	strip --remove-section .comment ${OUTFILE}

debug:
	CFLAGS="${CFLAGS_RCB_DBG}" rcb --force $(RCBFLAGS) ${MAINFILE} $(LINKLIBS)


.PHONY: all obfuscated debug
