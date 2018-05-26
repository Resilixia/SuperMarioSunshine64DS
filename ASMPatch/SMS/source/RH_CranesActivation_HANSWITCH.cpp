#include "RiccoHarbour.h"

/* 
 * Object 043 - HANSWITCH "Red ! Switch"
 */

const short int PARAMETER_01_OFFSET = 0x08;

unsigned int OBJ_043_ObjectAddress = 0xFFFFFFFF;

const short int OBJ_043_RH_CRANES_ACTIVATION_PARAM01_VALUE = 0xFF04;

const short int OBJ_043_PRESSED_BUTTON_DRAW_HEIGHT_OFFSET = 0x324;
const short int OBJ_043_PRESSED_COLLISION_VALUE_OFFSET = 0x32C;
const short int OBJ_043_PRESSED_INDICATOR_BYTE_01_OFFSET = 0x340;
const short int OBJ_043_PRESSED_INDICATOR_BYTE_02_OFFSET = 0x34D;

const short int OBJ_043_PRESSED_BUTTON_DRAW_HEIGHT = 0x0333;
const int OBJ_043_PRESSED_COLLISION_VALUE = 0x0003000C;
const byte OBJ_043_PRESSED_INDICATOR_BYTE_01 = 0x02;
const byte OBJ_043_PRESSED_INDICATOR_BYTE_02 = 0x01;

short int const DRAW_DISTANCE_OFFSET = 0xBC;

// If parameter 01 set to 0xFF04, check whether the cranes have already 
// been activated in R.H. and if so, set switch permanently into its 
// activated (pressed) state
void hook_020BAA00_ov_02()
{
	asm
	(
		"ldr r2, =OBJ_043_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	);
	
	short int param01 = *((volatile short int*)(OBJ_043_ObjectAddress + PARAMETER_01_OFFSET));
	
	if (param01 == OBJ_043_RH_CRANES_ACTIVATION_PARAM01_VALUE)
	{
		if (RH_AreCranesActivated)
		{
			*((volatile short int*)(OBJ_043_ObjectAddress + OBJ_043_PRESSED_BUTTON_DRAW_HEIGHT_OFFSET)) = 
				OBJ_043_PRESSED_BUTTON_DRAW_HEIGHT;
			*((volatile int*)(OBJ_043_ObjectAddress + OBJ_043_PRESSED_COLLISION_VALUE_OFFSET)) = 
				OBJ_043_PRESSED_COLLISION_VALUE;
			*((volatile byte*)(OBJ_043_ObjectAddress + OBJ_043_PRESSED_INDICATOR_BYTE_01_OFFSET)) = 
				OBJ_043_PRESSED_INDICATOR_BYTE_01;
			*((volatile byte*)(OBJ_043_ObjectAddress + OBJ_043_PRESSED_INDICATOR_BYTE_02_OFFSET)) = 
				OBJ_043_PRESSED_INDICATOR_BYTE_02;
		}
	}
	
	*((volatile unsigned int*)(OBJ_043_ObjectAddress + DRAW_DISTANCE_OFFSET)) *= 2;
}

// If parameter 01 set to 0xFF04, activate the cranes in Ricco Harbour 
// when stepped on
void hook_020BA38C_ov_02()
{
	asm
	(
		"ldr r2, =OBJ_043_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	);
	
	short int param01 = *((volatile short int*)(OBJ_043_ObjectAddress + PARAMETER_01_OFFSET));
	
	if (param01 == OBJ_043_RH_CRANES_ACTIVATION_PARAM01_VALUE)
	{
		SetCannonsToActivatedForCurrentLevel();
		RH_AreCranesActivated = true;
	}
}