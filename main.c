#include "crazybus.h"

//Okay, first of all let's do a magic trick. Open buses.txt and look at it.
//  BUS(century390, "Century 390\n  Mercedes or Volvo chassis\n  400 HP\n  Capacity: 40 passengers\n  Extra luxurious.")
//Seems clear, right? Name of the picture in /buses, and the description. No fuss no muss.
//How do we turn that file into C data? With a preprocessor magic trick.

//Step one: creating externs. We define a macro BUS(x, y) that turns the above into
//  extern const unsigned short century390Tiles[], century390Pal[], century390_picTiles[], century390_picPal[];
//then include buses.txt and finally undefine the macro again for the next step.
#define BUS(x, y) extern const unsigned short x ## Tiles[], x ## Pal[], x ## _picTiles[], x ## _picPal[];
#include "buses.txt"
#undef BUS
//Now we use the same trick to build a bunch of arrays.
#define BUS(x, y) x ## Tiles,
const unsigned short* busTiles[] = {
	#include "buses.txt"
};
#undef BUS
#define BUS(x, y) x ## Pal,
const unsigned short* busPal[] = {
	#include "buses.txt"
};
#undef BUS
#define BUS(x, y) x ## _picTiles,
const unsigned short* busPicTiles[] = {
	#include "buses.txt"
};
#undef BUS
#define BUS(x, y) x ## _picPal,
const unsigned short* busPicPal[] = {
	#include "buses.txt"
};
#undef BUS
//This time, we'll finally use the Y part, the description. This is done exactly as you'd think.
#define BUS(x, y) y,
const char* busTexts[] = {
	#include "buses.txt"
};
#undef BUS
//Now, to figure out how -many- buses there are, we make an enum that defines a terminal value.
//The meaning of bus_lastBus will shift accordingly. Genius, innit?
#define BUS(x, y) bus_ ## x,
typedef enum eBuses {
	#include "buses.txt"
	bus_lastBus
} eBuses;
#undef BUS
//End of the magic trick, go home.

//OtherGfx data doesn't change much so we don't need any magic bullshit here.
extern const unsigned short fontTiles[4096], fontPal[16];
extern const unsigned short legalTiles[7760], legalPal[16], legalMap[640];
extern const unsigned short selectTiles[660], selectPal[16], selectMap[640];
extern const unsigned short driveTiles[7760], drivePal[16], driveMap[640];

//Buncha boilerplate that dates all the way back to OpenPoke. Hardly interesting.
extern void intr_main(void);
typedef void (*IntrFuncp)(void);
void DoVBlank();
void KeyRead(void);
void IntrDummy(void);
void VBlankIntr(void);
void IntroVBlank(void);
void HBlankIntr(void);
void HBlankTest(void);
IntrFuncp IntrTable[13] =
{
	VBlankIntr,	// V Blank interrupt
	IntrDummy,	// H Blank interrupt
	IntrDummy,	// V Counter interrupt
	IntrDummy,	// Timer 0 interrupt
	IntrDummy,	// Timer 1 interrupt
	IntrDummy,	// Timer 2 interrupt
	IntrDummy,	// Timer 3 interrupt
	IntrDummy,	// Serial communication interrupt
	IntrDummy,	// DMA 0 interrupt
	IntrDummy,	// DMA 1 interrupt
	IntrDummy,	// DMA 2 interrupt
	IntrDummy,	// DMA 3 interrupt
	IntrDummy,	// Key interrupt
};
u16 Cont, Trg;
OamData OamBak[128];

//For ease of access to the layer we put our text on, we define this.
unsigned short* BG1 = (unsigned short*)(MEM_VRAM + 0xE800);
//Game variables, no real biggie here.
int chosenBus, distance;
//How long to wait for the next note of the musical masterpiece that is CrazyBus, and a randomizer seed to drive it.
int musicDelay, rndseed;

//Support routine: a simple text writer that outputs in the 16th palette and supports newlines.
void Write(const char* str, int left, int top)
{
	unsigned int p = (top * 32) + left;
	unsigned int sp = p;
	unsigned int c;
	while ((c = *str))
	{
		if (c == '\n')
		{
			sp += 0x20;
			p = sp;
		}
		else
		{
			BG1[p] = 0xF000 | c;
			p++;
		}
		str++;
	}
}

//Simple fade routines from OpenPoke.
void FadeIn(void)
{
	int i;
	//We want to fade everything, so set up the BLenD CoNTrol as such.
	REG_BLDCNT = BLD_BLACK | BLD_BD | BLD_BG0 | BLD_BG1 | BLD_BG2 | BLD_BG3 | BLD_OBJ;
	for(i = 16; i >= 0; i--)
	{
		DoVBlank();
		REG_BLDY = i;
	}
}
void FadeOut(void)
{
	int i;
	REG_BLDCNT = BLD_BLACK | BLD_BD | BLD_BG0 | BLD_BG1 | BLD_BG2 | BLD_BG3 | BLD_OBJ;
	for(i = 0; i <= 16; i++)
	{
		DoVBlank();
		REG_BLDY = i;
	}
}

