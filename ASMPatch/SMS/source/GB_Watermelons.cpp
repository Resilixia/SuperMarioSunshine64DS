#include "GelatoBeach.h"

// Rollable Melon:
// Can be rolled about, can collide with each other and if 
// colliding with a Cataquack gets thrown up in air, explodes 
// upon hitting the ground and is reset to its starting 
// position.
// Either size 2 or 3, two 2's and one 3 watermelons, the size 
// 3 one will win Mario the competition, the others earn him 
// only some coins when brought to juice bar. 
//
// Competitor Melon:
// Fixed positions, rollable melons may collide with them 
// but the player can't move them.
//
// Goal Marker:
// Can have its size set, when colliding with Player will 
// check whether a rollable melon in range and will be 
// used to determine whether Mario's won. Does not appear 
// visible and has no collision.

// Parameter 1:
// Same as original object. For rollable melons should be 
// FF02 and for the other two, FF02
// Parameter 2:
// byte 1 (left): 0 - Competitor, 1 - Rollable, 2 - Goal 
// Marker
// byte 2(right): Size of melon, between 00 and 30, divided 
// by 16 then multiplied by 0x1000 to get model size and by 
// 0.1 to get the collision size

const short int PUSH_BLOCK_ACTOR_ID = 0xC0; // 192

////////////////////// OBJ 014 GORO_ROCK //////////////////////

// OBJ_014_FOUR_FREE_BYTES_01_OFFSET:
// byte 1: Watermelon size: 0x10 = 1, 0x20 = 2 etc. 
// byte 2: bit 1: bool, whether the current watermelon's been attacked 
// 				  by a Cataquack
// 		   bit 2: bool, whether after being attacked by a Cataquack 
// 		   it's been thrown up into the air
// 		   bit 3: bool, whether it's a competitor's melon
//  	   bit 4: bool, whether it's one of the 3 rollable melons
//  	   bit 5: bool, whether it's the goal marker
// 		   bit 6: bool, whether it's on goal marker
// 		   bit 7: bool, should explode for losing contest

// OBJ_014_FOUR_FREE_BYTES_02_OFFSET:
// Stores the address of the associated Push Block

short int const PLAYER_COLLISION_OFFSET_01_DWORD = 0x1D8;
short int const PLAYER_COLLISION_OFFSET_02_WORD = 0x1DC;
short int const GROUND_COLLISION_OFFSET_BYTE = 0x208;
short int const COLLISION_SPHERE_RADIUS_OFFSET = 0x210;

int const OBJ_014_MAX_FORWARD_SPEED = 0x8000;

int const OBJ_014_GOAL_RADIUS = 0xC8000; // 0.2

bool OBJ_014_IsOnSlopeZ = false;
bool OBJ_014_IsOnSlopeX = false;
bool OBJ_014_IsOnGround = false;
bool OBJ_014_IsPlayerColliding = false;
bool OBJ_014_IsHittingWall = false;

int OBJ_014_OriginalCalculatedZSpeed = 0;
int OBJ_014_FinalCalculatedZSpeed = 0;
int OBJ_014_OriginalCalculatedXSpeed = 0;
int OBJ_014_FinalCalculatedXSpeed = 0;

unsigned int OBJ_014_ObjectAddress = 0xFFFFFFFF;

byte OBJ_014_WatermelonIndex = 0;

const byte OBJ_014_WINNING_MELON_SIZE = 0x30;

const short int OBJECT_ROT_DEGREES_IN_SEMICIRCLE = degreesToAngle(180);
const short int OBJECT_ROT_DEGREES_IN_SEMICIRCLE_OVER_TWO = OBJECT_ROT_DEGREES_IN_SEMICIRCLE / 2;
const short int OBJECT_ROT_DEGREES_IN_SEMICIRCLE_STEP = degreesToAngle(10);
const short int OBJ_014_MARIO_FACING_DIRECTION_PUSH_IN_X_POS_START = degreesToAngle(0);
const short int OBJ_014_MARIO_FACING_DIRECTION_PUSH_IN_X_NEG_START = degreesToAngle(180) * 2;
const short int OBJ_014_MARIO_FACING_DIRECTION_PUSH_IN_Z_POS_START = degreesToAngle(270) * 2;
const short int OBJ_014_MARIO_FACING_DIRECTION_PUSH_IN_Z_NEG_START = degreesToAngle(90) * 2;

