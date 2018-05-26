#include "RiccoHarbourSewers.h"

short int const OBJ_311_COLLISION_SPHERE_SIZE_OFFSET = 0x114;

unsigned int OBJ_311_ObjectAddress = 0xFFFFFFFF;
unsigned int OBJ_311_BMDAddress = 0xFFFFFFFF;

// Parameters: 
// 	Parameter 01: 
// 		byte 1: mode (0 - 2 original):
// 					  0 = no movement, temporary, nothing when passed through, 
// 					  1 = no movement, temporary, shows number collected when passed through, 
// 					  2 = rises up, temporary, shows number collected when passed through, 
// 					  3 = normal marker
// 					  4 = start marker
// 					  5 = end marker 
// 					  6 = Koopa shell spawner

enum OBJ_311_MarkerType
{
	OBJ_311_MRKTYP_NO_ACTION = 0, 
	OBJ_311_MRKTYP_STILL_AND_COUNT = 1, 
	OBJ_311_MRKTYP_RISE_AND_COUNT = 2, 
	OBJ_311_MRKTYP_RACE_NORMAL = 3, 
	OBJ_311_MRKTYP_RACE_START = 4, 
	OBJ_311_MRKTYP_RACE_END = 5, 
	OBJ_311_MRKTYP_RACE_KOOPA_SHELL_SPAWN = 6 
};

const unsigned short int OBJ_311_STARTING_LIFESPAN_FRAMES = 0xC8;
const unsigned short int OBJ_311_LIFESPAN_OFFSET = 0x100;

const int OBJ_311_COLLISION_SPHERE_X_SIZE = 0x00088000;
const int OBJ_311_COLLISION_SPHERE_Y_SIZE = 0x000E8000;
const int OBJ_311_COLLISION_SPHERE_Z_SIZE = 0x00000000;