//Does exactly what it implies. Recall that BG1 is our text layer.
inline void ClearBG1()
{
	CpuFastClear(0, BG1, 2048);
}

//Simple randomizer copied from OpenPoke, which is in turn copied straight from Pokemon Fire Red.
unsigned int rand()
{
	rndseed = (rndseed * 0x41C64E6D) + 0x6073;
	return rndseed;
}
unsigned int rrand(unsigned int range)
{
	return (rand() >> 8) % range;
}

//Set different rndseed values to change the tune.
void MusicalRape()
{
	if (musicDelay > 0)
	{
		musicDelay--;
		return;
	}
	musicDelay = rrand(10) + 4;
	REG_SND1CNT = SSQR_ENV_BUILD(10 + rrand(5), 0, 1 + rrand(4));
	REG_SND1FREQ = SFREQ_BUILD(rand(), 1, 1);
	REG_SND2CNT = SSQR_ENV_BUILD(10 + rrand(5), 0, 1 + rrand(4));
	REG_SND2FREQ = SFREQ_BUILD(rand(), 1, 1);
}

//Draws the currently selected bus's description and photo.
void DrawSelection()
{
	//We use the text layer to show the photo, so only clear the top part!
	CpuFastClear(0, BG1, 420);
	Write(busTexts[chosenBus], 1, 1);
	//Copy the bus's picture.
	CpuFastCopy(busPicTiles[chosenBus], MEM_VRAM + 0x2000, 5120);
	CpuFastCopy(busPicPal[chosenBus], MEM_PAL + 0x20, 16);
	//Fix the palette.
	((u16*)MEM_PAL)[0] = busPicPal[chosenBus][0];
}

//===***CHOOSE YOUR CRAZYBUS***===
void SelectLoop()
{
	int i, j = 0, k = 0x1000 | 256; //for setting up the bus photo area.
	ClearBG1();
	//Copy the interface-y bits.
	CpuFastArrayCopy(selectTiles, MEM_VRAM + 0x8000);
	CpuFastArrayCopy(selectPal, MEM_PAL);
	CpuFastArrayCopy(selectMap, MEM_VRAM + 0xF800);
	CpuFastArrayCopy(fontTiles, MEM_VRAM + 0x0000);
	CpuFastArrayCopy(fontPal, MEM_PAL + 0x1E0);
	//Set up the bus photo area.
	for (i = 0xE1; i < 0x212; i++)
	{
		BG1[i] = k++;
		j++;
		//Skip to the next line once this one is finished.
		if (j == 16)
		{
			j = 0;
			i += 16;
		}
	}
	//Make sure the selected bus is already there before we fade in.
	DrawSelection();
	FadeIn();
	rndseed = 9001; //set up the procedural music generator
	while(1)
	{
		DoVBlank();
		KeyRead();
		MusicalRape();
		if (Trg & R_KEY) {
			chosenBus++;
			if (chosenBus >= bus_lastBus) //remember the magic trick?
				chosenBus = 0;
			DrawSelection();
		}
		if (Trg & L_KEY) {
			chosenBus--;
			if (chosenBus < 0)
				chosenBus = bus_lastBus - 1;
			DrawSelection();
		}
		if (Trg & A_BUTTON) {
			FadeOut();
			return; //continues on to the main game.
		}
	}
}

inline void DrivingNoise()
{
	REG_SND1CNT = SSQR_ENV_BUILD(15, 0, 7);
	REG_SND1FREQ = SFREQ_BUILD(124, 1, 1);
}

inline void ClaxonNoise()
{
	REG_SND2CNT = SSQR_ENV_BUILD(15, 1, 8);
	REG_SND2FREQ = SFREQ_BUILD(320, 1, 1);
}

