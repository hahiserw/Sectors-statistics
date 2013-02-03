# PROGRAM=sectors-visualizer
PROGRAM=sectors-statistics
C=gcc
CFLAGS=-g
#CFLAGS=`pkg-config --libs --cflags gtk+-3.0`

all: $(PROGRAM)

$(PROGRAM): main.c
	$(C) $< $(CFLAGS) -o $@