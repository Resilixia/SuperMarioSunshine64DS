#include "RiccoHarbourSewers.h"

const int OBJ_084_MAX_SPEED_UP_NORMAL = 0x40000;
const int OBJ_084_MAX_SPEED_UP_TURBO = 0x4A000;
const int OBJ_084_MAX_SPEED_NO_UP_NORMAL = 0x18000;
const int OBJ_084_MAX_SPEED_NO_UP_TURBO = 0x34000;
const short int OBJ_084_ACCELERATION_NORMAL = 0x1100;
const short int OBJ_084_ACCELERATION_TURBO = 0x2800;
const short int OBJ_084_START_SPEED_NORMAL = 0x600;
const short int OBJ_084_START_SPEED_TURBO = 0x2800;

int OBJ_084_MaxSpeedUp = OBJ_084_MAX_SPEED_UP_TURBO;
int OBJ_084_MaxSpeedNoUp = OBJ_084_MAX_SPEED_NO_UP_TURBO;
short int OBJ_084_Acceleration = OBJ_084_ACCELERATION_TURBO;
short int OBJ_084_StartSpeed = OBJ_084_START_SPEED_TURBO;

// Increase maximum speed with 'Up' pressed
void repl_020CC3BC()
{
	asm
	(
		"ldr r0, =OBJ_084_MaxSpeedUp	\t\n"
		"ldrne r6, [r0]					\r\n"
	);
}

// Increase maximum speed without 'Up' pressed
void repl_020CC3C8()
{
	asm
	(
		"ldr r7, =OBJ_084_MaxSpeedNoUp	\t\n"
		"ldreq r6, [r7]					\t\n"
	);
}

// Increase acceleration
void repl_020CC3F0()
{
	asm
	(
		"ldr r2, =OBJ_084_Acceleration	\t\n"
		"ldrh r2, [r2]					\t\n"
	);
}

// Increase starting speed
void repl_020CC3F8()
{
	asm
	(
		"ldr r1, =OBJ_084_StartSpeed	\t\n"
		"ldrgth r2, [r1]				\t\n"
	);
}

// Re-enter level if crashed during race
void hook_0214D064()
{
	if (STR_RaceStarted && !STR_RaceFinished)
	{
		LoadLevel(RHS_LevelID, 0, 0xFFFFFFFF, 0);
	}
	else if (!STR_RaceStarted)
	{
		RHS_KoopaShellDestroyed = true;
	}
}