// Set the initial forward speed to zero
void repl_02112CA8_ov_15()
{
	asm
	(
		"mov r0, #0x0000		\t\n"
	);
}

// Set the initial value at +0xAC to zero
void repl_02112CB4_ov_15()
{
	asm
	(
		"mov r0, #0x0000		\t\n"
	);
}

// Create a (245) Push Block object that will follow OBJ 014 around 
// to provide a KCL map
short int const OBJ_014_PUSH_BLOCK_PARAM_01 = 0x0001;
int OBJ_014_PushBlockPosition[3] = { 0x00, 0x00, 0x00 };
short OBJ_014_PushBlockRotation[3] = { 0x00, 0x00, 0x00 };
void hook_02112CC4_ov_15()
{
	asm
	(
		"ldr r0, =OBJ_014_ObjectAddress		\t\n"
		"str r4, [r0]						\t\n"
	);	
	
	bool competitor = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x04;
	bool rollable = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x08;
	bool goalMarker = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x10;
	
	if (goalMarker)
	{
		GB_WatermelonGoal = OBJ_014_ObjectAddress;
		return;
	}
	if (!competitor && rollable)
	{	
		if (OBJ_014_WatermelonIndex >= GB_NumWatermelons)
		{
			OBJ_014_WatermelonIndex = 0;
			GB_CompetitionWon = false;
		}
		
		// Store the address of all watermelons for access by Popoi objects, 
		// didn't want to do it this way but couldn't find how to get the 
		// object another was colliding with
		GB_Watermelons[OBJ_014_WatermelonIndex++] = OBJ_014_ObjectAddress;
	}
	
	OBJ_014_PushBlockPosition[0] = 
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_X_LOC_OFFSET)); 
	OBJ_014_PushBlockPosition[1] = 
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_Y_LOC_OFFSET)); 
	OBJ_014_PushBlockPosition[2] = 
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_Z_LOC_OFFSET)); 
	
	OBJ_014_PushBlockRotation[1] = 
		*((volatile short int*)(OBJ_014_ObjectAddress + OBJ_Y_ROT_OFFSET));
	
	unsigned int pushBlockAddress = SpawnActor(
					PUSH_BLOCK_ACTOR_ID, 
					OBJ_014_PUSH_BLOCK_PARAM_01, 
					OBJ_014_PushBlockPosition, 
				    OBJ_014_PushBlockRotation);
	
	*((volatile unsigned int*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_02_OFFSET)) = 
		pushBlockAddress;
	
	*((volatile int*)(OBJ_014_ObjectAddress + OBJ_014_TWELVE_FREE_BYTES_OFFSET + 0x00)) = 
		OBJ_014_PushBlockPosition[0];
	*((volatile int*)(OBJ_014_ObjectAddress + OBJ_014_TWELVE_FREE_BYTES_OFFSET + 0x04)) = 
		OBJ_014_PushBlockPosition[1];
	*((volatile int*)(OBJ_014_ObjectAddress + OBJ_014_TWELVE_FREE_BYTES_OFFSET + 0x08)) = 
		OBJ_014_PushBlockPosition[2];
}

// Change the scale of the object depending on parameter 2 1/3 - 
// Modify the bone scale values in the BMD file to 1 * parameter 2
unsigned int OBJ_014_BMDAddress;
void hook_02112B80_ov_15()
{
	asm
	(
		"ldr r0, =OBJ_014_BMDAddress	\t\n"
		"str r1, [r0]					\t\n"
		"ldr r0, =OBJ_014_ObjectAddress	\t\n"
		"str r4, [r0]					\t\n"
	);
	
	unsigned int boneSectionAddress = 
		*((volatile unsigned int*)(OBJ_014_BMDAddress + 0x08));
	
	byte param02_01 = *((volatile byte*)(OBJ_014_ObjectAddress + 0x92 + 0x00));
	// byte sizeGroup = (byte)((param02_01 & 0xF0) >> 4);
	
	*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET)) = 
		param02_01;
	
	int scale = (param02_01 * 0x100);
	
	byte param02_02 = *((volatile byte*)(OBJ_014_ObjectAddress + 0x92 + 0x01));
	if (param02_02 == 0x00)
	{
		// Static, competitor
		*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) |= 
			0x04;
	}
	else if (param02_02 == 0x01)
	{
		// One of the three rollable melons
		*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) |= 
			0x08;
	}
	else if (param02_02 == 0x02)
	{
		// Goal marker
		*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) |= 
			0x10;
		*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET)) = 0x00;
		scale = 0x0000;
	}
	
	*((volatile int*)(boneSectionAddress + 0x10)) = scale;
	*((volatile int*)(boneSectionAddress + 0x14)) = scale;
	*((volatile int*)(boneSectionAddress + 0x18)) = scale;
}