// Constructor
void hook_0211A0A4_ov_40()
{
	asm
	(
		"ldr r2, =OBJ_311_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	);
	
	short int param01 = *((volatile short int*)(OBJ_311_ObjectAddress + OBJ_PARAMETER_01_OFFSET));
	
	OBJ_311_MarkerType markerType = OBJ_311_MarkerType((byte)(param01 & 0x00FF));
	
	if (markerType < OBJ_311_MRKTYP_RACE_NORMAL)
	{
		return;
	}
	else
	{
		if (markerType < OBJ_311_MRKTYP_RACE_KOOPA_SHELL_SPAWN)
		{
			RHS_TotalNumRaceMarkers++;
		}
		
		*((volatile unsigned short int*)(OBJ_311_ObjectAddress + OBJ_311_LIFESPAN_OFFSET)) = OBJ_311_STARTING_LIFESPAN_FRAMES;
	}
	
	*((volatile unsigned int*)(OBJ_311_ObjectAddress + OBJ_DRAW_DISTANCE_OFFSET)) *= 4;
}

// Make model invisible if Koopa shell spawner
void hook_0211A0B0_ov_40()
{
	asm
	(
		"ldr r2, =OBJ_311_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
		"ldr r2, =OBJ_311_BMDAddress		\t\n"
		"str r1, [r2]						\t\n"
	);
	
	short int param01 = *((volatile short int*)(OBJ_311_ObjectAddress + OBJ_PARAMETER_01_OFFSET));
	
	OBJ_311_MarkerType markerType = OBJ_311_MarkerType((byte)(param01 & 0x00FF));
	
	if (markerType == OBJ_311_MRKTYP_RACE_KOOPA_SHELL_SPAWN || markerType == OBJ_311_MRKTYP_RACE_END)
	{
		unsigned int boneSectionAddress = 
			*((volatile unsigned int*)(OBJ_311_BMDAddress + 0x08));
			
		int scale = (markerType == OBJ_311_MRKTYP_RACE_KOOPA_SHELL_SPAWN) ? 0 : 2;
		scale = scale << 12;
		
		*((volatile int*)(boneSectionAddress + 0x10)) = scale;
		*((volatile int*)(boneSectionAddress + 0x14)) = scale;
		*((volatile int*)(boneSectionAddress + 0x18)) = scale;
	}
}

// If Koopa shell spawner, disable collision
void hook_0211A178_ov_40()
{
	asm
	(
		"ldr r2, =OBJ_311_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	);
	
	short int param01 = *((volatile short int*)(OBJ_311_ObjectAddress + OBJ_PARAMETER_01_OFFSET));
	
	OBJ_311_MarkerType markerType = OBJ_311_MarkerType((byte)(param01 & 0x00FF));
	
	if (markerType >= OBJ_311_MRKTYP_RACE_NORMAL && !STR_RaceStarted)
	{
		*((volatile int*)(OBJ_311_ObjectAddress + OBJ_311_COLLISION_SPHERE_SIZE_OFFSET + 0x00)) = 0;
		*((volatile int*)(OBJ_311_ObjectAddress + OBJ_311_COLLISION_SPHERE_SIZE_OFFSET + 0x04)) = 0;
		*((volatile int*)(OBJ_311_ObjectAddress + OBJ_311_COLLISION_SPHERE_SIZE_OFFSET + 0x08)) = 0;
	}
}

// If it's a race marker, don't decrement the lifespan
void hook_0211A00C_ov_40()
{
	asm
	(
		"ldr r2, =OBJ_311_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	);
	
	short int param01 = *((volatile short int*)(OBJ_311_ObjectAddress + OBJ_PARAMETER_01_OFFSET));
	short int param02 = *((volatile short int*)(OBJ_311_ObjectAddress + OBJ_PARAMETER_02_OFFSET));
	short int param03 = *((volatile short int*)(OBJ_311_ObjectAddress + OBJ_PARAMETER_03_OFFSET));
	
	OBJ_311_MarkerType markerType = OBJ_311_MarkerType((byte)(param01 & 0x00FF));
	
	if (markerType < OBJ_311_MRKTYP_RACE_NORMAL)
	{
		return;
	}
	else
	{
		*((volatile unsigned short int*)(OBJ_311_ObjectAddress + OBJ_311_LIFESPAN_OFFSET)) = OBJ_311_STARTING_LIFESPAN_FRAMES;
		
		int xPos = *((volatile int*)(OBJ_311_ObjectAddress + OBJ_X_LOC_OFFSET));
		int yPos = *((volatile int*)(OBJ_311_ObjectAddress + OBJ_Y_LOC_OFFSET));
		int zPos = *((volatile int*)(OBJ_311_ObjectAddress + OBJ_Z_LOC_OFFSET));
		
		if (markerType == OBJ_311_MRKTYP_RACE_START && !STR_RaceStarted)
		{
			byte xRange = (byte)(param02 & 0x00FF);
			byte yRange = (byte)((param02 & 0xFF00) >> 8);
			byte zRange = (byte)(param03 & 0x00FF);
			
			int xDiff = PLAYER->xPos - xPos;
			if (xDiff < 0) xDiff = 0 - xDiff;
			int yDiff = PLAYER->yPos - yPos;
			if (yDiff < 0) yDiff = 0 - yDiff;
			int zDiff = PLAYER->zPos - zPos;
			if (zDiff < 0) zDiff = 0 - zDiff;
			
			if (xDiff <= (int)(xRange * 100 * 4096) && yDiff <= (int)(yRange * 100 * 4096) && zDiff <= (int)(zRange * 100 * 4096))
			{
				STR_ShouldOverrideBehaviour = true;
				STR_OverridePlayerPlus664Value = STR_START_TIMER_BEHAVIOUR;
			}
		}
		else if (markerType == OBJ_311_MRKTYP_RACE_KOOPA_SHELL_SPAWN)
		{
			if (RHS_KoopaShellDestroyed)
			{
				TmpThreeIntArray[0] = xPos;
				TmpThreeIntArray[1] = yPos;
				TmpThreeIntArray[2] = zPos;
				SpawnActor(285, 0x0000, TmpThreeIntArray, 0);
				
				RHS_KoopaShellDestroyed = false;
			}
		}
		else if (STR_RaceStarted && markerType >= OBJ_311_MRKTYP_RACE_NORMAL && markerType <= OBJ_311_MRKTYP_RACE_END)
		{
			*((volatile int*)(OBJ_311_ObjectAddress + OBJ_311_COLLISION_SPHERE_SIZE_OFFSET + 0x00)) = OBJ_311_COLLISION_SPHERE_X_SIZE;
			*((volatile int*)(OBJ_311_ObjectAddress + OBJ_311_COLLISION_SPHERE_SIZE_OFFSET + 0x04)) = OBJ_311_COLLISION_SPHERE_Y_SIZE;
			*((volatile int*)(OBJ_311_ObjectAddress + OBJ_311_COLLISION_SPHERE_SIZE_OFFSET + 0x08)) = OBJ_311_COLLISION_SPHERE_Z_SIZE;
			
			if (markerType == OBJ_311_MRKTYP_RACE_END)
			{
				*((volatile int*)(OBJ_311_ObjectAddress + OBJ_311_COLLISION_SPHERE_SIZE_OFFSET + 0x00)) *= 2;
				*((volatile int*)(OBJ_311_ObjectAddress + OBJ_311_COLLISION_SPHERE_SIZE_OFFSET + 0x04)) *= 2;
				*((volatile int*)(OBJ_311_ObjectAddress + OBJ_311_COLLISION_SPHERE_SIZE_OFFSET + 0x08)) *= 2;
			}
		}
	}
}

// Increment number of markers passed through when ring passed through
void hook_02119CFC_ov_40()
{
	asm
	(
		"ldr r2, =OBJ_311_ObjectAddress		\t\n"
		"str r5, [r2]						\t\n"
	);
	
	short int param01 = *((volatile short int*)(OBJ_311_ObjectAddress + OBJ_PARAMETER_01_OFFSET));
	
	OBJ_311_MarkerType markerType = OBJ_311_MarkerType((byte)(param01 & 0x00FF));
	
	if (markerType < OBJ_311_MRKTYP_RACE_NORMAL)
	{
		return;
	}
	else
	{
		if (markerType < OBJ_311_MRKTYP_RACE_KOOPA_SHELL_SPAWN)
		{
			RHS_NumRaceMarkersPassed++;
		}
		
		if (markerType == OBJ_311_MRKTYP_RACE_END)
		{
			RHS_EndRace();
		}
	}
}
