/* Modifies the Whomp King object's behaviour to suit Petey Piranha. 
 * 
 * The following changes are made to object behaviour:
 * 		1) Gradually increases the X rotation to a maximum when in flying state.
 * 		2) Gradually increases the X rotation to a maximum when in attacking states.
 *		3) Limits the X and Z positions to no more than +/- 0.8 the starting position.
 */

#include "SM64DS.h"

enum WhompKingState
{
	WK_WALK_START 	= 0x40056000,
	WK_WALK 		= 0x0003B000,
	WK_ATTACK_1		= 0x40013000,
	WK_ATTACK_2		= 0x4000F000,
	WK_DAMAGE		= 0x40015000,
	WK_RECOVER		= 0x4002B000
};

unsigned int OBJ_065_ObjectAddress;

short int const OBJ_065_CURRENT_STATE_OFFSET = 0x320;
short int const OBJ_065_ANIMATION_SPEED_OFFSET = 0x328;

WhompKingState OBJ_065_CurrentState;

int OBJ_065_FlyFrameCount = 0;
int const OBJ_065_NumFlyFrames = 8;
unsigned short int const OBJ_065_WK_FLYING_X_ROTATION_VALUE = 0x1000;
unsigned short int const OBJ_065_WK_FLY_X_ROTATION_INCREMENT = 0x200;

int OBJ_065_AttackFrameCount = 0;
int const OBJ_065_NumAttackFrames = 34;
unsigned short int const OBJ_065_WK_ATTACK_MAX_X_ROTATION_VALUE = 0x3C00;
unsigned short int const OBJ_065_WK_ATTACK_X_ROTATION_INCREMENT = 0x1C3;

int OBJ_065_CHASE_FORWARD_SPEED = 0x020000; // 131072d, 32 (unit?)
int OBJ_065_CHASE_PLAYER_DETECTION_DISTANCE = 16384000; // 4
int OBJ_065_CHASE_ANIMATION_SPEED = 0x1800; // 1.5

int OBJ_065_WKStartXPosition;
int OBJ_065_WKStartZPosition;
int const OBJ_065_WK_X_UPPER_BOUNDS = 0x00320000; // (3276800, 0.8)
int const OBJ_065_WK_X_LOWER_BOUNDS = 0xFFCE0000; // (-3276800, -0.8)
int const OBJ_065_WK_Z_UPPER_BOUNDS = 0x00320000; // (3276800, 0.8)
int const OBJ_065_WK_Z_LOWER_BOUNDS = 0xFFCE0000; // (-3276800, -0.8)

void hook_021241B8_ov_4F()
{
	asm
	(
		"ldr r0, =OBJ_065_ObjectAddress	\t\n"
		"str r5, [r0]					\t\n"
	);
	
	OBJ_065_WKStartXPosition = *((volatile int*)(OBJ_065_ObjectAddress + OBJ_X_LOC_OFFSET));
	OBJ_065_WKStartZPosition = *((volatile int*)(OBJ_065_ObjectAddress + OBJ_Z_LOC_OFFSET));
}

void hook_02124190_ov_4F()
{	
	OBJ_065_CurrentState = WhompKingState(*((volatile unsigned int*)(OBJ_065_ObjectAddress + OBJ_065_CURRENT_STATE_OFFSET)));
	
	if (OBJ_065_CurrentState == WK_WALK || OBJ_065_CurrentState == WK_WALK_START)
	{
		*((volatile short int*)(OBJ_065_ObjectAddress + OBJ_X_ROT_OFFSET)) = (OBJ_065_FlyFrameCount * OBJ_065_WK_FLY_X_ROTATION_INCREMENT);
		
		if (OBJ_065_FlyFrameCount < OBJ_065_NumFlyFrames) OBJ_065_FlyFrameCount++;
		
		OBJ_065_AttackFrameCount = 0;
	}
	else if (OBJ_065_CurrentState == WK_ATTACK_1 || OBJ_065_CurrentState == WK_ATTACK_2)
	{
		*((volatile short int*)(OBJ_065_ObjectAddress + OBJ_X_ROT_OFFSET)) = (OBJ_065_AttackFrameCount * OBJ_065_WK_ATTACK_X_ROTATION_INCREMENT);
		
		if (OBJ_065_AttackFrameCount < OBJ_065_NumAttackFrames) OBJ_065_AttackFrameCount++;
		
		OBJ_065_FlyFrameCount = 0;
	}
	else
	{
		OBJ_065_FlyFrameCount = 0;
		OBJ_065_AttackFrameCount = 0;
	}
	
	int xpos = *((volatile int*)(OBJ_065_ObjectAddress + OBJ_X_LOC_OFFSET));
	int zpos = *((volatile int*)(OBJ_065_ObjectAddress + OBJ_Z_LOC_OFFSET));
	
	if (xpos > (OBJ_065_WKStartXPosition + OBJ_065_WK_X_UPPER_BOUNDS))
	{
		*((volatile int*)(OBJ_065_ObjectAddress + OBJ_X_LOC_OFFSET)) = (OBJ_065_WKStartXPosition + OBJ_065_WK_X_UPPER_BOUNDS);
	}
	if (xpos < (OBJ_065_WKStartXPosition + OBJ_065_WK_X_LOWER_BOUNDS))
	{
		*((volatile int*)(OBJ_065_ObjectAddress + OBJ_X_LOC_OFFSET)) = (OBJ_065_WKStartXPosition + OBJ_065_WK_X_LOWER_BOUNDS);
	}
	if (zpos > (OBJ_065_WKStartZPosition + OBJ_065_WK_Z_UPPER_BOUNDS))
	{
		*((volatile int*)(OBJ_065_ObjectAddress + OBJ_Z_LOC_OFFSET)) = (OBJ_065_WKStartZPosition + OBJ_065_WK_Z_UPPER_BOUNDS);
	}
	if (zpos < (OBJ_065_WKStartZPosition + OBJ_065_WK_Z_LOWER_BOUNDS))
	{
		*((volatile int*)(OBJ_065_ObjectAddress + OBJ_Z_LOC_OFFSET)) = (OBJ_065_WKStartZPosition + OBJ_065_WK_Z_LOWER_BOUNDS);
	}
}

// Disable the shadow
void repl_021242F0_ov_4F() { }

// Change the particle effect when WK defeated to red, orange and yellow circles
void repl_02124908_ov_4F()
{
	asm
	(
		"mov r0, #0x17		\t\n"
	);
}

// Change the particle effect when WK ground-pounded to red, orange and yellow circles
void repl_02124F30_ov_4F()
{
	asm
	(
		"mov r0, #0x17		\t\n"
	);
}

// Disable stopping every few steps when chasing player
void repl_02125568_ov_4F() { }

// Disable the footstep sound effect
void repl_02125590_ov_4F() { }

// Disable dust cloud footsteps
void repl_021256A4_ov_4F() { }

// Increase player detection distance to begin chasing
void repl_02125798_ov_4F()
{
	asm
	(
		"ldr r0, =OBJ_065_CHASE_PLAYER_DETECTION_DISTANCE	\t\n"
		"ldr r0, [r0]										\t\n"
	);
}

// Set chasing speed
void repl_021257A0_ov_4F()
{
	asm
	(
		"ldrlt r0, =OBJ_065_CHASE_FORWARD_SPEED		\t\n"
		"ldrlt r0, [r0]								\t\n"
	);
}
