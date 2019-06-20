PNGFILES	:= $(wildcard *.png)
OFILES		:= $(PNGFILES:.png=.o)
TARGET		:= ../buses.a

include ../gfx_baserules

%.s : %.png
	$(GRIT) $< -fts -fh! -gu16 -gB4 -p
