#ifndef __CRAZYBUS
#define __CRAZYBUS
#define INEWRAM	__attribute__((section(".ewram")))
#include <stdarg.h>
#include "toolinclude/tonc.h"
#include "toolinclude/noaglue.h"
// --------------------------------------------------------------------
#define CSTRING const char * const
#define ASM_BREAK()	asm volatile("\tmov\t\tr11, r11")
u32 strnlen(const char *str, u32 count);
int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
#define GETBIT(f, b) ((f[(b)/8]>>((b)&7))&1)
#define SETBIT(f, b)  f[(b)/8]|=1<<((b)&7)
#define CLRBIT(f, b)  f[(b)/8]&=~(1<<((b)&7))
// --------------------------------------------------------------------
extern u16 Cont, Trg;
extern OamData OamBak[128];
extern char* strcat(char *,const char *);
extern int strcmp(char *,const char *);
extern int strcpy(char *,const char *);
extern void* memset(char*, int, int);
#endif