//This is it. The important part. Woot. So crazy. Much bus.
void GameLoop()
{
	int i;
	char b[256]; //a buffer for the distance counter to be written in.
	ClearBG1();
	distance = 0;
	//Copy the interface and our chosen bus.
	CpuFastArrayCopy(driveTiles, MEM_VRAM + 0x8000);
	CpuFastArrayCopy(drivePal, MEM_PAL);
	CpuFastArrayCopy(driveMap, MEM_VRAM + 0xF800);
	CpuFastCopy(busTiles[chosenBus], MEM_VRAM_OBJ, 6400);
	CpuFastCopy(busPal[chosenBus], MEM_PAL_OBJ, 16);
	//Dirty trick to draw the road.
	CpuFastClear(0xF080F080, BG1 + 0x180, 64);
	CpuFastClear(0xF081F081, BG1 + 0x1A0, 64);
	//Set up the bus sprites. It's two big ones side by side, and two wide ones below that.
	for (i = 0; i < 4; i++)
	{
		OamBak[i].CharNo = i * 4;
		OamBak[i].VPos = 48;
		OamBak[i].Size = 2;
		OamBak[i + 4].CharNo = 128 + (i * 4);
		OamBak[i + 4].VPos = 48 + 32;
		OamBak[i + 4].Size = 2;
		OamBak[i + 4].Shape = 1;
		OamBak[i].HPos = ((distance - 256) / 2) + (i * 32);
		OamBak[i + 4].HPos = ((distance - 256) / 2) + (i * 32);
	}
	Write("==**CrazyBus Advance 1.1**==", 1, 16);
	//Gotta write the distance before we fade in. We could just Write("Distance: 0", 1, 1) directly...
	//sprintf(b, "Distance: %d  ", distance);
	strcpy(b, "Distance: ");
	itoa(distance, b + 10, 10);
	Write(b, 1, 1);
	FadeIn();
	while(1)
	{
		//Update the sprites, no matter if we moved or not. I DON'T GIVE A FUCK.
		for (i = 0; i < 4; i++)
		{
			OamBak[i].HPos = ((distance - 256) / 2) + (i * 32);
			OamBak[i + 4].HPos = ((distance - 256) / 2) + (i * 32);
		}
		//Update our distance counter no matter if we moved or not too.
		//sprintf(b, "Distance: %d  ", distance);
		itoa(distance, b + 10, 10);
		Write(b, 1, 1);
		DoVBlank();
		KeyRead();
		if (Cont & R_KEY)
		{
			distance++;
			DrivingNoise();
		}
		else if (Cont & L_KEY)
		{
			distance--;
			DrivingNoise();
		}
		else if (Trg & START_BUTTON)
		{
			FadeOut();
			DmaArrayClear(160, OamBak); //hide the sprites so we don't get them in the select screen.
			return; //loops back to the select screen.
		}
		if (Trg & A_BUTTON)
		{
			ClaxonNoise();
 		}
	}
}

void Legal()
{
	int i = 4 * 60; //Four times 60 ticks, thus four seconds.
	CpuFastArrayCopy(legalTiles, MEM_VRAM + 0x8000);
	CpuFastArrayCopy(legalPal, MEM_PAL);
	CpuFastArrayCopy(legalMap, MEM_VRAM + 0xF800);
	FadeIn();
	while(i--)
		DoVBlank();
	FadeOut();
}

//Our entry point. Mostly boilerplate.
int main(void)
{
	//Set up interrupts, screen modes and such...
	*(vu32 *)INTR_VECTOR_BUF = (vu32 )intr_main;
	REG_IE = V_BLANK_INTR_FLAG | H_BLANK_INTR_FLAG | CASSETTE_INTR_FLAG;
	REG_DISPSTAT = STAT_V_BLANK_IF_ENABLE | STAT_H_BLANK_IF_ENABLE;
	REG_DISPCNT =  DCNT_MODE0;
	REG_BLDY = 15;
	REG_IME = 1;
	//Clear out the sprites (technically put them offscreen)...
	DmaArrayClear(160, OamBak);
	//Set up the background layers -- we use two.
	REG_BG0CNT = BG_4BPP | BG_SIZE0 | BG_PRIO(3) | 31 << BG_SBB_SHIFT | 2 << BG_CBB_SHIFT;
	REG_BG1CNT = BG_4BPP | BG_SIZE0 | BG_PRIO(2) | 29 << BG_SBB_SHIFT | 0 << BG_CBB_SHIFT;
	REG_BG2CNT = BG_4BPP | BG_SIZE0 | BG_PRIO(1) | 28 << BG_SBB_SHIFT | 0 << BG_CBB_SHIFT;
	REG_BG3CNT = BG_4BPP | BG_SIZE0 | BG_PRIO(0) | 30 << BG_SBB_SHIFT | 0 << BG_CBB_SHIFT;
	//Turn on all background layers, and the sprites.
	REG_DISPCNT =  DCNT_MODE0 | DCNT_OBJ | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3 | DCNT_OBJ_2D;

	REG_SNDDMGCNT = SDMG_BUILD_LR(SDMG_SQR1 | SDMG_SQR2, 6);
	REG_SNDSTAT = SSTAT_ON;
	//full volume, enable sound 1 to left and right
	//REG_SOUNDCNT_L=0x1177;
	// Overall output ratio - Full
	//REG_SOUNDCNT_H = 2;

	//Reset the bus choice just to be sure.
	chosenBus = 0;
	Legal();
	//We never ever break out of this loop. Repeatedly ask for a bus and drive it.
	while(1)
	{
		SelectLoop();
		GameLoop();
	}
	return 0;
}

void DoVBlank()
{
	VBlankIntrWait();
	Halt();
}

//This is ran every time we press a key.
void KeyRead(void)
{
	//Imagine if the key input register changed value in mid-function, or contained bullshit in the upper bits.
	u16 ReadData = (REG_KEYINPUT ^ 0x03ff);
	//Some minor-league bit tricks with the previous Cont lets us detect newly-pressed buttons in Trg.
	Trg  = ReadData & (ReadData ^ Cont);
	Cont = ReadData;
}

//This is ran every 60th of a second.
void VBlankIntr(void)
{
	//Update sprites and acknowledge we've handled the VBlank.
	DmaArrayCopy(OamBak,MEM_OAM);
	*(u16 *)INTR_CHECK_BUF = V_BLANK_INTR_FLAG;
}

void IntrDummy(void) { ; }