// Change the scale of the object depending on parameter 2 2/3 - 
// Set size of collision sphere radius equal to 0.00625 * parameter 2
void repl_02112BC0_ov_15()
{
	asm
	(
		"mov r1, #0x6400			\t\n"
		"ldrb r0, [r4, #0x84]		\t\n"
		"mul r2, r0, r1				\t\n"
		
		"mov r1, r4					\t\n" // Original instruction
	);
}
// Disable the original write to r2 of 0x12C000 (0.3)
void repl_02112BC8_ov_15() { }

// Change the scale of the object depending on parameter 2 3/3 - 
// Set size of Player collision "Clipper" to 0.005 * parameter 2
void repl_02112C64_ov_15()
{
	asm
	(
		"mov r1, #0x5000		\t\n"
		"ldrb r0, [r4, #0x84]	\t\n"
		"mul r3, r0, r1			\t\n"
		
		"mov r1, r4				\t\n" // Original instruction
	);
}
// Disable the original write to r3 of #0x0F3000 
void repl_02112C6C_ov_15() { }

// Change the scale of the object depending on parameter 2 cleanup - 
// Reset values at +0x92 and +0x8C from parameter 2 to zero
void hook_02112C74_ov_15()
{
	asm
	(
		"mov r0, #0x00			\t\n"
		"strh r0, [r4, #0x8C]	\t\n"
		"strh r0, [r4, #0x92]	\t\n"
		"strh r0, [r4, #0x90]	\t\n"
	);
}

// Read whether the Player is colliding with OBJ 014
void hook_02112874_ov_15()
{
	asm
	(
		"ldr r0, =OBJ_014_ObjectAddress		\t\n"
		"str r4, [r0]						\t\n"
	);
	
	OBJ_014_IsPlayerColliding = 
		( *((volatile int*)(OBJ_014_ObjectAddress + PLAYER_COLLISION_OFFSET_02_WORD)) > 0x00 );
	
	OBJ_014_IsOnGround = 
		( *((volatile int*)(OBJ_014_ObjectAddress + GROUND_COLLISION_OFFSET_BYTE)) == 0x90 );
}

// Read whether watermelon colliding with wall
void hook_02112730_ov_15()
{
	asm
	(
		"ldr r1, =OBJ_014_IsHittingWall		\t\n"
		"str r0, [r1]						\t\n"
	);
}

// Reduce speed multiplier used in below location for calculating new 
// forward speed
void repl_0211269C_ov_15()
{
	asm
	(
		"mov r1, #0x02		\t\n"
	);
}

// Code used to calculate updated x speed: new speed = 
// (r1 const * slope) + old speed
// Get whether the object is on a slope or not
void hook_02112698_ov_15()
{
	asm
	(
		// Store whether rock is on a slope
		"ldr r0, =OBJ_014_IsOnSlopeX	\t\n"
		"mov r1, #0x00					\t\n"
		"cmp r2, #0x100					\t\n"
		"movgt r1, #0x01				\t\n"
		"strb r1, [r0]					\t\n"
	);
}

void repl_021126A4_ov_15()
{
	asm
	(
		"ldr r0, =OBJ_014_OriginalCalculatedXSpeed		\t\n"
		"str r3, [r0]									\t\n"
	);
}

// Code used to calculate updated z speed: new speed = 
// (r1 const * slope) + old speed
// Get whether the object is on a slope or not
void hook_021126B0_ov_15()
{
	asm
	(
		// Store whether rock is on a slope
		"ldr r0, =OBJ_014_IsOnSlopeZ	\t\n"
		"mov r1, #0x00					\t\n"
		"cmp r2, #0x100					\t\n"
		"movgt r1, #0x01				\t\n"
		"strb r1, [r0]					\t\n"
	);
}

