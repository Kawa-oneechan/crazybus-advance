//
//  Input header
//
//! \file tonc_input.h
//! \author J Vijn
//! \date 20060508 - 20060508
//
// === NOTES ===


#ifndef TONC_INPUT
#define TONC_INPUT

#include "tonc_memmap.h"
#include "tonc_memdef.h"

#include "tonc_core.h"


/*!	\addtogroup grpInput	*/
/*! \{	*/

// === CONSTANTS ======================================================

typedef enum eKeyIndex
{
	KI_A=0, KI_B, KI_SELECT, KI_START, 
	KI_RIGHT, KI_LEFT, KI_UP, KI_DOWN,
	KI_R, KI_L, KI_MAX
} eKeyIndex;

// === MACROS =========================================================

// Check which of the specified keys are down or up right now

//! 
#define KEY_DOWN_NOW(key)	(~(REG_KEYINPUT) & key)
#define KEY_UP_NOW(key)		( (REG_KEYINPUT) & key)	

// test whether all keys are pressed, released, whatever.
// Example use:
//   KEY_EQ(key_hit, KEY_L | KEY_R)
// will be true if and only if KEY_L and KEY_R are _both_ being pressed
#define KEY_EQ(key_fun, keys)	( key_fun(keys) == (keys) )

// === CLASSES ========================================================
// === GLOBALS ========================================================

extern u16 __key_curr, __key_prev;	// in tonc_core.c

// === PROTOTYPES =====================================================

void key_wait_for_clear(u32 key);		// wait for keys to be up

// --- synchronous key states ---
INLINE void key_poll();
INLINE u32 key_curr_state();
INLINE u32 key_prev_state();

INLINE u32 key_is_down(u32 key);
INLINE u32 key_is_up(u32 key);

INLINE u32 key_was_down(u32 key);
INLINE u32 key_was_up(u32 key);

INLINE u32 key_transit(u32 key);
INLINE u32 key_held(u32 key);
INLINE u32 key_hit(u32 key);
INLINE u32 key_released(u32 key);

// --- tribools ---
INLINE int key_tri_horz();
INLINE int key_tri_vert();
INLINE int key_tri_shoulder();
INLINE int key_tri_fire();



// === INLINES=========================================================


//! Poll for keystates
INLINE void key_poll()
{	__key_prev= __key_curr;	__key_curr= ~REG_KEYINPUT & KEY_MASK;	}


//! \name Basic synchonous keystates
//\{

//! Get current keystate
INLINE u32 key_curr_state()			{	return __key_curr;			}

//! Get previous key state
INLINE u32 key_prev_state()			{	return __key_prev;			}

//! Gives the keys of \a key that are currently down
INLINE u32 key_is_down(u32 key)		{	return  __key_curr & key;	}

//! Gives the keys of \a key that are currently up
INLINE u32 key_is_up(u32 key)		{	return ~__key_curr & key;	}

//! Gives the keys of \a key that were previously down
INLINE u32 key_was_down(u32 key)	{	return  __key_prev & key;	}

//! Gives the keys of \a key that were previously down
INLINE u32 key_was_up(u32 key)		{	return ~__key_prev & key;	}

//\}


//! \name Transitional keystates
//\{

//! Gives the keys of \a key that are different from before
INLINE u32 key_transit(u32 key)
{	return (__key_curr^__key_prev) & key;	}

//! Gives the keys of \a key that are being held down
INLINE u32 key_held(u32 key)
{	return (__key_curr& __key_prev) & key;	}

//! Gives the keys of \a key that are pressed (down now but not before)
INLINE u32 key_hit(u32 key)
{	return (__key_curr&~__key_prev) & key;	}

//! Gives the keys of \a key that are being released
INLINE u32 key_released(u32 key)
{	return (~__key_curr&__key_prev) & key;	}

//\}


//! \name Tribools
//\{

//! Horizontal tribool (right,left)=(+,-)
INLINE int key_tri_horz()		
{	return bit_tribool(__key_curr, KI_RIGHT, KI_LEFT);	}

//! Vertical tribool (down,up)=(+,-)
INLINE int key_tri_vert()		
{	return bit_tribool(__key_curr, KI_DOWN, KI_UP);		}

//! Shoulder-button tribool (R,L)=(+,-)
INLINE int key_tri_shoulder()	
{	return bit_tribool(__key_curr, KI_R, KI_L);			}

//! Fire-button tribool (A,B)=(+,-)
INLINE int key_tri_fire()		
{	return bit_tribool(__key_curr, KI_A, KI_B);			}

//\}


/*	\}	*/


#endif // TONC_INPUT
