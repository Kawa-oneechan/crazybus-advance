PATH	:= $(DEVKITARM)/bin:$(PATH)

CROSS	:= arm-none-eabi-

# To make things go
CC		:= $(CROSS)gcc		#agbcc
AS		:= $(CROSS)gcc	#agbas
LD		:= $(CROSS)gcc	#agbld
OBJCOPY	:= $(CROSS)objcopy

# --------------------------------------------------------------------
# FILES
# --------------------------------------------------------------------

TARGET	:= crazybus
TITLE	:= $(TARGET)

# Regular makefile continues here
SFILES	:=	tonc_bios.s		\
		
CFILES	:=	main.c printf.c

export AFILES	:= othergfx.a buses.a
export ROFILES	:= $(SFILES:.s=.o) $(CFILES:.c=.o)
export IOFILES	:= $(ICFILES:.c=.o) $(MAPPACK)
export OFILES	:= $(ROFILES) $(IOFILES) toolinclude/irq_single.o

# --------------------------------------------------------------------
# OPTIONS
# --------------------------------------------------------------------

MAPFILE		:= $(TARGET).map

AGBINC	   =	$(AGBDIR)/include
AGBLIB	   =	$(AGBDIR)/lib

ARCH    := -mthumb-interwork -mthumb
RARCH   := -mthumb-interwork -mthumb
IARCH   := -mthumb-interwork -marm
SPECS   := -specs=gba.specs

INCLUDE		:= -I toolinclude
LIBPATHS	:=

CBASE   := $(INCLUDE) -O2 -Wall -fno-strict-aliasing
RCFLAGS := $(CBASE) $(RARCH)
ICFLAGS := $(CBASE) $(IARCH) -mlong-calls
CFLAGS  := $(RCFLAGS)

ASFLAGS := -x assembler-with-cpp -c -mthumb-interwork -Wall
LDFLAGS := $(ARCH) $(SPECS) $(LIBPATHS) $(LIBS)

LDFLAGS += -Wl,-Map $(MAPFILE) #-nostartfiles

# ---------------------------------------------------------------------
# RULES
# ---------------------------------------------------------------------

$(TARGET).gba : $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	@gbafix $@ -tKAWA_CRAZYBUS -cKWAD -mXX

$(TARGET).elf : $(OFILES) $(AFILES) Makefile $(DEPENDFILE)
	@echo > $(MAPFILE)
	$(LD) -g -o $@ $(OFILES) $(AFILES) $(LDFLAGS)

buses.a:
	$(MAKE) -C buses
othergfx.a:
	$(MAKE) -C othergfx

.PHONY: all clean depend

all:    clean $(TARGET).elf

# clean:
#	rm $(AFILES) $(OFILES) $(DEPENDFILE) $(MAPFILE) $(TARGET).elf

depend:
	$(CC) $(RCFLAGS) -M $(RCFILES) > $(DEPENDFILE)
	$(CC) $(ICFLAGS) -M $(ICFILES) > $(DEPENDFILE)

$(DEPENDFILE): 
	$(CC) $(RCFLAGS) -M $(CFILES) $(ICFLAGS) $(ICFILES) > $(DEPENDFILE)


# --- IWRAM compilation ----
%.iwram.o : %.iwram.c
	$(CC) $(ICFLAGS) -c $< -o $@

# --- ROM compilation ---
%.o : %.c
	$(CC) $(RCFLAGS) -c $< -o $@

# --- Assembling ---
%.o : %.s
	$(AS) $(ASFLAGS) -c $< -o $@


#$(.IOFILES) : %.o : %.c
#	$(CC) $(ICFLAGS) -c $< -o $@