// Calculate X and Z Speeds
void hook_021126B4_ov_15()
{
	asm
	(
		"ldr r2, =OBJ_014_ObjectAddress				\t\n"
		"str r4, [r2]								\t\n"
		"ldr r2, =OBJ_014_OriginalCalculatedZSpeed	\t\n"
		"str r1, [r2]								\t\n"
	);
	
	bool competitor = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x04;
	bool goalMarker = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x10;
	bool onGoal = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x20;
	if (competitor || goalMarker || onGoal) 
	{
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_X_SPEED_OFFSET)) = 0;
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_Z_SPEED_OFFSET)) = 0;
		return; 
	}
	
	int currentXLocation = *((volatile int*)(OBJ_014_ObjectAddress + OBJ_X_LOC_OFFSET));
	int currentZLocation = *((volatile int*)(OBJ_014_ObjectAddress + OBJ_Z_LOC_OFFSET));
	
	OBJ_014_FinalCalculatedXSpeed = OBJ_014_OriginalCalculatedXSpeed;
	OBJ_014_FinalCalculatedZSpeed = OBJ_014_OriginalCalculatedZSpeed;
	
	if (OBJ_014_FinalCalculatedXSpeed > OBJ_014_MAX_FORWARD_SPEED)
	{
		OBJ_014_FinalCalculatedXSpeed = OBJ_014_MAX_FORWARD_SPEED;
	}
	if (OBJ_014_FinalCalculatedXSpeed < (0 - OBJ_014_MAX_FORWARD_SPEED))
	{
		OBJ_014_FinalCalculatedXSpeed = 0 - OBJ_014_MAX_FORWARD_SPEED;
	}
	if (OBJ_014_FinalCalculatedZSpeed > OBJ_014_MAX_FORWARD_SPEED)
	{
		OBJ_014_FinalCalculatedZSpeed = OBJ_014_MAX_FORWARD_SPEED;
	}
	if (OBJ_014_FinalCalculatedZSpeed < (0 - OBJ_014_MAX_FORWARD_SPEED))
	{
		OBJ_014_FinalCalculatedZSpeed = 0 - OBJ_014_MAX_FORWARD_SPEED;
	}
	
	int previousXSpeed = *((volatile int*)(OBJ_014_ObjectAddress + OBJ_X_SPEED_OFFSET));
	int previousZSpeed = *((volatile int*)(OBJ_014_ObjectAddress + OBJ_Z_SPEED_OFFSET));
	
	short int MarioYRot = (short int)(PLAYER->yRot);
	
	int diffX = PLAYER->xPos - currentXLocation;
	int diffZ = PLAYER->zPos - currentZLocation;
	
	if (diffX < 0) diffX = 0 - diffX;
	if (diffZ < 0) diffZ = 0 - diffZ;
	
	byte melonSize = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET));
	melonSize = (byte)((melonSize & 0xF0) >> 4);
	
	// 327680 : 0.08
	short int stepsDiffX = diffX / 163840;
	short int stepsDiffZ = diffZ / 163840;
	
	int speedStep = ((4 - melonSize) * 0x1000);
	
	if ((Player_CurrentAction == ACT_Punch) || 
		(Player_CurrentAction == ACT_DoublePunch) || 
		(Player_CurrentAction == ACT_Kick))
	{
		speedStep += 0x0800;
	}
	
	if (OBJ_014_IsPlayerColliding)
	{
		if (PLAYER->xPos > currentXLocation)
		{
			if (MarioYRot < OBJ_014_MARIO_FACING_DIRECTION_PUSH_IN_X_NEG_START)
			{
				stepsDiffX = 0;
			}
			
			OBJ_014_FinalCalculatedXSpeed = (previousXSpeed - (stepsDiffX * speedStep));
		}
		else if (PLAYER->xPos < currentXLocation)
		{
			if (MarioYRot > (OBJ_014_MARIO_FACING_DIRECTION_PUSH_IN_X_POS_START + OBJECT_ROT_DEGREES_IN_SEMICIRCLE))
			{
				stepsDiffX = 0;
			}
			
			OBJ_014_FinalCalculatedXSpeed = (previousXSpeed + (stepsDiffX * speedStep));
		}
		
		if (PLAYER->zPos > currentZLocation)
		{
			if (MarioYRot < OBJ_014_MARIO_FACING_DIRECTION_PUSH_IN_Z_NEG_START || 
				MarioYRot > (OBJ_014_MARIO_FACING_DIRECTION_PUSH_IN_Z_NEG_START + OBJECT_ROT_DEGREES_IN_SEMICIRCLE))
			{
				stepsDiffZ = 0;
			}
			
			OBJ_014_FinalCalculatedZSpeed = (previousZSpeed - (stepsDiffZ * speedStep));
		}
		else if (PLAYER->zPos < currentZLocation)
		{
			if (MarioYRot <= OBJ_014_MARIO_FACING_DIRECTION_PUSH_IN_Z_POS_START && 
				MarioYRot >= (OBJ_014_MARIO_FACING_DIRECTION_PUSH_IN_Z_POS_START - OBJECT_ROT_DEGREES_IN_SEMICIRCLE))
			{
				stepsDiffZ = 0;
			}
			
			OBJ_014_FinalCalculatedZSpeed = (previousZSpeed + (stepsDiffZ * speedStep));
		}
	}
	else if (!OBJ_014_IsPlayerColliding && OBJ_014_IsOnGround)
	{
		// Gradually come to a halt
		if (!OBJ_014_IsOnSlopeX)
		{
			if (previousXSpeed > 0)
			{
				int result = previousXSpeed - 0x400;
				OBJ_014_FinalCalculatedXSpeed = (result >= 0) ? 
					result : 0;
			}
			else if (previousXSpeed < 0)
			{
				int result = previousXSpeed + 0x400;
				OBJ_014_FinalCalculatedXSpeed = (result <= 0) ? 
					result : 0;
			}
			else if (previousXSpeed == 0)
			{
				OBJ_014_FinalCalculatedXSpeed = 0;
			}
		}
		
		if (!OBJ_014_IsOnSlopeZ)
		{
			if (previousZSpeed > 0)
			{
				int result = previousZSpeed - 0x400;
				OBJ_014_FinalCalculatedZSpeed = (result >= 0) ? 
					result : 0;
			}
			else if (previousZSpeed < 0)
			{
				int result = previousZSpeed + 0x400;
				OBJ_014_FinalCalculatedZSpeed = (result <= 0) ? 
					result : 0;
			}
			else if (previousZSpeed == 0)
			{
				OBJ_014_FinalCalculatedZSpeed = 0;
			}
		}
	}
	
	*((volatile int*)(OBJ_014_ObjectAddress + OBJ_X_SPEED_OFFSET)) = 
		OBJ_014_FinalCalculatedXSpeed;
	*((volatile int*)(OBJ_014_ObjectAddress + OBJ_Z_SPEED_OFFSET)) = 
		OBJ_014_FinalCalculatedZSpeed;
	
	// Calculate the Y rotation based on X and Z speeds
	if (OBJ_014_FinalCalculatedXSpeed != 0 ||
		OBJ_014_FinalCalculatedZSpeed != 0)
	{
		unsigned short int yAngle = (unsigned short int)atan2Cordic(
			OBJ_014_FinalCalculatedZSpeed, OBJ_014_FinalCalculatedXSpeed);
		
		// Need to double result for object angles
		*((volatile short int*)(OBJ_014_ObjectAddress + OBJ_Y_ROT_OFFSET)) = 
			(2 * yAngle);
	}
}

