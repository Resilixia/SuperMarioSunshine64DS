#include "SM64DS.h"

// Parameters:
// 		Parameter 1:
// 		Byte (1 = Rightmost bit)	Desc.
// 		1 - 2						X Size * 100
//		3 - 4						Z Size * 100
// 
// 		Parameter 2:
// 		Byte (1 = Rightmost bit)	Desc.
// 		1 - 2						Y Size * 100
// 		3							Value by which 32768 should be multiplied to get bounce speed						
short int const OBJ_243_OBJ_PARAMETER_01_OFFSET = 0x08;
short int const OBJ_243_OBJ_PARAMETER_02_OFFSET = 0x92;
short int const OBJ_243_OBJ_PARAMETER_03_OFFSET = 0x90;

// OBJ_243_FREE_FOUR_BYTES_OBJECT_OFFSET:
// Offset 	Size	Desc.
// 0x00		1		Bit 1: 0 - false, 1 - true: whether this is a bouncy surface
// 0x01		1		X Size as specified in Parameter 02
// 0x02		1		Z Size as specified in Parameter 02
// 0x03		1		Y Size as specified in Parameter 03

short int const OBJ_243_FREE_FOUR_BYTES_OFFSET = 0xA0;

short int const PLAYER_ON_GROUND_RELATED_VARIABLE_OFFSET = 0x390;
short int const PLAYER_ON_GROUND_BOOL_OFFSET = 0x391;

int const OBJ_243_BOUNCE_SPEED_INCREMENT = 32768;
int const OBJ_243_PLAYER_ZERO_SPEED = 0xFFFF8000;

short int const OBJ_243_PLAYER_NEARBY_DETECTION_VARIABLE01_OFFSET = 0xB4;
short int const OBJ_243_PLAYER_NEARBY_DETECTION_VARIABLE02_OFFSET = 0xB8;
short int const OBJ_243_PLAYER_NEARBY_DETECTION_VARIABLE03_OFFSET = 0xBC;
int const OBJ_243_PLAYER_NEARBY_DETECTION_BOUNDS_VALUE = 0x00600000;

int const OBJ_243_Y_SHIFT_SIZE = 0xFFC18000; // -1

unsigned int OBJ_243_ObjectAddress = 0xFFFFFFFF;
int OBJ_243_BOUNCE_SPEED = 0x00040000;

void hook_020B068C_ov_02()
{
	asm
	(
		"ldr r0, =OBJ_243_ObjectAddress		\t\n"
		"str r4, [r0]						\t\n"
	);
	
	//short int param01 = *((volatile short int*)(OBJ_243_ObjectAddress + OBJ_243_OBJ_PARAMETER_01_OFFSET));
	short int param02 = *((volatile short int*)(OBJ_243_ObjectAddress + OBJ_243_OBJ_PARAMETER_02_OFFSET));
	short int param03 = *((volatile short int*)(OBJ_243_ObjectAddress + OBJ_243_OBJ_PARAMETER_03_OFFSET));
	
	byte xSize = (param02 & 0xFF);
	byte zSize = ((param02 & 0xFF00) >> 8);
	byte ySize = (param03 & 0xFF);
	byte bounceSpeed = ((param03 & 0xFF00) >> 8);
	
	if (xSize || ySize || zSize)
	{	
		*((volatile byte*)(OBJ_243_ObjectAddress + OBJ_243_FREE_FOUR_BYTES_OFFSET + 0x00)) |= 0x01;
	}
	
	*((volatile byte*)(OBJ_243_ObjectAddress + OBJ_243_FREE_FOUR_BYTES_OFFSET + 0x01)) = xSize;
	*((volatile byte*)(OBJ_243_ObjectAddress + OBJ_243_FREE_FOUR_BYTES_OFFSET + 0x02)) = zSize;
	*((volatile byte*)(OBJ_243_ObjectAddress + OBJ_243_FREE_FOUR_BYTES_OFFSET + 0x03)) = ySize;
	
	OBJ_243_BOUNCE_SPEED = OBJ_243_BOUNCE_SPEED_INCREMENT * bounceSpeed;
	
	// Because you can still collide with a pole of size zero, shift its Y location out of the way,  
	// subtract shift when calculating current Y position
	*((volatile int*)(OBJ_243_ObjectAddress + OBJ_Y_LOC_OFFSET)) += OBJ_243_Y_SHIFT_SIZE;
}

