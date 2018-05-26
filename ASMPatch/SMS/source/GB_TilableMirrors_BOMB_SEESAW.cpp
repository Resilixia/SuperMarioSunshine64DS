/* Modify the BOMB_SEESAW and SECRET_COIN (Invisible Secret) objects as follows to allow the 
 * Player to collect the first Star in Gelato Beach by correctly aligning the three mirrors 
 * that heat the Sand Egg:
 * 		- Disable original BOMB_SEESAW behaviour
 * 		- Update the BOMB_SEESAW behaviour so that it tilts about its X and Z axes depending 
 * 		  on the player's location. 
 * 		- The correct alignment of each BOMB_SEESAW may be specified through settings parameters 
 * 		  2 and 3 to the correct X and Z rotation values respectively. Values correspond to the 
 * 		  values used in this code and aren't consistent with those used in the rest of the game.
 * 		- Once the BOMB_SEESAW has been aligned, spawns an OBJ_NUMBER and once all aligned, spawns 
 * 		  a Star from inside the Sand Egg to the corresponding Star Marker.
 */

#include "GelatoBeach.h"

short int const MARIO_COLLIDING_OFFSET = 0x326;

byte const OBJ_173_NUM_PIVOT_MIRRORS = 3;
byte OBJ_173_NumMirrorsAligned = 0;

const short int OBJ_061_ACTOR_ID = 178;
const short int OBJ_063_ACTOR_ID = 180;
const short int OBJ_306_ACTOR_ID = 330;

/////////////////////////////  173 - BOMB_SEESAW   /////////////////////////////

// Unique address: 021374FC

// OBJ_173_FREE_EIGHT_BYTES_OBJECT_OFFSET:
// Offset		Size 		Desc.
// 0x00			1			Bit 1: bool starting_rotation_set
// 							Bit 2: bool rotation_aligned
// 0x04			2			Correctly aligned X rotation 
// 0x06			2			Correctly aligned Z rotation 

short int const OBJ_173_OBJ_PARAMETER_01_OFFSET = 0x08;
short int const OBJ_173_OBJ_PARAMETER_02_OFFSET = 0x92;
short int const OBJ_173_OBJ_PARAMETER_03_OFFSET = 0x96;
short int const OBJ_173_FREE_EIGHT_BYTES_OBJECT_OFFSET = 0xA0;

// OBJ_173_LOC_DIFF_STEP calculated by (approx.): ((OBJ_173_RADIUS / (OBJ_173_ROTATION_MAX / OBJ_173_ROTATION_INCREMENT)) / 2)
int const OBJ_173_RADIUS = 5873664; // ~1.434
int const OBJ_173_ROTATION_INCREMENT = 64;
int const OBJ_173_ROTATION_MAX = 5120;
int const OBJ_173_LOC_DIFF_STEP = 36710;
short int const OBJ_173_CORRECT_ALIGNMENT_TOLERANCE = 512;

short int const OBJ_173_COLLISION_BOUNDS_OFFSET = 0xB8;
int const OBJ_173_COLLISION_BOUNDS_VALUE_RSHIFT3 = 0x0C0000;

unsigned int OBJ_173_ObjectAddress = 0xFFFFFFFF;

byte OBJ_173_StarSpawnDelayCounter = 0;
const byte OBJ_173_STAR_SPAWN_DELAY = 5;

// Initialisation
void hook_02135B80_ov_5F()
{
	asm
	(
		"ldr r1, =OBJ_173_ObjectAddress		\t\n"
		"str r4, [r1]						\t\n"
	);
	
	*((volatile unsigned int*)(OBJ_173_ObjectAddress + OBJ_173_FREE_EIGHT_BYTES_OBJECT_OFFSET)) = 0x00000000;
	
	byte startingXRotNumSteps = (byte)(OBJ_173_ObjectAddress) >> 2; // 4th (LSB) of object address / 2
	byte startingZRotNumSteps = (byte)((OBJ_173_ObjectAddress << 8) >> 24) >> 2; // 3rd (2nd LSB) of object address / 2
	
	*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) = (short int)(startingXRotNumSteps * OBJ_173_ROTATION_INCREMENT);
	*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) = (short int)(startingZRotNumSteps * OBJ_173_ROTATION_INCREMENT);
	
	*((volatile byte*)(OBJ_173_ObjectAddress + OBJ_173_FREE_EIGHT_BYTES_OBJECT_OFFSET)) |= 0x01;
	
	short int param02 = *((volatile short int*)(OBJ_173_ObjectAddress + OBJ_173_OBJ_PARAMETER_02_OFFSET));
	short int param03 = *((volatile short int*)(OBJ_173_ObjectAddress + OBJ_173_OBJ_PARAMETER_03_OFFSET));
	
	*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_173_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x04)) = param02;
	*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_173_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x06)) = param03;
}