// Disable the original write of Z speed
void repl_021126B8_ov_15() { }

// Disable the original X and Y rotation calculation and writes
void repl_021128CC_ov_15() { } // Write to +0x8E
void repl_021126CC_ov_15() { } // atan2 call
void repl_021126D0_ov_15() { } // Write to +0x94

// Throw watermelon up in air if attacked by a Cataquack
void hook_021128E4_ov_15()
{
	asm
	(
		"ldr r1, =OBJ_014_ObjectAddress		\t\n"
		"str r4, [r1]						\t\n"
	);
	
	bool beingAttacked = 
		*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x01;
	bool beenThrown = 
		*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x02;
	bool popTooSmall = 
		*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x40;
	bool competitor = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x04;
	bool goalMarker = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x10;
		
	int currentYSpeed = *((volatile int*)(OBJ_014_ObjectAddress + OBJ_Y_SPEED_OFFSET));
	
	if (beingAttacked && !beenThrown)
	{
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_Y_SPEED_OFFSET)) = 0xA0000;
		*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) |= 
			0x02;
	}
	else if ( (beingAttacked && beenThrown && currentYSpeed < 0 && OBJ_014_IsOnGround) || 
		popTooSmall )
	{
		// No movement
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_X_SPEED_OFFSET)) = 0;
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_Y_SPEED_OFFSET)) = 0;
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_Z_SPEED_OFFSET)) = 0;
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_FORWARD_SPEED_OFFSET)) = 0;
		
		// Explode
		int currentXPos = *((volatile int*)(OBJ_014_ObjectAddress + OBJ_X_LOC_OFFSET));
		int currentYPos = *((volatile int*)(OBJ_014_ObjectAddress + OBJ_Y_LOC_OFFSET));
		int currentZPos = *((volatile int*)(OBJ_014_ObjectAddress + OBJ_Z_LOC_OFFSET));
		
		ParticleEffect(0x0D, currentXPos, currentYPos, currentZPos);
		ParticleEffect(0x14, currentXPos, currentYPos, currentZPos);
		ParticleEffect(0x16, currentXPos, currentYPos, currentZPos);
		ParticleEffect(0x17, currentXPos, currentYPos, currentZPos);
		ParticleEffect(0x1D, currentXPos, currentYPos, currentZPos);
		
		// Reset to starting position
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_X_LOC_OFFSET)) = 
			*((volatile int*)(OBJ_014_ObjectAddress + OBJ_014_TWELVE_FREE_BYTES_OFFSET + 0x00));
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_Y_LOC_OFFSET)) = 
			*((volatile int*)(OBJ_014_ObjectAddress + OBJ_014_TWELVE_FREE_BYTES_OFFSET + 0x04));
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_Z_LOC_OFFSET)) = 
			*((volatile int*)(OBJ_014_ObjectAddress + OBJ_014_TWELVE_FREE_BYTES_OFFSET + 0x08));
		
		// Reset state
		*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) &= 0xFE; // beingAttacked
		*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) &= 0xFD; // beenThrown
		*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) &= 0xDF; // onGoal
		*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) &= 0xBF; // popTooSmall
	}
	else if (!competitor && !goalMarker)
	{
		// Check whether it's on top of goal
		int goalXLocation = *((volatile int*)(GB_WatermelonGoal + OBJ_X_LOC_OFFSET));
		int goalYLocation = *((volatile int*)(GB_WatermelonGoal + OBJ_Y_LOC_OFFSET));
		int goalZLocation = *((volatile int*)(GB_WatermelonGoal + OBJ_Z_LOC_OFFSET));
		int currentXLocation = *((volatile int*)(OBJ_014_ObjectAddress + OBJ_X_LOC_OFFSET));
		int currentYLocation = *((volatile int*)(OBJ_014_ObjectAddress + OBJ_Y_LOC_OFFSET));
		int currentZLocation = *((volatile int*)(OBJ_014_ObjectAddress + OBJ_Z_LOC_OFFSET));
		int goalDiffX = currentXLocation - goalXLocation;
		int goalDiffY = currentYLocation - goalYLocation;
		int goalDiffZ = currentZLocation - goalZLocation;
		if (goalDiffX < 0) goalDiffX = 0 - goalDiffX;
		if (goalDiffY < 0) goalDiffY = 0 - goalDiffY;
		if (goalDiffZ < 0) goalDiffZ = 0 - goalDiffZ;
		if (goalDiffX <= OBJ_014_GOAL_RADIUS && 
			/*goalDiffY <= OBJ_014_GOAL_RADIUS && */
			goalDiffZ <= OBJ_014_GOAL_RADIUS)
		{
			*((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) |= 0x20;
			
			GB_WatermelonCurrentEntry = OBJ_014_ObjectAddress;
			
			byte melonSize = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET));
			if (melonSize == OBJ_014_WINNING_MELON_SIZE)
			{
				GB_CompetitionWon = true;
			}
		}
	}
}

