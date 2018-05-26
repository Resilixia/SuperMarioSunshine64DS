#include "SM64DS.h"
#include "atan2.h"
#include "MathsHelper.h"
#include "PositionAndRotationHelper.h"

/* 
 * Monty Mole mini-boss
 */

short int const OBJ_208_ACTOR_ID = 53; 	// BK_DOSSUNBAR_L
short int const OBJ_248_ACTOR_ID = 310; // CHOROPU
short int const OBJ_249_ACTOR_ID = 311;	// CHORO_ROCK
short int const OBJ_61_ACTOR_ID = 178; // STAR
short int const OBJ_63_ACTOR_ID = 180; // STARBASE

const byte MONTY_MOLE_STAR_ID = 0x01;

bool IsMarioWithinDistanceFromObjectCompletely(unsigned int object_address, int distance);
bool IsMarioOutsideDistanceFromObjectPartially(unsigned int object_address, int distance);

__attribute__((noinline)) bool IsMarioWithinDistanceFromObjectCompletely(unsigned int object_address, int distance) 
{
	int xPos = *((volatile int*)(object_address + OBJ_X_LOC_OFFSET));
	int yPos = *((volatile int*)(object_address + OBJ_Y_LOC_OFFSET));
	int zPos = *((volatile int*)(object_address + OBJ_Z_LOC_OFFSET));
	int xDiff = xPos - PLAYER->xPos;
	int yDiff = yPos - PLAYER->yPos;
	int zDiff = zPos - PLAYER->zPos;
	
	return (((xDiff < 0) ? (0 - xDiff) : (xDiff)) < distance && 
			((yDiff < 0) ? (0 - yDiff) : (yDiff)) < distance && 
			((zDiff < 0) ? (0 - zDiff) : (zDiff)) < distance);
}

__attribute__((noinline)) bool IsMarioOutsideDistanceFromObjectPartially(unsigned int object_address, int distance) 
{
	int xPos = *((volatile int*)(object_address + OBJ_X_LOC_OFFSET));
	int zPos = *((volatile int*)(object_address + OBJ_Z_LOC_OFFSET));
	int xDiff = xPos - PLAYER->xPos;
	if (xDiff < 0) xDiff = 0 - xDiff;
	int zDiff = zPos - PLAYER->zPos;
	if (zDiff < 0) zDiff = 0 - zDiff;
	
	return (xDiff > distance || zDiff > distance);	
}

/* 
 * Monty Mole 
 */

enum MM_STATE 
{
	MM_UNDERGROUND = 0x01, 
	MM_POPPING_UP = 0x02, 
	MM_THROWING_ROCK = 0x03, 
	MM_SEARCHING = 0x04, 
	MM_GOING_UNDERGROUND = 0x05
};

short int const OBJ_248_FREE_FOUR_BYTES_OBJECT_OFFSET = 0xA0;
short int const OBJ_248_CURRENT_STATE_OFFSET = 0x17C;

int const OBJ_248_HIDE_DISTANCE = 0x3E8000;
int const OBJ_248_BOB_THROW_STOP_DISTANCE = 0x5DC000;
int const OBJ_248_CANNON_FIRE_START_DISTANCE = 0x5DC000;

unsigned int OBJ_248_ObjectAddress = 0xFFFFFFFF;

// Get Monty Mole's address during construction
void hook_02124888_ov_50() 
{
	asm
	(
		"ldr r0, =OBJ_248_ObjectAddress		\t\n"
		"str r4, [r0]						\t\n"
	); 
	
	unsigned short int param02 = *((volatile unsigned short int*)(OBJ_248_ObjectAddress + OBJ_PARAMETER_02_OFFSET));
	
	byte cannonIndex = (byte)(param02 & 0x0F);
	*((volatile byte*)(OBJ_248_ObjectAddress + OBJ_248_FREE_FOUR_BYTES_OBJECT_OFFSET)) |= cannonIndex;
	
	// Increase draw distance 
	*((volatile unsigned int*)(OBJ_248_ObjectAddress + OBJ_DRAW_DISTANCE_OFFSET)) *= 4;
}

// Replace throwing rocks with throwing Bob-Ombs 
void repl_02123B24_ov_50() 
{
	asm
	(
		"mvn r0, #0x0000		\t\n"
		"str r0, [r13, #0x04]	\t\n"
		"mov r0, #0xCE			\t\n"
		"mov r1, #0x02			\t\n"
	);
}