// Increase the collision detection bounds
void hook_02135FB0_ov_5F()
{
	asm
	(
		"ldr r0, =OBJ_173_ObjectAddress		\t\n"
		"str r4, [r0]						\t\n"
	);
	
	*((volatile int*)(OBJ_173_ObjectAddress + OBJ_173_COLLISION_BOUNDS_OFFSET)) = OBJ_173_COLLISION_BOUNDS_VALUE_RSHIFT3;
}

int OBJ_173_TmpPosArray[3] = { 0, 0, 0 };
void hook_02135980_ov_5F()
{
	asm
	(
		"ldr r0, =OBJ_173_ObjectAddress		\t\n"
		"str r5, [r0]						\t\n"
	);
	
	if (STAR_ID != GB_AlignMirrorsMissionStarID)
	{
		short int alignedRotX = *((volatile short int*)(OBJ_173_ObjectAddress + OBJ_173_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x04));
		short int alignedRotZ = *((volatile short int*)(OBJ_173_ObjectAddress + OBJ_173_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x06));
		
		*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) = alignedRotX;
		*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) = alignedRotZ;
		
		return;
	}
	
	// The use if OBJ_173_StarSpawnDelayCounter is required because spawning the Star 
	// immediately after the OBJ_NUMBER seems to cause a crash on read NDS
	if (OBJ_173_NumMirrorsAligned == OBJ_173_NUM_PIVOT_MIRRORS)
	{
		OBJ_173_StarSpawnDelayCounter++;
	}
	if (OBJ_173_StarSpawnDelayCounter == OBJ_173_STAR_SPAWN_DELAY)
	{
		short int param01 = (short int)(0x0040 | GB_AlignMirrorsMissionStarID);
		
		SpawnActor(OBJ_061_ACTOR_ID, param01, GB_AlignedMirrorsStarSpawnStartPos, 0);
		
		OBJ_173_NumMirrorsAligned = 0;
		OBJ_173_StarSpawnDelayCounter = 0;
	}
	
	bool collidingWithMario = *((volatile bool*)(OBJ_173_ObjectAddress + MARIO_COLLIDING_OFFSET));
	
	if (!collidingWithMario) return;
	
	bool aligned = *((volatile byte*)(OBJ_173_ObjectAddress + OBJ_173_FREE_EIGHT_BYTES_OBJECT_OFFSET)) & 0x02;
	if (aligned) return;
	
	int playerXPos = PLAYER->xPos;
	int playerYPos = PLAYER->yPos;
	int playerZPos = PLAYER->zPos;
	
	int xLocationDiff = (int)playerXPos - *((volatile int*)(OBJ_173_ObjectAddress + OBJ_X_LOC_OFFSET));
	bool xNegDiff = (xLocationDiff < 0);
	if (xNegDiff)
	{
		xLocationDiff = 0 - xLocationDiff;
	}
	
	if (xLocationDiff > OBJ_173_RADIUS) return;
	
	int zLocationDiff = (int)playerZPos - *((volatile int*)(OBJ_173_ObjectAddress + OBJ_Z_LOC_OFFSET));
	bool zNegDiff = (zLocationDiff < 0);
	if (zNegDiff)
	{
		zLocationDiff = 0 - zLocationDiff;
	}
	
	int xNumSteps = 0;
	int tmp = xLocationDiff;
	while (tmp > 0)
	{
		xNumSteps++;
		tmp -= OBJ_173_LOC_DIFF_STEP;
	}
	
	int zNumSteps = 0;
	tmp = zLocationDiff;
	while (tmp > 0)
	{
		zNumSteps++;
		tmp -= OBJ_173_LOC_DIFF_STEP;
	}
	
	short int maxZRotation = (short int)(xNumSteps * OBJ_173_ROTATION_INCREMENT);
	short int maxXRotation = (short int)(zNumSteps * OBJ_173_ROTATION_INCREMENT);
	
	// Update object's Z Rotation
	// If Player's X location < object's, turn clockwise
	if (xNegDiff)
	{
		if (*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) + OBJ_173_ROTATION_INCREMENT < maxZRotation)
		{
			*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) += OBJ_173_ROTATION_INCREMENT;
		}
		else if (*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) + OBJ_173_ROTATION_INCREMENT > maxZRotation)
		{
			short int zRotDiff = *((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) - maxZRotation;
			short int decrementValue = (zRotDiff >= OBJ_173_ROTATION_INCREMENT) ? OBJ_173_ROTATION_INCREMENT : zRotDiff;
			*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) -= decrementValue;
		}
		else
		{
			*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) = maxZRotation;
		}
	}
	// Else if it's > object's, turn counter-clockwise
	else
	{
		maxZRotation = 0 - maxZRotation;
		if (*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) - OBJ_173_ROTATION_INCREMENT > maxZRotation)
		{
			*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) -= OBJ_173_ROTATION_INCREMENT;
		}
		else if (*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) - OBJ_173_ROTATION_INCREMENT < maxZRotation)
		{
			short int zRotDiff = *((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) - maxZRotation;
			zRotDiff = 0 - zRotDiff;
			short int incrementValue = (zRotDiff >= OBJ_173_ROTATION_INCREMENT) ? OBJ_173_ROTATION_INCREMENT : zRotDiff;
			*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) += incrementValue;
		}
		else
		{
			*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) = maxZRotation;
		}
	}
	
	// Update object's X Rotation
	if (!zNegDiff)
	{
		if (*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) + OBJ_173_ROTATION_INCREMENT < maxXRotation)
		{
			*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) += OBJ_173_ROTATION_INCREMENT;
		}
		else if (*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) + OBJ_173_ROTATION_INCREMENT > maxXRotation)
		{
			short int xRotDiff = *((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) - maxXRotation;
			short int decrementValue = (xRotDiff >= OBJ_173_ROTATION_INCREMENT) ? OBJ_173_ROTATION_INCREMENT : xRotDiff;
			*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) -= decrementValue;
		}
		else
		{
			*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) = maxXRotation;
		}
	}
	else
	{
		maxXRotation = 0 - maxXRotation;
		if (*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) - OBJ_173_ROTATION_INCREMENT > maxXRotation)
		{
			*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) -= OBJ_173_ROTATION_INCREMENT;
		}
		else if (*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) - OBJ_173_ROTATION_INCREMENT < maxXRotation)
		{
			short int xRotDiff = *((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) - maxXRotation;
			xRotDiff = 0 - xRotDiff;
			short int incrementValue = (xRotDiff >= OBJ_173_ROTATION_INCREMENT) ? OBJ_173_ROTATION_INCREMENT : xRotDiff;
			*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) += incrementValue;
		}
		else
		{
			*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) = maxXRotation;
		}
	}
	
	short int alignedRotX = *((volatile short int*)(OBJ_173_ObjectAddress + OBJ_173_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x04));
	short int alignedRotZ = *((volatile short int*)(OBJ_173_ObjectAddress + OBJ_173_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x06));
	short int alignedXUpper = alignedRotX + OBJ_173_CORRECT_ALIGNMENT_TOLERANCE;
	short int alignedXLower = alignedRotX - OBJ_173_CORRECT_ALIGNMENT_TOLERANCE;
	short int alignedZUpper = alignedRotZ + OBJ_173_CORRECT_ALIGNMENT_TOLERANCE;
	short int alignedZLower = alignedRotZ - OBJ_173_CORRECT_ALIGNMENT_TOLERANCE;
	if ( (*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) > alignedXLower && 
		*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_X_ROT_OFFSET)) < alignedXUpper) && 
		(*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) > alignedZLower && 
		*((volatile short int*)(OBJ_173_ObjectAddress + OBJ_Z_ROT_OFFSET)) < alignedZUpper) )
	{
		*((volatile byte*)(OBJ_173_ObjectAddress + OBJ_173_FREE_EIGHT_BYTES_OBJECT_OFFSET)) |= 0x02;
		OBJ_173_NumMirrorsAligned++;
		
		OBJ_173_TmpPosArray[0] = playerXPos;
		OBJ_173_TmpPosArray[1] = playerYPos + 0x064000;
		OBJ_173_TmpPosArray[2] = playerZPos;
		SpawnActor(OBJ_306_ACTOR_ID, (short int)OBJ_173_NumMirrorsAligned, OBJ_173_TmpPosArray, 0);
	}
}

// Disable balancing, part 1
void repl_021358E4_ov_5F() { }

// Disable balancing, part 2
void repl_02135920_ov_5F() { }

// Disable original X rotation, part 1
void repl_02135B1C_ov_5F() { }

// Disable original X rotation, part 2
void repl_02135B30_ov_5F() { }

// Disable original X rotation, part 3
void repl_02135D4C_ov_5F() { }

// Disable original X rotation, part 4 (ground pound)
void repl_02135898_ov_5F() { }

// Disable original X rotation, part 5 (ground pound)
void repl_021358A8_ov_5F() { }