// Update the position of the current melon's Push Block object
void hook_0211293C_ov_15()
{
	asm
	(
		"ldr r1, =OBJ_014_ObjectAddress		\t\n"
		"str r4, [r1]						\t\n"
	);
	
	bool goalMarker = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x10;
	if (goalMarker) return;
	
	byte melonSize = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET));
	
	// 0.00625 x melonSize 
	int radius = 0x6400 * melonSize;
	
	unsigned int pushBlockAddress = 
		*((volatile unsigned int*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_02_OFFSET));
	
	if (pushBlockAddress == 0x00000000 || pushBlockAddress == 0xFFFFFFFF) return;
	
	*((volatile int*)(pushBlockAddress + OBJ_X_LOC_OFFSET)) = 
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_X_LOC_OFFSET));
	*((volatile int*)(pushBlockAddress + OBJ_Y_LOC_OFFSET)) = 
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_Y_LOC_OFFSET)) - 
		(radius / 2);
	*((volatile int*)(pushBlockAddress + OBJ_Z_LOC_OFFSET)) = 
		*((volatile int*)(OBJ_014_ObjectAddress + OBJ_Z_LOC_OFFSET));
		
	*((volatile short int*)(pushBlockAddress + OBJ_Y_ROT_OFFSET)) = 
		*((volatile short int*)(OBJ_014_ObjectAddress + OBJ_Y_ROT_OFFSET));
}

