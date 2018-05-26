/* Converts the POPOI object (Heave Ho) into Cataquacks in Gelato Beach.
 */

#include "GelatoBeach.h"

short int const OBJ_251_WALL_COLLIDING_OFFSET = 0x106;

bool OBJ_251_IsCollidingWithWall = false;

// Based on parameter 1 and the current Star ID, set whether the 
// current POPOI is blue or red
// Change the palette of the materials to use the red one if current 
// Popoi is red
unsigned int OBJ_251_ObjectAddress = 0xFFFFFFFF;
unsigned int OBJ_251_BMDAddress = 0xFFFFFFFF;
void hook_02127080_ov_4D()
{
	asm
	(
		"ldr r2, =OBJ_251_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
		"ldr r2, =OBJ_251_BMDAddress		\t\n"
		"str r1, [r2]						\t\n"
	);
	
	byte param01 = *((volatile byte*)(OBJ_251_ObjectAddress + 0x08));
	
	bool isRed = false;
	if (STAR_ID >= param01)
	{
		isRed = true;
	}
	
	*((volatile byte*)(OBJ_251_ObjectAddress + 0x08)) = isRed;
	
	int textureID, paletteID;
	textureID = paletteID = (isRed) ? 1 : 0;
	
	int nMaterials = *((volatile int*)(OBJ_251_BMDAddress + 0x24));
	unsigned int offMaterials = 
		*((volatile unsigned int*)(OBJ_251_BMDAddress + 0x28));
	
	unsigned int adrMaterial = offMaterials;
	for (int i = 0; i < nMaterials; i++)
	{
		*((volatile int*)(adrMaterial + 0x04)) = textureID;
		*((volatile int*)(adrMaterial + 0x08)) = paletteID;
		adrMaterial += 48;
	}
}

// Reduce height at which Blues throw Mario into the air so that they 
// don't cause damage but leave as is for Reds
void repl_021266FC_ov_4D()
{
	asm
	(
		"ldrb r2, [r4, #0x08]	\t\n"
		"cmp r2, #0x00			\t\n"
		"moveq r2, #0x060000	\t\n"
		"movne r2, #0x070000	\t\n"
	);
}

// Disable transition from Move 1 to Move 2
void repl_02126BE8_ov_4D()
{
	asm
	(
		"cmp r14, #0x00		\t\n" // Will never be true therefore ensuring following BNE always followed
	);
}

// Set length of Move 2 state to zero just in case above doesn't work
void repl_02126A98_ov_4D()
{
	asm
	(
		"mov r3, #0x00		\t\n" // Length of Move 2 state
	);
}

// Set the initial animation speed to 1.625x
void hook_02127214_ov_4D()
{
	asm
	(
		"mov r1, #0x1A00		\t\n"
		"str r1, [r4, #0x39C]	\t\n"
	);
}

// Set the standard animation speed for Move 1 to 1.625x 1/4
void repl_02126BD8_ov_4D()
{
	asm
	(
		"mov r0, #0x1A00		\t\n" // Animation speed argument
	);
}

// Set the standard animation speed for Move 1 to 1.625x 2/4
void repl_02126AA0_ov_4D()
{
	asm
	(
		"mov r3, #0x1A00		\t\n"
	);
}

// Set the standard animation speed for Move 1 to 1.625x 3/4
void repl_02126988_ov_4D()
{
	asm
	(
		"mov r3, #0x1A00		\t\n"
	);
}

// Set the standard animation speed for Move 1 to 1.625x 4/4
void repl_02126D18_ov_4D()
{
	asm
	(
		"mov r3, #0x1A00		\t\n"
	);
}

// Set the animation speed for Move 1 when Mario close to 2.0x for Blues 
// and 3.0x for Reds
void repl_021268C4_ov_4D() 
{ 
	asm
	(
		"ldrb r0, [r5, #0x08]	\t\n"
		"cmp r0, #0x00			\t\n"
		"moveq r0, #0x2000		\t\n"
		"movne r0, #0x3000		\t\n"
	);
}

// Set the forward speed to nearly double when close to Mario for Blues 
// and even higher for Reds
void hook_021268B4_ov_4D()
{
	asm
	(
		"ldrb r0, [r5, #0x08]	\t\n"
		"cmp r0, #0x00			\t\n"
		"moveq r0, #0x00F000	\t\n"
		"movne r0, #0x016000	\t\n"
		"str r0, [r5, #0x98]	\t\n"
	);
}

// Increase the distance at which Cataquack detects Mario nearby
void repl_021268A0_ov_4D()
{
	asm
	(
		"cmp r1, #0xD0		\t\n"
	);
}

// If colliding with a wall, check whether it's a watermelon
void hook_02126F8C_ov_4D()
{
	asm
	(
		"ldr r1, =OBJ_251_ObjectAddress			\t\n"
		"str r4, [r1]							\t\n"
	);
	
	OBJ_251_IsCollidingWithWall = 
		( *((volatile byte*)(OBJ_251_ObjectAddress + OBJ_251_WALL_COLLIDING_OFFSET)) != 0x00 );
	
	if (!OBJ_251_IsCollidingWithWall) return;
	
	int currentXPos = 
		*((volatile int*)(OBJ_251_ObjectAddress + OBJ_X_LOC_OFFSET));
	//int currentYPos = 
	//	*((volatile int*)(OBJ_251_ObjectAddress + OBJ_Y_LOC_OFFSET));
	int currentZPos = 
		*((volatile int*)(OBJ_251_ObjectAddress + OBJ_Z_LOC_OFFSET));
	
	byte index = -1;
	bool hit = false;
	for (int i = 0; i < GB_NumWatermelons; i++)
	{
		unsigned int wMelonAddress = GB_Watermelons[i];
		
		if (wMelonAddress == 0xFFFFFFFF || wMelonAddress == 0) continue;
		
		int wMelonXPos = 
			*((volatile int*)(wMelonAddress + OBJ_X_LOC_OFFSET));
		//int wMelonYPos = 
		//	*((volatile int*)(wMelonAddress + OBJ_Y_LOC_OFFSET));
		int wMelonZPos = 
			*((volatile int*)(wMelonAddress + OBJ_Z_LOC_OFFSET));
		
		byte wMelonParam02 = 
			*((volatile byte*)(wMelonAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET));
		
		int wMelonRadius = 0x64000 * wMelonParam02;
		
		//if (currentYPos > (wMelonYPos + wMelonRadius)) continue;
		//if (currentYPos < (wMelonYPos - wMelonRadius)) continue;
		
		if (currentXPos > (wMelonXPos + wMelonRadius)) continue;
		if (currentXPos < (wMelonXPos - wMelonRadius)) continue;
		
		if (currentZPos > (wMelonZPos + wMelonRadius)) continue;
		if (currentZPos < (wMelonZPos - wMelonRadius)) continue;
		
		// If we reach here, it's colliding with the current watermelon
		
		// Need to check whether it's roughly facing the melon before 
		// throwing it
		
		short int rotFaceMelon = atan2Cordic(wMelonZPos - currentZPos, 
			wMelonXPos - currentXPos) * 2;
		
		if ( !(rotFaceMelon < 0 + 0x38E0) ) continue;
		
		hit = true;
		index = i;
		break;
	}
	
	if (!hit) return;
	
	// Mark the watermelon with which the current Popoi is colliding 
	// so it knows it's been got by a Cataquack
	unsigned int wMelonAddress = GB_Watermelons[index];
	
	*((volatile byte*)(wMelonAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) |= 
		0x01;
}