// Increase the boundary at which the Player is considered nearby
void hook_020B06C4_ov_02()
{
	asm
	(
		"ldr r0, =OBJ_243_ObjectAddress		\t\n"
		"str r4, [r0]						\t\n"
	);
	
	*((volatile int*)(OBJ_243_ObjectAddress + OBJ_243_PLAYER_NEARBY_DETECTION_VARIABLE01_OFFSET)) = OBJ_243_PLAYER_NEARBY_DETECTION_BOUNDS_VALUE;
	*((volatile int*)(OBJ_243_ObjectAddress + OBJ_243_PLAYER_NEARBY_DETECTION_VARIABLE02_OFFSET)) = OBJ_243_PLAYER_NEARBY_DETECTION_BOUNDS_VALUE;
	*((volatile int*)(OBJ_243_ObjectAddress + OBJ_243_PLAYER_NEARBY_DETECTION_VARIABLE03_OFFSET)) = OBJ_243_PLAYER_NEARBY_DETECTION_BOUNDS_VALUE;
}

void hook_020B0660_ov_02()
{
	asm
	(
		"ldr r0, =OBJ_243_ObjectAddress		\t\n"
		"str r4, [r0]						\t\n"
	);
	
	bool bouncy = *((volatile byte*)(OBJ_243_ObjectAddress + OBJ_243_FREE_FOUR_BYTES_OFFSET + 0x00)) & 0x01;
	
	if (!bouncy) return;
	
	bool playerOnGround = *((volatile byte*)PLAYER + PLAYER_ON_GROUND_BOOL_OFFSET);
	
	if (!playerOnGround) return;
	
	byte xSize = *((volatile byte*)(OBJ_243_ObjectAddress + OBJ_243_FREE_FOUR_BYTES_OFFSET + 0x01));
	byte zSize = *((volatile byte*)(OBJ_243_ObjectAddress + OBJ_243_FREE_FOUR_BYTES_OFFSET + 0x02));
	byte ySize = *((volatile byte*)(OBJ_243_ObjectAddress + OBJ_243_FREE_FOUR_BYTES_OFFSET + 0x03));
	
	int xDist = (xSize * 10 * 4096);
	int zDist = (zSize * 10 * 4096);
	int yDist = (ySize * 10 * 4096);
	
	int currentX = *((volatile int*)(OBJ_243_ObjectAddress + OBJ_X_LOC_OFFSET));
	int currentY = *((volatile int*)(OBJ_243_ObjectAddress + OBJ_Y_LOC_OFFSET)) - OBJ_243_Y_SHIFT_SIZE;
	int currentZ = *((volatile int*)(OBJ_243_ObjectAddress + OBJ_Z_LOC_OFFSET));
	
	int upperX = currentX + xDist;
	int lowerX = currentX - xDist;
	int upperY = currentY + yDist;
	int lowerY = currentY - yDist;
	int upperZ = currentZ + zDist;
	int lowerZ = currentZ - zDist;
	
	int playerXPos = PLAYER->xPos;
	int playerYPos = PLAYER->yPos;
	int playerZPos = PLAYER->zPos;
	
	if ( !(playerXPos >= lowerX && playerXPos <= upperX) ) return;
	if ( !(playerYPos >= lowerY && playerYPos <= upperY) ) return;
	if ( !(playerZPos >= lowerZ && playerZPos <= upperZ) ) return;
	
	// If we've reached here, Player is within bounds
	
	PLAYER->ySpeed = OBJ_243_BOUNCE_SPEED;
}
