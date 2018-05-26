#include "GelatoBeach.h"

unsigned int OBJ_280_ObjectAddress = 0xFFFFFFFF;
bool OBJ_280_TriggerBattle = false;

const int OBJ_280_BATTLE_TRIGGER_RANGE_X_Z = 0x12C000; // 0.3
const int OBJ_280_BATTLE_TRIGGER_RANGE_Y_BELOW = 0x64000; // 0.1

// Constructor
void hook_02113248_ov_22()
{
	OBJ_280_TriggerBattle = false;
}

// Change battle trigger from being based on Mario's Y position being 
// 0.3 above Wiggler's to Mario's location being within a particular 
// range 1/2
void hook_02112384_ov_22()
{
	asm
	(
		"ldr r0, =OBJ_280_ObjectAddress		\t\n"
		"str r5, [r0]						\t\n"
	);
	
	if (OBJ_280_TriggerBattle) return;
	
	int xPos = *((volatile int*)(OBJ_280_ObjectAddress + OBJ_X_LOC_OFFSET));
	int yPos = *((volatile int*)(OBJ_280_ObjectAddress + OBJ_Y_LOC_OFFSET));
	int zPos = *((volatile int*)(OBJ_280_ObjectAddress + OBJ_Z_LOC_OFFSET));
	
	int xDiff = PLAYER->xPos - xPos;
	int yDiff = PLAYER->yPos - yPos;
	int zDiff = PLAYER->zPos - zPos;
	
	xDiff = (xDiff < 0) ? (0 - xDiff) : xDiff;
	zDiff = (zDiff < 0) ? (0 - zDiff) : zDiff;
	
	if (xDiff > OBJ_280_BATTLE_TRIGGER_RANGE_X_Z) return;
	if (zDiff > OBJ_280_BATTLE_TRIGGER_RANGE_X_Z) return;
	
	if (yDiff > 0)
	{
		if (yDiff > OBJ_280_BATTLE_TRIGGER_RANGE_X_Z) return;
	}
	else if (yDiff < 0)
	{
		if (yDiff < OBJ_280_BATTLE_TRIGGER_RANGE_Y_BELOW) return;
	}
	
	// Else, it's within range
	OBJ_280_TriggerBattle = true;
}

// If Mario found to be within range in above method, ensure battle 
// triggered
void repl_02112388_ov_22()
{
	asm
	(
		"ldr r0, =OBJ_280_TriggerBattle		\t\n"
		"ldrb r0, [r0]						\t\n"
		"cmp r0, #0x00						\t\n"
	);
}

