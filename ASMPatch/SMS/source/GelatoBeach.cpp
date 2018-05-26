/* Constants for the Gelato Beach level
 */

#include "GelatoBeach.h"
#include "SharedSecretLevel.h"

byte GB_LevelID = LEVEL_WetDryWorld;

byte GB_AlignMirrorsMissionStarID = 1;
byte GB_AlignMirrorsMissionLevelID = LEVEL_WetDryWorld;
int GB_AlignedMirrorsStarSpawnStartPos[3] = { (int)0xFEB4C000, (int)0x00CB2000, (int)0xFEB4C000}; // -5.3, 3.25, -5.3

byte GB_SandBirdMissionStarID = 6;
byte GB_SandBirdMissionLevelID = LEVEL_WetDryWorld;

byte GB_WatermelonsMissionStarID = 7;
byte GB_NumWatermelons = 3;
unsigned int GB_Watermelons[3] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
unsigned int GB_WatermelonGoal = 0xFFFFFFFF;
unsigned int GB_WatermelonCurrentEntry = 0xFFFFFFFF;
bool GB_CompetitionWon = false;

byte GB_WigglerMissionStarID = 2;
byte GB_WigglerMissionObjectBank7Value = 27;
byte GB_OriginalObjectBank7Value = 14;

LevelSettings GB_SecretLevel_Settings = 
{
	{ 0, 0, 5, 1, 4, 0, 0, 8 }, 
	1958, 
	1956, 
	1959, 
	1960, 
	false, 
	1600, 
	8, 
	{ 3, 32, 57 }, 
	10
};
byte GB_SecretLevel_StarID = 0x05;

// If it's the Wiggler mission, change the value of Object Bank 7 to 
// allow the Wiggler object
byte GB_RequiredObjectBank07 = 0x00;
void hook_0214EAAC_ov_7C()
{
	if (STAR_ID == GB_WigglerMissionStarID)
	{
		GB_RequiredObjectBank07 = GB_WigglerMissionObjectBank7Value;
	}
	else
	{
		GB_RequiredObjectBank07 = GB_OriginalObjectBank7Value;
	}
	
	*((volatile byte*)(LEVEL_OVERLAY_LOAD_ADDRESS + OBJECT_BANK_7_OFFSET)) = 
		GB_RequiredObjectBank07;
	
	*((volatile byte*)(ACT_SELECTOR_ID_TABLE_ADDRESS + SSL_LevelID)) = 
		GB_SecretLevel_Settings.ActSelectorID;
	
	asm
	(
		"pop {r0}							\t\n"
		"ldr r0, =GB_RequiredObjectBank07	\t\n"
		"ldrb r0, [r0]						\t\n"
		"push {r0}							\t\n"
		
		"cmp r0, #0x00						\t\n" // Original instruction
	);
}

void GB_LoadSecretLevel()
{
	/*int positions[3] = { 0, 0, 0 };
	short int rotations[3] = { 0, 0, 0 };
	
	SpawnActor(16, 0x0000, positions, rotations);*/
}