// Disable the code that checks if forward speed >= 0x22000 and if 
// so sets the values of X and Z speed, a problem as it can overwrite 
// the changes we've calculated
void repl_021126D8_ov_15()
{
	asm
	(
		// Subsequent BLT always false as r12 huge
		"cmp r0, r12	\t\n" 
	);
}

// Change the size of the shadow depending on watermelon size 
// unless it's the Goal Marker in which case no shadow 1/2
int OBJ_014_ShadowSize = 0x64000;
void hook_0211237C_ov_15()
{
	asm
	(
		"ldr r1, =OBJ_014_ObjectAddress		\t\n"
		"str r5, [r1]						\t\n"
	);
	
	bool goalMarker = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) & 0x10;
	byte melonSize = *((volatile byte*)(OBJ_014_ObjectAddress + OBJ_014_FOUR_FREE_BYTES_01_OFFSET));
	
	if (goalMarker)
	{
		OBJ_014_ShadowSize = 0;
	}
	else
	{
		OBJ_014_ShadowSize = 0x140000 + (((melonSize & 0xF0) >> 4) * 0x10000);
	}
}

// Change the size of the shadow depending on watermelon size 
// unless it's the Goal Marker in which case no shadow 2/2
void repl_02112380_ov_15()
{
	asm
	(
		"ldr r3, =OBJ_014_ShadowSize	\t\n"
		"ldr r3, [r3]					\t\n"
	);
}

// Disable the dust cloud effect when on flat ground
void repl_02112668_ov_15() { }

// Disable the dust cloud effect when on a slope
void repl_02112630_ov_15() { }

// Disable the Player taking damage upon contact with OBJ 014
void repl_02112878_ov_15() { }

////////////////////// OBJ 245 PUSH BLOCK //////////////////////

// If parameter 01 set to 1, it's inside a watermelon so don't show 
// the model
unsigned int OBJ_245_ObjectAddress = 0xFFFFFFFF;
unsigned int OBJ_245_BMDAddress = 0xFFFFFFFF;
void hook_020B9000_ov_02()
{
	asm
	(
		"ldr r0, =OBJ_245_ObjectAddress		\t\n"
		"str r4, [r0]						\t\n"
		"ldr r0, =OBJ_245_BMDAddress		\t\n"
		"str r1, [r0]						\t\n"
	);
	
	byte param01 = 
		*((volatile unsigned int*)(OBJ_245_ObjectAddress + 0x08));
	
	if (param01 != 0x01) return;
	
	unsigned int boneSectionAddress = 
		*((volatile unsigned int*)(OBJ_245_BMDAddress + 0x08));
	
	int scale = 0x0000;
	
	*((volatile int*)(boneSectionAddress + 0x10)) = scale;
	*((volatile int*)(boneSectionAddress + 0x14)) = scale;
	*((volatile int*)(boneSectionAddress + 0x18)) = scale;
}

// Enable updating of position without pushing block 1/2
void repl_020B8DF4_ov_02() 
{ 
	asm
	(
		"mov r0, #0x00		\t\n"
	);
}

// Enable updating of position without pushing block 2/2
void repl_020B8E14_ov_02() { }