// Stay above ground 1/3 
// Once underground, re-appear instantly
void hook_02123CCC_ov_50() 
{
	asm 
	(
		"ldrb r1, [r5, #0xA1]		\t\n"
		"and r1, #0x01				\t\n"
		"cmp r1, #0x01				\t\n"
		"beq undergrnd_02123CCC		\t\n"
		"cmp r0, r15				\t\n"
		"b exit_02123CCC			\t\n"
		"undergrnd_02123CCC: 		\t\n"
		"cmp r0, r0					\t\n"
		"exit_02123CCC: 			\t\n"
	);
}

// Stay above ground 2/3 
// Hide underground instantly 
unsigned int OBJ_248_STAY_ABOVE_GROUND_2_JUMP_ADDRESS = 0x02123908;
void repl_02123880_ov_50() 
{
	asm 
	(
		"ldrb r1, [r4, #0xA1]								\t\n"
		"and r1, #0x01										\t\n"
		"cmp r1, #0x01										\t\n"
		"beq exit_02123880									\t\n"
		"ldr r1, =OBJ_248_STAY_ABOVE_GROUND_2_JUMP_ADDRESS	\t\n"
		"ldr r15, [r1]										\t\n"
		"exit_02123880:										\t\n"
	);
}

// Stay above ground 3/3 
// Fix animation "flickering"
void repl_02123F4C_ov_50() { }

