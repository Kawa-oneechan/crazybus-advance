FOURWPAL	:=	font.png
FOURWPALMAP	:=	legal.png select.png drive.png

PNGFILES	:=	$(FOURWPAL) $(FOURWPALMAP)
OFILES		:=	$(PNGFILES:.png=.o)
TARGET		:=	../othergfx.a

GRITBASE	:=	-fts -fh! -gu16

include ../gfx_baserules

# ---------------------------------------------------------------------
# Rules
# ---------------------------------------------------------------------

$(FOURBPP:.png=.s) : %.s : %.png
	$(GRIT) $< $(GRITBASE) -gB4 -p!

$(FOURWPAL:.png=.s) : %.s : %.png
	$(GRIT) $< $(GRITBASE) -gB4 -p

$(FOURWMAP:.png=.s) : %.s : %.png
	$(GRIT) $< $(GRITBASE) -gB4 -p! -mRtf

$(FOURWPALMAP:.png=.s) : %.s : %.png
	$(GRIT) $< $(GRITBASE) -gB4 -p -mRtf

$(FOURWMAPONLY:.png=.s) : %.s : %.png
	$(GRIT) $< $(GRITBASE) -gB4 -p! -mRtf

$(FOURWPAL_C:.png=.s) : %.s : %.png
	$(GRIT) $< $(GRITBASE) -gB4 -gzl -p

# No 'p' here - no per-tile palette, causes glitch tiles.
$(EIGHTWPALMAP_C:.png=.s) : %.s : %.png
	$(GRIT) $< $(GRITBASE) -gB8 -gzl -p -mRtf -mzl