// Disable going underground if Mario far away but not if he's too close 
void hook_02123EF8_ov_50() 
{
	asm
	(
		"ldr r2, =OBJ_248_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	); 
	
	if (IsMarioWithinDistanceFromObjectCompletely(OBJ_248_ObjectAddress, OBJ_248_HIDE_DISTANCE)) 
	{
		asm 
		(
			"cmp r0, #0x01	\t\n" // true: go underground
		);
	} 
	else 
	{
		asm 
		(
			"cmp r0, #0xFF	\t\n" // false: above ground 
		);
	}
}

// Disable throwing Bob-Ombs if Mario too far away 
byte OBJ_248_TMP_THROW_VAR = 0x00;
void hook_02123B88_ov_50() 
{
	asm
	(
		"ldr r2, =OBJ_248_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
		"ldr r2, =OBJ_248_TMP_THROW_VAR		\t\n"
		"strb r0, [r2]						\t\n"
	); 
	
	if (!IsMarioWithinDistanceFromObjectCompletely(OBJ_248_ObjectAddress, OBJ_248_CANNON_FIRE_START_DISTANCE)) 
	{
		asm 
		(
			"mov r0, #0x01	\t\n" // can't throw
		);
	} 
	else 
	{
		asm 
		(
			"ldr r0, =OBJ_248_TMP_THROW_VAR		\t\n" // restore original check value 
			"ldrb r0, [r0]						\t\n"
		);
	}
	
	asm 
	(
		"cmp r0, #0x00	\t\n"
	);
}

// Disable original Y rotation behaviour of always facing Mario 
void repl_02123F68_ov_50() { }

// Disable the soil particle effects when hiding underground 
void repl_021238BC_ov_50() { }
void repl_021238D4_ov_50() { }

enum MM_STATE GetMoleState(unsigned int mole_address) 
{
	return (MM_STATE)(*((volatile byte*)(mole_address + OBJ_248_CURRENT_STATE_OFFSET)));
}

/* 
 * Monty Mole's Cannon
 */

short int const OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET = 0xA0;
short int const OBJ_208_Y_ROT_INCREMENT = 0x100;

unsigned int OBJ_208_ObjectAddress = 0xFFFFFFFF;

// Initialise
void hook_02112130_ov_0F() 
{
	asm
	(
		"ldr r0, =OBJ_208_ObjectAddress		\t\n"
		"str r4, [r0]						\t\n"
	); 
	
	unsigned short int param02 = *((volatile unsigned short int*)(OBJ_208_ObjectAddress + OBJ_PARAMETER_02_OFFSET));
	
	// Save the index of the mole using this cannon so that we can get his address
	byte moleIndex = (byte)(param02 & 0x0F);
	*((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET)) |= moleIndex;
	
	// Bullet Bill firing timers
	*((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x01)) = 0;
	*((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x02)) = 0;
	
	// Hit counter 
	*((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x03)) &= 0xF0;
	
	// Looped counter
	*((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x04)) = 0x00;
	
	// Increase draw distance 
	*((volatile unsigned int*)(OBJ_208_ObjectAddress + OBJ_DRAW_DISTANCE_OFFSET)) *= 4;
}

// Disable the original sliding movement
void repl_02111CEC_ov_0F() { }

short int TmpRot[3] = { 0x0000, 0x0000, 0x0000 };
int MM_SinRot = 0x00000000;
int MM_CosRot = 0x00007FFF;
short int MM_LastRotAngle = 0x0000;
void hook_02111CE8_ov_0F() 
{
	byte hitFlags = *((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x03));
	bool defeated = (hitFlags & 0x08) > 0;
	if (defeated) 
	{
		return;
	}
	
	byte moleIndex = *((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET));
	unsigned int moleAddress = 0;
	byte count = 0;
	do 
	{
		moleAddress = FindNextObjectByActorID(OBJ_248_ACTOR_ID, moleAddress);
	}
	while (count++ < moleIndex); 
	
	int xPos = *((volatile int*)(OBJ_208_ObjectAddress + OBJ_X_LOC_OFFSET));
	int yPos = *((volatile int*)(OBJ_208_ObjectAddress + OBJ_Y_LOC_OFFSET));
	int zPos = *((volatile int*)(OBJ_208_ObjectAddress + OBJ_Z_LOC_OFFSET));
	
	byte hitCount = hitFlags & 0x03;
	bool justHit = (hitFlags & 0x04) > 0;
	byte loopedCounter = *((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x04));
	if (justHit) 
	{
		ParticleEffect(0x1B, xPos, yPos, zPos);
		hitCount++;
		*((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x03)) = (0xF0 | hitCount);
	}	
	if (hitCount == 3) 
	{
		ParticleEffect(0x04, xPos, yPos + 0xC8000, zPos);
		ParticleEffect(0x0C, xPos, yPos + 0xC8000, zPos);
		ParticleEffect(0x0D, xPos, yPos + 0xC8000, zPos);
		ParticleEffect(0x10, xPos, yPos + 0xC8000, zPos);
		ParticleEffect(0x17, xPos, yPos + 0xC8000, zPos);
		ParticleEffect(0x124, xPos, yPos + 0xC8000, zPos);
		
		DestroyObject(moleAddress);
		DestroyObject(OBJ_208_ObjectAddress);
		
		TmpThreeIntArray[0] = xPos;
		TmpThreeIntArray[1] = yPos + 0xC8000;
		TmpThreeIntArray[2] = zPos;
		SpawnActor(OBJ_63_ACTOR_ID, (0x20 | MONTY_MOLE_STAR_ID), TmpThreeIntArray, 0); 
		SpawnActor(OBJ_61_ACTOR_ID, (0x40 | MONTY_MOLE_STAR_ID), TmpThreeIntArray, 0); 
		
		*((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x03)) |= 0x08;
		
		return;
	} 
	else 
	{
		if (hitCount > 0 && (byte)(loopedCounter & 0x3F) == 0x3F) 
		{
			ParticleEffect(0x18, xPos, yPos + 0x190000, zPos);
		}
		if (hitCount > 1 && (byte)(loopedCounter & 0x1F) == 0x1F)
		{
			ParticleEffect(0x15, xPos, yPos + 0x190000, zPos);
		}
	}
	*((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x04)) += 1;
	
	if (IsMarioWithinDistanceFromObjectCompletely(moleAddress, OBJ_248_HIDE_DISTANCE)) 
	{
		*((volatile byte*)(moleAddress + OBJ_248_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x01)) |= 0x01;
	}
	else 
	{
		*((volatile byte*)(moleAddress + OBJ_248_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x01)) &= 0xFE;
	}
	
	MM_STATE moleState = GetMoleState(moleAddress);
	if (!(moleState == MM_UNDERGROUND || moleState == MM_POPPING_UP)) 
	{
		TurnToFaceMario(OBJ_208_ObjectAddress, OBJ_208_Y_ROT_INCREMENT, AXIS_Y);
		*((volatile short int*)(moleAddress + OBJ_Y_ROT_OFFSET)) = 
			*((volatile short int*)(OBJ_208_ObjectAddress + OBJ_Y_ROT_OFFSET));
	}
	
	byte blastTimerBatch = *((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x01));
	byte blastTimerTriplet = *((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x02));
	
	if (blastTimerBatch == 0x7F) 
	{
		if (blastTimerTriplet % 30 == 0) 
		{
			// FIRE!
			if (IsMarioOutsideDistanceFromObjectPartially(moleAddress, OBJ_248_CANNON_FIRE_START_DISTANCE)) 
			{
				int cannonYPos = *((volatile int*)(OBJ_208_ObjectAddress + OBJ_Y_LOC_OFFSET));
				
				short int bulletBillAngle = (*((volatile short int*)(moleAddress + OBJ_Y_ROT_OFFSET)) + 0x4000) / 2;
				if (bulletBillAngle != MM_LastRotAngle)
				{
					MM_SinRot = sinLerp(bulletBillAngle);
					MM_CosRot = cosLerp(bulletBillAngle);
					MM_LastRotAngle = bulletBillAngle;
				}
				
				int bulletBillXPos = MM_CosRot * (0 - (0x2CE000 / 4096)); // 0.718
				int bulletBillYPos = cannonYPos + 0x000C8000; // 0.2
				int bulletBillZPos = MM_SinRot * (0x2CE000 / 4096); // 0.718
				
				short int param01 = 0x0000;
				TmpThreeIntArray[0] = xPos + bulletBillXPos; 
				TmpThreeIntArray[1] = bulletBillYPos;
				TmpThreeIntArray[2] = zPos + bulletBillZPos;
				TmpRot[1] = *((volatile short int*)(OBJ_208_ObjectAddress + OBJ_Y_ROT_OFFSET));
				SpawnActor(OBJ_249_ACTOR_ID, param01, TmpThreeIntArray, TmpRot);
				
				ParticleEffect(0x17, TmpThreeIntArray[0], TmpThreeIntArray[1], TmpThreeIntArray[2]);
			}
		}
		blastTimerTriplet++;
		if (blastTimerTriplet >= 90)
		{
			blastTimerTriplet = 0;
			blastTimerBatch = 0;
		}
	}
	else 
	{
		blastTimerBatch++;
	}
	
	*((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x01)) = blastTimerBatch;
	*((volatile byte*)(OBJ_208_ObjectAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x02)) = blastTimerTriplet;
}

/* 
 * Bullet Bill 
 */

short int const OBJ_249_FREE_FOUR_BYTES_1_OFFSET = 0xD8; 
short int const OBJ_249_FREE_FOUR_BYTES_2_OFFSET = 0xDC; 
short int const OBJ_249_Y_ROT_INCREMENT = 0x200;
short int const OBJ_249_COLLIDING_WITH_GROUND_OFFSET = 0x1A4;
int const OBJ_249_Y_FORWARD_SPEED = 0x20000;
int const OBJ_249_COLLISION_SPHERE_RADIUS = 0x12C000;

int const OBJ_249_VERTICAL_SPEED_INCREMENT = 0x8000;
short int const OBJ_249_X_ROT_INCREMENT = 0x100;
byte const OBJ_249_RISE_FALL_DELAY_FRAMES = 0x03;

unsigned int OBJ_249_ObjectAddress = 0xFFFFFFFF;
bool OBJ_249_ShouldExplode = false;

bool IsObject249Colliding(unsigned int object_address);
bool IsObject249HittingWallHeadOn(unsigned int object_address);
bool IsObject249HittingMario(unsigned int object_address);
void DamageCannonIfHit(unsigned int object_address);

// Increase collision sphere radius 1/2
void repl_021248F4_ov_50() 
{
	asm 
	(
		"mov r2, #0x12C000		\t\n"
	);
}

// Increase collision sphere radius 2/2
void repl_02124918_ov_50() 
{
	asm 
	(
		"mov r2, #0x12C000		\t\n"
	);
}

__attribute__((noinline)) bool IsObject249Colliding(unsigned int object_address) 
{
	byte collidingWithSolidObject1A4 = *((volatile byte*)(object_address + 0x1A4));
	bool collidingWithSolidObjectWallOrMario = (collidingWithSolidObject1A4 != 0 || 
			IsObject249HittingMario(object_address) || 
			IsObject249HittingWallHeadOn(object_address));
	if (collidingWithSolidObjectWallOrMario) 
	{
		return true;
	}
	
	bool hitByOtherBulletBill = ((*((volatile byte*)(OBJ_249_ObjectAddress + OBJ_249_FREE_FOUR_BYTES_2_OFFSET)) & 0x02) > 0x00);
	if (hitByOtherBulletBill) 
	{
		return true;
	}
	
	unsigned int bulletBillAddress = object_address;
	int xPos = *((volatile int*)(object_address + OBJ_X_LOC_OFFSET));
	int yPos = *((volatile int*)(object_address + OBJ_Y_LOC_OFFSET));
	int zPos = *((volatile int*)(object_address + OBJ_Z_LOC_OFFSET));
	while ((bulletBillAddress = FindNextObjectByActorID(OBJ_249_ACTOR_ID, bulletBillAddress)) != 0) 
	{
		if (bulletBillAddress == object_address) 
		{
			continue;
		}
		
		int otherXPos = *((volatile int*)(bulletBillAddress + OBJ_X_LOC_OFFSET));
		int otherYPos = *((volatile int*)(bulletBillAddress + OBJ_Y_LOC_OFFSET));
		int otherZPos = *((volatile int*)(bulletBillAddress + OBJ_Z_LOC_OFFSET));
		
		if (abs32(yPos - otherYPos) >= 0xC8000 || 
			abs32(xPos - otherXPos) >= 0x12C000 || 
			abs32(zPos - otherZPos) >= 0x12C000) 
		{
			continue;
		}
		else 
		{
			*((volatile byte*)(bulletBillAddress + OBJ_249_FREE_FOUR_BYTES_2_OFFSET)) |= 0x02;
			return true;
		}
	}
	return false;
}

bool IsObject249HittingWallHeadOn(unsigned int object_address) 
{
	return (*((volatile unsigned int*)(OBJ_249_ObjectAddress + 0xE8)) > 0x00);
}

bool IsObject249HittingMario(unsigned int object_address) 
{
	return (*((volatile unsigned int*)(object_address + 0x184)) != 0);
}

void DamageCannonIfHit(unsigned int object_address) 
{
	int xPos = *((volatile int*)(object_address + OBJ_X_LOC_OFFSET));
	int yPos = *((volatile int*)(object_address + OBJ_Y_LOC_OFFSET));
	int zPos = *((volatile int*)(object_address + OBJ_Z_LOC_OFFSET));
	unsigned int cannonAddress = 0;
	while ((cannonAddress = FindNextObjectByActorID(OBJ_208_ACTOR_ID, cannonAddress)) != 0) 
	{		
		int cannonXPos = *((volatile int*)(cannonAddress + OBJ_X_LOC_OFFSET));
		int cannonYPos = *((volatile int*)(cannonAddress + OBJ_Y_LOC_OFFSET));
		int cannonZPos = *((volatile int*)(cannonAddress + OBJ_Z_LOC_OFFSET));
		
		if (yPos - cannonYPos >= 0x17C000 || 
			abs32(xPos - cannonXPos) >= 0x352000 || 
			abs32(zPos - cannonZPos) >= 0x352000) 
		{
			continue;
		}
		else 
		{
			*((volatile byte*)(cannonAddress + OBJ_208_FREE_FOUR_BYTES_OBJECT_OFFSET + 0x03)) |= 0x04;
			return;
		}
	}
}

void hook_02124744_ov_50() 
{
	asm
	(
		"ldr r1, =OBJ_249_ObjectAddress		\t\n"
		"str r4, [r1]						\t\n"
	); 
	
	*((volatile int*)(OBJ_249_ObjectAddress + OBJ_FORWARD_SPEED_OFFSET)) = OBJ_249_Y_FORWARD_SPEED;
	
	unsigned short int frameCounter = *((volatile unsigned short int*)(OBJ_249_ObjectAddress + (OBJ_249_FREE_FOUR_BYTES_2_OFFSET + 0x02)));
	int ySpeed = 0;
	short int xRot = *((volatile short int*)(OBJ_249_ObjectAddress + OBJ_X_ROT_OFFSET));
	if (frameCounter >= 30) 
	{
		TurnToFaceMario(OBJ_249_ObjectAddress, OBJ_249_Y_ROT_INCREMENT, AXIS_Y); 
		
		bool yPosUpdated = false;
		int yPos = *((volatile int*)(OBJ_249_ObjectAddress + OBJ_Y_LOC_OFFSET));
		int targetYPos = PLAYER->yPos + 0x64000;
		int diff = abs32(yPos - targetYPos);
		if (diff > 0x19000) // 0.025
		{
			byte riseFallDelayCounter = *((volatile byte*)(OBJ_249_ObjectAddress + (OBJ_249_FREE_FOUR_BYTES_2_OFFSET + 0x01)));
			byte riseDelay = ((riseFallDelayCounter & 0xF0) >> 4);
			byte fallDelay = (riseFallDelayCounter & 0x0F);
			
			if ((yPos + OBJ_249_VERTICAL_SPEED_INCREMENT) < targetYPos) 
			{
				riseDelay++;
				fallDelay = 0;
				if (riseDelay >= OBJ_249_RISE_FALL_DELAY_FRAMES) 
				{
					// move up
					if (xRot - OBJ_249_X_ROT_INCREMENT > 0 - 0x600) 
					{
						xRot -= OBJ_249_X_ROT_INCREMENT;
					}
					ySpeed = OBJ_249_VERTICAL_SPEED_INCREMENT;
					riseDelay = OBJ_249_RISE_FALL_DELAY_FRAMES;
					yPosUpdated = true;
				}
			}
			else if ((yPos - OBJ_249_VERTICAL_SPEED_INCREMENT) > targetYPos) 
			{
				fallDelay++;
				riseDelay = 0;
				if (fallDelay >= OBJ_249_RISE_FALL_DELAY_FRAMES) 
				{
					// move down
					if (xRot + OBJ_249_X_ROT_INCREMENT < 0x600) 
					{
						xRot += OBJ_249_X_ROT_INCREMENT;
					}
					ySpeed = 0 - OBJ_249_VERTICAL_SPEED_INCREMENT;
					fallDelay = OBJ_249_RISE_FALL_DELAY_FRAMES;
					yPosUpdated = true;
				}
			}
			
			*((volatile byte*)(OBJ_249_ObjectAddress + (OBJ_249_FREE_FOUR_BYTES_2_OFFSET + 0x01))) = 
				(byte)((riseDelay << 4) | (fallDelay & 0x0F));
		}
		
		if (!yPosUpdated) 
		{
			if (xRot > 0) 
			{
				xRot -= OBJ_249_X_ROT_INCREMENT;
			} 
			else if (xRot < 0) 
			{
				xRot += OBJ_249_X_ROT_INCREMENT;
			}
		}
	}
	
	OBJ_UpdateObjectXYZSpeedBasedOnForwardSpeedAndDirection(OBJ_249_ObjectAddress);
	
	*((volatile int*)(OBJ_249_ObjectAddress + OBJ_Y_SPEED_OFFSET)) = ySpeed;
	*((volatile short int*)(OBJ_249_ObjectAddress + OBJ_X_ROT_OFFSET)) = xRot;
	
	UpdatePositionFromSpeed(OBJ_249_ObjectAddress);
	
	*((volatile unsigned short int*)(OBJ_249_ObjectAddress + (OBJ_249_FREE_FOUR_BYTES_2_OFFSET + 0x02))) += 0x01;
	
	TmpThreeIntArray[0] = *((volatile int*)(OBJ_249_ObjectAddress + OBJ_X_LOC_OFFSET));
	TmpThreeIntArray[1] = *((volatile int*)(OBJ_249_ObjectAddress + OBJ_Y_LOC_OFFSET));
	TmpThreeIntArray[2] = *((volatile int*)(OBJ_249_ObjectAddress + OBJ_Z_LOC_OFFSET));
}

// Replace the original call to OBJ_UpdateObjectModelRotationY (0x0203BD6C) 
// with a call to OBJ_UpdateObjectModelRotation (0x0203BE9C)
void repl_02124428_ov_50() 
{
	//UpdateObjectModelPositionAndRotation(OBJ_249_ObjectAddress, 0x12C, 0x150);
	asm
	(
		"push {r14}							\t\n"
		"ldrsh r1, [r4, #0x8C]				\t\n"
		"ldrsh r2, [r4, #0x8E]				\t\n"
		"ldrsh r3, [r4, #0x90]				\t\n"
		"bl OBJ_UpdateObjectModelRotation	\t\n"
		"pop {r14}							\t\n"
	);
}

// Remove the original movement calculation and updating 
void repl_02124748_ov_50() { } 

// Undo effects of "bouncing" back from hitting a wall head-on 
void hook_0212475C_ov_50() 
{
	bool hasBeenNotColliding = 
		(*((volatile byte*)(OBJ_249_ObjectAddress + OBJ_249_FREE_FOUR_BYTES_2_OFFSET)) & 0x01) > 0x00;
	if (IsObject249HittingWallHeadOn(OBJ_249_ObjectAddress)) 
	{
		if (hasBeenNotColliding) 
		{
			*((volatile int*)(OBJ_249_ObjectAddress + OBJ_X_LOC_OFFSET)) = TmpThreeIntArray[0];
			*((volatile int*)(OBJ_249_ObjectAddress + OBJ_Y_LOC_OFFSET)) = TmpThreeIntArray[1];
			*((volatile int*)(OBJ_249_ObjectAddress + OBJ_Z_LOC_OFFSET)) = TmpThreeIntArray[2];
			
			*((volatile byte*)(OBJ_249_ObjectAddress + OBJ_249_COLLIDING_WITH_GROUND_OFFSET)) = 
				0x01;
		}
	} 
	else if (!hasBeenNotColliding)
	{
		*((volatile byte*)(OBJ_249_ObjectAddress + OBJ_249_FREE_FOUR_BYTES_2_OFFSET)) |= 0x01;
	}
}

// If the rock is hitting the ground, is "stuck" colliding 
// head on with a wall having been fired from the cannon, 
// is hitting or has been hit by another rock or it's too "old"
// then explode
void hook_02124668_ov_50() 
{
	asm
	(
		"ldr r1, =OBJ_249_ObjectAddress		\t\n"
		"str r4, [r1]						\t\n"
	); 
	
	bool isColliding = IsObject249Colliding(OBJ_249_ObjectAddress);
	bool timeUp = (*((volatile unsigned short int*)(OBJ_249_ObjectAddress + (OBJ_249_FREE_FOUR_BYTES_2_OFFSET + 0x02))) >= 0x200);
	OBJ_249_ShouldExplode = (isColliding | timeUp);
	if (OBJ_249_ShouldExplode) 
	{
		DamageCannonIfHit(OBJ_249_ObjectAddress);
	}
	
	asm 
	(
		"ldr r1, =OBJ_249_ShouldExplode	\t\n"
		"ldrb r1, [r1]					\t\n"
		
		"checkExplode_02124668: 		\t\n"
		"cmp r1, #0x00					\t\n"
	);
}

// Increase the damage dealt upon hitting Mario 
void repl_0212462C_ov_50() 
{
	asm 
	(
		"mov r2, #0x03		\t\n"
	);
}

// Disable the spawning of additional smaller rocks upon hitting the ground 
void repl_02124674_ov_50() 
{
	asm 
	(
		"cmp r0, r14 		\t\n"
	);
}

// Add smoke particle effects when moving 
void hook_0212474C_ov_50() 
{
	ComplexParticleEffectForObject(OBJ_249_ObjectAddress, 0x90, OBJ_249_FREE_FOUR_BYTES_1_OFFSET);
}

// Add explosion effects upon collision 
void hook_02124738_ov_50() 
{
	ParticleEffect(0xB1, 
		*((volatile int*)(OBJ_249_ObjectAddress + OBJ_X_LOC_OFFSET)), 
		*((volatile int*)(OBJ_249_ObjectAddress + OBJ_Y_LOC_OFFSET)), 
		*((volatile int*)(OBJ_249_ObjectAddress + OBJ_Z_LOC_OFFSET)));
	ParticleEffect(0x8F, 
		*((volatile int*)(OBJ_249_ObjectAddress + OBJ_X_LOC_OFFSET)), 
		*((volatile int*)(OBJ_249_ObjectAddress + OBJ_Y_LOC_OFFSET)), 
		*((volatile int*)(OBJ_249_ObjectAddress + OBJ_Z_LOC_OFFSET)));
}


