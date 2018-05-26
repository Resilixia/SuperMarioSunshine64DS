#include "GelatoBeach.h"
#include "RiccoHarbourSewers.h"

short int const OBJ_020_FREE_FOUR_BYTES_OFFSET = 0xA0;

unsigned int OBJ_020_ObjectAddress = 0xFFFFFFFF;

unsigned int OBJ_020_BMDAddress = 0xFFFFFFFF;
unsigned int OBJ_020_BMDAddressOld = 0xFFFFFFFF;
unsigned int OBJ_020_BMDBoneSectionOriginalOffset = 0xFFFFFFFF;

const unsigned short int OBJ_020_NUM_BONES_PER_ENEMY = 6;
const unsigned int BMD_NUM_BYTES_IN_BONE_HEADER = 64;
const unsigned int BMD_NUM_BYTES_IN_MATERIAL_HEADER = 48;

const unsigned int OBJ_020_NOKI_SHELL_MATERIAL_ID = 9;
const unsigned int OBJ_020_NOKI_HAIR_MATERIAL_ID = 3;
const unsigned int OBJ_020_PIANTA_LEAVES_MATERIAL_ID = 21;
const unsigned int OBJ_020_PIANTA_SKIN_MATERIAL_ID = 16;
const unsigned int OBJ_020_PIANTA_BRA_MATERIAL_ID = 24;
const unsigned int OBJ_020_PIANTA_FLOWER_MATERIAL_ID = 26;

const unsigned int OBJ_020_NOKI_SHELL_TEXTURE_ID_START = 8;
const unsigned int OBJ_020_NOKI_HAIR_TEXTURE_ID_START = 2;
const unsigned int OBJ_020_PIANTA_LEAVES_TEXTURE_ID_START = 20;
const unsigned int OBJ_020_PIANTA_SKIN_TEXTURE_ID_START = 15;
const unsigned int OBJ_020_PIANTA_BRA_TEXTURE_ID_START = 22;
const unsigned int OBJ_020_PIANTA_FLOWER_TEXTURE_ID_START = 24;

const unsigned int OBJ_020_NOKI_MARIO_HEAD_TALK_Y_OFFSET = 0x050000;
const unsigned int OBJ_020_PIANTA_MARIO_HEAD_TALK_Y_OFFSET = 0x0C0000;

const short int OBJ_020_SHADOW_RADIUS_X_OFFSET = 0x19C;
const short int OBJ_020_SHADOW_RADIUS_Y_OFFSET = 0x1AC;
const short int OBJ_020_SHADOW_RADIUS_Z_OFFSET = 0x1BC;

const int OBJ_020_PIANTA_SHADOW_RADIUS = 0x2800;

const short int OBJ_020_COLLISION_HORIZONTAL_RADIUS_OFFSET = 0xD8;
const short int OBJ_020_COLLISION_VERTICAL_RADIUS_OFFSET = 0xDC;

const int OBJ_020_PIANTA_COLLISION_RADIUS = 0x0C0000;

const short int OBJ_061_ACTOR_ID = 0xB2;

// Parameter 02: 
// 		byte 1: Special mission ID, 0x00 for none
// 		byte 2: 
// 				bits 1 - 4: 1 = Change message depending on character, 0 = same for all

// Parameter 03:
// Shared:
// 		byte 1: 
//				bits 1 - 4: 0 = Noki, 1 = Pianta
// 				bits 5 - 8: 0 = Male, 1 = Female
// Noki:
// 		byte 2: 	
// 				bits 1 - 4: Shell colour
// 				bits 5 - 8: Hair colour
// Pianta: 
// 		byte 2: 
// 				bits 1 - 4: Skin colour

// OBJ_020_FREE_FOUR_BYTES_OFFSET: 
// Shared:
// 		bytes 1 - 2: parameter 2 value
// Special Mission: GB_WATERMELONS: 
// 		byte 1: 
// 				bit 1: bool, has handed out prize 
// Special Mission: RH_ACTIVATE_CRANES: 
// 		byte 1: 
// 				bit 1: bool, has handed out Star 

enum OBJ_020_EnemyType
{
	OBJ_020_ENM_NOKI = 0, 
	OBJ_020_ENM_PIANTA = 1
};

enum OBJ_020_Sex
{
	OBJ_020_SEX_MALE = 0,
	OBJ_020_SEX_FEMALE = 1
};

enum OBJ_020_NokiHairColour
{
	OBJ_020_NHAIR_GREEN = 0, 
	OBJ_020_NHAIR_BLUE = 1, 
	OBJ_020_NHAIR_RED = 2, 
	OBJ_020_NHAIR_PURPLE = 3, 
	OBJ_020_NHAIR_PINK = 4, 
	OBJ_020_NHAIR_ORANGE = 5
};

enum OBJ_020_NokiShellColour
{
	OBJ_020_NSHELL_GREEN = 0, 
	OBJ_020_NSHELL_BLUE = 1, 
	OBJ_020_NSHELL_RED = 2, 
	OBJ_020_NSHELL_PURPLE = 3, 
	OBJ_020_NSHELL_PINK = 4, 
	OBJ_020_NSHELL_ORANGE = 5 
};

enum OBJ_020_PiantaSkinColour
{
	OBJ_020_PSKIN_GREEN = 0, 
	OBJ_020_PSKIN_BLUE = 1, 
	OBJ_020_PSKIN_ORANGE = 2, 
	OBJ_020_PSKIN_PINK = 3, 
	OBJ_020_PSKIN_YELLOW = 4 
};

enum OBJ_020_SpecialMissionID
{
	OBJ_020_SPCMSN_NONE = 0, 
	OBJ_020_SPCMSN_GB_WATERMELONS = 1, 
	OBJ_020_SPCMSN_RH_ACTIVATE_CRANES = 2, 
	OBJ_020_SPCMSN_RHS_SEWER_RACE = 3
};

unsigned short int GetMessageIndex_SPCMSN_NONE();
unsigned short int GetMessageIndex_SPCMSN_GB_WATERMELONS();
unsigned short int GetMessageIndex_SPCMSN_RH_ACTIVATE_CRANES();
unsigned short int GetMessageIndex_SPCMSN_RHS_SEWER_RACE();

void FinishAction_SPCMSN_NONE();
void FinishAction_SPCMSN_GB_WATERMELONS();
void FinishAction_SPCMSN_RH_ACTIVATE_CRANES();
void FinishAction_SPCMSN_RHS_SEWER_RACE();

void OBJ_020_SpawnStar(byte starID);

const short int TEXT_FIRST_RED_BOMBHEI_MESSAGE_INDEX = 0x0083;

void hook_0212D048_ov_54()
{
	asm
	(
		"ldr r2, =OBJ_020_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
		"ldr r2, =OBJ_020_BMDAddress		\t\n"
		"str r1, [r2]						\t\n"
	);
	
	//unsigned short int param01 = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_PARAMETER_01_OFFSET));
	unsigned short int param02 = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_PARAMETER_02_OFFSET));
	unsigned short int param03 = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_PARAMETER_03_OFFSET));
	
	OBJ_020_EnemyType type = OBJ_020_EnemyType((byte)(param03 & 0x000F));
	OBJ_020_Sex sex = OBJ_020_Sex((byte)((param03 & 0x00F0) >> 4));
	
	if (OBJ_020_BMDAddress != OBJ_020_BMDAddressOld)
	{
		OBJ_020_BMDBoneSectionOriginalOffset = 
			*((volatile unsigned int*)(OBJ_020_BMDAddress + 0x08));
	}
	{
		// Modify the Transform/Bone map so that Pianta's entries 
		// point to Bone ID's in range 0 - 5 as it would for the 
		// first 6 entries (as with Noki's)
		unsigned int nBones = 
			*((volatile unsigned int*)(OBJ_020_BMDAddress + 0x04));
		unsigned int offTransformBoneMap = 
			*((volatile unsigned int*)(OBJ_020_BMDAddress + 0x2C));
		
		for (unsigned int i = 0; i < nBones; i++)
		{
			unsigned short int entry = 
				*((volatile unsigned short int*)(offTransformBoneMap + (i * 2)));
			if (entry >= OBJ_020_NUM_BONES_PER_ENEMY)
			{
				*((volatile unsigned short int*)(offTransformBoneMap + (i * 2))) = 
					(unsigned short int)(entry - OBJ_020_NUM_BONES_PER_ENEMY);
			}
		}
		
		// Modify the Pianta bone ID's to be in the range 0 - 5
		for (unsigned int i = OBJ_020_NUM_BONES_PER_ENEMY; i < nBones; i++)
		{
			unsigned int boneID = 
				*((volatile unsigned int*)(OBJ_020_BMDBoneSectionOriginalOffset + 
				(i * BMD_NUM_BYTES_IN_BONE_HEADER)));
			*((volatile unsigned int*)(OBJ_020_BMDBoneSectionOriginalOffset + 
				(i * BMD_NUM_BYTES_IN_BONE_HEADER))) = 
				(unsigned int)(boneID - OBJ_020_NUM_BONES_PER_ENEMY);
		}
		
		// Set nBones to 6
		*((volatile unsigned int*)(OBJ_020_BMDAddress + 0x04)) = 
			(unsigned int)OBJ_020_NUM_BONES_PER_ENEMY;
	}
	
	unsigned int offMaterials = 
		*((volatile unsigned int*)(OBJ_020_BMDAddress + 0x28));
	
	if (type == OBJ_020_ENM_NOKI)
	{
		// Set first bone to Noki root bone, Pianta structure will 
		// be ignored
		*((volatile unsigned int*)(OBJ_020_BMDAddress + 0x08)) = 
			OBJ_020_BMDBoneSectionOriginalOffset;
		
		OBJ_020_NokiShellColour shellColour = OBJ_020_NokiShellColour((byte)((param03 & 0x0F00) >> 8));
		OBJ_020_NokiHairColour hairColour = OBJ_020_NokiHairColour((byte)((param03 & 0xF000) >> 12));
		
		unsigned int shellTex = OBJ_020_NOKI_SHELL_TEXTURE_ID_START + (byte)(shellColour);
		unsigned int hairTex = OBJ_020_NOKI_HAIR_TEXTURE_ID_START + (byte)(hairColour);
		
		*((volatile int*)(offMaterials + (OBJ_020_NOKI_SHELL_MATERIAL_ID * BMD_NUM_BYTES_IN_MATERIAL_HEADER) + 0x04)) = shellTex;
		*((volatile int*)(offMaterials + (OBJ_020_NOKI_SHELL_MATERIAL_ID * BMD_NUM_BYTES_IN_MATERIAL_HEADER) + 0x08)) = shellTex;
		*((volatile int*)(offMaterials + (OBJ_020_NOKI_HAIR_MATERIAL_ID * BMD_NUM_BYTES_IN_MATERIAL_HEADER) + 0x04)) = hairTex;
		*((volatile int*)(offMaterials + (OBJ_020_NOKI_HAIR_MATERIAL_ID * BMD_NUM_BYTES_IN_MATERIAL_HEADER) + 0x08)) = hairTex;
	}
	else if (type == OBJ_020_ENM_PIANTA)
	{
		// Set first bone to Pianta root bone, Noki structure will 
		// be hidden
		unsigned int offPiantaBones = OBJ_020_BMDBoneSectionOriginalOffset + 
			(OBJ_020_NUM_BONES_PER_ENEMY * BMD_NUM_BYTES_IN_BONE_HEADER);
		*((volatile unsigned int*)(OBJ_020_BMDAddress + 0x08)) = 
			offPiantaBones;
		
		OBJ_020_PiantaSkinColour skinColour = OBJ_020_PiantaSkinColour((byte)((param03 & 0x0F00) >> 8));
		
		unsigned int skinTex = OBJ_020_PIANTA_SKIN_TEXTURE_ID_START + (byte)(skinColour);
		unsigned int leavesTex = OBJ_020_PIANTA_LEAVES_TEXTURE_ID_START + (byte)(sex);
		unsigned int braTex = OBJ_020_PIANTA_BRA_TEXTURE_ID_START + (byte)(sex);
		unsigned int flowerTex = (sex == OBJ_020_SEX_MALE) ? 
			(OBJ_020_PIANTA_BRA_TEXTURE_ID_START + (byte)(sex)) : 
			(OBJ_020_PIANTA_FLOWER_TEXTURE_ID_START);
		
		*((volatile int*)(offMaterials + (OBJ_020_PIANTA_SKIN_MATERIAL_ID * BMD_NUM_BYTES_IN_MATERIAL_HEADER) + 0x04)) = skinTex;
		*((volatile int*)(offMaterials + (OBJ_020_PIANTA_SKIN_MATERIAL_ID * BMD_NUM_BYTES_IN_MATERIAL_HEADER) + 0x08)) = skinTex;
		*((volatile int*)(offMaterials + (OBJ_020_PIANTA_LEAVES_MATERIAL_ID * BMD_NUM_BYTES_IN_MATERIAL_HEADER) + 0x04)) = leavesTex;
		*((volatile int*)(offMaterials + (OBJ_020_PIANTA_LEAVES_MATERIAL_ID * BMD_NUM_BYTES_IN_MATERIAL_HEADER) + 0x08)) = leavesTex;
		*((volatile int*)(offMaterials + (OBJ_020_PIANTA_BRA_MATERIAL_ID * BMD_NUM_BYTES_IN_MATERIAL_HEADER) + 0x04)) = braTex;
		*((volatile int*)(offMaterials + (OBJ_020_PIANTA_BRA_MATERIAL_ID * BMD_NUM_BYTES_IN_MATERIAL_HEADER) + 0x08)) = braTex;
		*((volatile int*)(offMaterials + (OBJ_020_PIANTA_FLOWER_MATERIAL_ID * BMD_NUM_BYTES_IN_MATERIAL_HEADER) + 0x04)) = flowerTex;
		*((volatile int*)(offMaterials + (OBJ_020_PIANTA_FLOWER_MATERIAL_ID * BMD_NUM_BYTES_IN_MATERIAL_HEADER) + 0x08)) = flowerTex;
	}
	
	OBJ_020_BMDAddressOld = OBJ_020_BMDAddress;
	
	*((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET)) = 
		*((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_PARAMETER_02_OFFSET));
	if (param02 != 0x01 && param02 != 0x03)
	{
		*((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_PARAMETER_02_OFFSET)) = 0x0000;
	}
}

// Change the collision radius 
void hook_0212D084_ov_54()
{
	asm
	(
		"ldr r2, =OBJ_020_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	);
	
	unsigned short int param03 = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_PARAMETER_03_OFFSET));
	
	OBJ_020_EnemyType type = OBJ_020_EnemyType((byte)(param03 & 0x000F));
	
	if (type == OBJ_020_ENM_PIANTA)
	{
		*((volatile int*)(OBJ_020_ObjectAddress + OBJ_020_COLLISION_HORIZONTAL_RADIUS_OFFSET)) = 
			OBJ_020_PIANTA_COLLISION_RADIUS;
		*((volatile int*)(OBJ_020_ObjectAddress + OBJ_020_COLLISION_VERTICAL_RADIUS_OFFSET)) = 
			OBJ_020_PIANTA_COLLISION_RADIUS;
	}
}

// Disable creaking sound effect playing when near B.O. Buddy
void repl_0212CFFC_ov_54() { }

// Change direction Mario's head faces when talking to B.O. Buddy
unsigned int OBJ_020_MarioTalkDirectionSettings = 0xFFFFFFFF;
void hook_0212CA40_ov_54()
{
	asm
	(
		"ldr r2, =OBJ_020_ObjectAddress					\t\n"
		"str r1, [r2]									\t\n"
		"ldr r2, =OBJ_020_MarioTalkDirectionSettings	\t\n"
		"str r3, [r2]									\t\n"
	);
	
	unsigned short int param03 = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_PARAMETER_03_OFFSET));
	
	OBJ_020_EnemyType type = OBJ_020_EnemyType((byte)(param03 & 0x000F));
	
	if (type == OBJ_020_ENM_NOKI)
	{
		*((volatile int*)(OBJ_020_MarioTalkDirectionSettings + 0x04)) = 
			*((volatile int*)(OBJ_020_ObjectAddress + OBJ_Y_LOC_OFFSET)) + OBJ_020_NOKI_MARIO_HEAD_TALK_Y_OFFSET;
	}
	else if (type == OBJ_020_ENM_PIANTA)
	{
		*((volatile unsigned int*)(OBJ_020_MarioTalkDirectionSettings + 0x04)) = 
			*((volatile int*)(OBJ_020_ObjectAddress + OBJ_Y_LOC_OFFSET)) + OBJ_020_PIANTA_MARIO_HEAD_TALK_Y_OFFSET;
	}
}

// Increase the shadow radius for Piantas
void hook_0212CEBC_ov_54()
{
	asm
	(
		"ldr r2, =OBJ_020_ObjectAddress					\t\n"
		"str r4, [r2]									\t\n"
	);
	
	unsigned short int param03 = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_PARAMETER_03_OFFSET));
	
	OBJ_020_EnemyType type = OBJ_020_EnemyType((byte)(param03 & 0x000F));
	
	if (type == OBJ_020_ENM_PIANTA)
	{
		*((volatile int*)(OBJ_020_ObjectAddress + OBJ_020_SHADOW_RADIUS_X_OFFSET)) = 
			OBJ_020_PIANTA_SHADOW_RADIUS;
		*((volatile int*)(OBJ_020_ObjectAddress + OBJ_020_SHADOW_RADIUS_Y_OFFSET)) = 
			OBJ_020_PIANTA_SHADOW_RADIUS;
		*((volatile int*)(OBJ_020_ObjectAddress + OBJ_020_SHADOW_RADIUS_Z_OFFSET)) = 
			OBJ_020_PIANTA_SHADOW_RADIUS;
	}
}

// Change message spoken to match parameter 1 value or override with 
// parameter 2 settings if specified
unsigned short int OBJ_020_MessageIndex = 0x0000;
void hook_0212CA44_ov_54()
{
	unsigned short int param02Copy = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET));
	OBJ_020_SpecialMissionID specialMissionID = OBJ_020_SpecialMissionID((byte)(param02Copy & 0x00FF));
	
	if (specialMissionID == OBJ_020_SPCMSN_NONE)
	{
		OBJ_020_MessageIndex = GetMessageIndex_SPCMSN_NONE();
	}
	else if (specialMissionID == OBJ_020_SPCMSN_GB_WATERMELONS)
	{
		OBJ_020_MessageIndex = GetMessageIndex_SPCMSN_GB_WATERMELONS();
	}
	else if (specialMissionID == OBJ_020_SPCMSN_RH_ACTIVATE_CRANES)
	{
		OBJ_020_MessageIndex = GetMessageIndex_SPCMSN_RH_ACTIVATE_CRANES();
	}
	else if (specialMissionID == OBJ_020_SPCMSN_RHS_SEWER_RACE)
	{
		OBJ_020_MessageIndex = GetMessageIndex_SPCMSN_RHS_SEWER_RACE();
	}
}

unsigned short int GetMessageIndex_SPCMSN_NONE()
{
	unsigned short int param01 = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_PARAMETER_01_OFFSET));
	unsigned short int param02Copy = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET));
	byte msgSettings = (byte)((param02Copy & 0xFF00) >> 8);
	bool sameMsgAllCharacters = (msgSettings & 0x0F) == 0;
	
	unsigned short int result = param01;
	
	if (!sameMsgAllCharacters)
	{
		result = (unsigned short int)(result + CHARACTER);
	}
	
	return result;
}

unsigned short int GetMessageIndex_SPCMSN_GB_WATERMELONS()
{
	unsigned short int param01 = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_PARAMETER_01_OFFSET));
	bool handedOutPrize = *((volatile byte*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET + 0x02)) & 0x01;
	
	if (!GB_CompetitionWon)
	{
		if (GB_WatermelonCurrentEntry == 0xFFFFFFFF)
		{
			// No entry
			return param01;
		}
		else
		{
			// Too small
			return (param01 + 0x0001);
		}
	}
	else 
	{
		if (!handedOutPrize)
		{
			return (param01 + 0x0002);
		}
		else 
		{
			return (param01 + 0x0003);
		}
	}
}

unsigned short int GetMessageIndex_SPCMSN_RH_ACTIVATE_CRANES()
{
	unsigned short int param01 = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_PARAMETER_01_OFFSET));
	bool handedOutStar = *((volatile byte*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET + 0x02)) & 0x01;
	
	if (STAR_ID == RH_ActivateCranesMissionStarID)
	{
		if (!RH_AreCranesActivated)
		{
			return param01;
		}
		else
		{
			if (!handedOutStar)
			{
				return (param01 + 0x0001);
			}
			else
			{
				return (param01 + 0x0002);
			}
		}
	}
	else
	{
		if (!RH_AreCranesActivated)
		{
			return (param01 + 0x0003);
		}
		else
		{
			return (param01 + 0x0002);
		}
	}
}

unsigned short int GetMessageIndex_SPCMSN_RHS_SEWER_RACE()
{
	unsigned short int param01 = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_PARAMETER_01_OFFSET));
	bool givenIntroduction = *((volatile byte*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET + 0x02)) & 0x01;
	
	if (!STR_RaceStarted)
	{
		if (!givenIntroduction)
		{
			*((volatile byte*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET + 0x02)) |= 0x01;
			
			return param01;
		}
		else
		{
			return (param01 + 0x0001);
		}
	}
	else
	{
		if (STR_RaceFinished)
		{
			if (STR_RaceWon)
			{
				// Winner
				return (param01 + 0x0002);
			}
			else
			{
				if (RHS_NumRaceMarkersPassed == RHS_TotalNumRaceMarkers)
				{
					// All rings but too slow
					return (param01 + 0x0004);
				}
				else
				{
					// Reached finish but didn't collect all rings
					return (param01 + 0x0003);
				}
			}
		}
		else
		{
			return (param01 + 0x0001);
		}
	}
}

// Read message index from OBJ_020_MessageIndex
void repl_0212CA48_ov_54()
{
	asm
	(
		"ldr r2, =OBJ_020_MessageIndex	\t\n"
		"ldrh r2, [r2]					\t\n"
	);
}

// Perform specific action at very end of interaction with Mario
void hook_0212C6D4_ov_54()
{
	asm
	(
		"ldr r2, =OBJ_020_ObjectAddress					\t\n"
		"str r6, [r2]									\t\n"
	);
	
	unsigned short int param02Copy = *((volatile unsigned short int*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET));
	OBJ_020_SpecialMissionID specialMissionID = OBJ_020_SpecialMissionID((byte)(param02Copy & 0x00FF));
	
	if (specialMissionID == OBJ_020_SPCMSN_NONE)
	{
		FinishAction_SPCMSN_NONE();
	}
	else if (specialMissionID == OBJ_020_SPCMSN_GB_WATERMELONS)
	{
		FinishAction_SPCMSN_GB_WATERMELONS();
	}
	else if (specialMissionID == OBJ_020_SPCMSN_RH_ACTIVATE_CRANES)
	{
		FinishAction_SPCMSN_RH_ACTIVATE_CRANES();
	}
	else if (specialMissionID == OBJ_020_SPCMSN_RHS_SEWER_RACE)
	{
		FinishAction_SPCMSN_RHS_SEWER_RACE();
	}
}

void OBJ_020_SpawnStar(byte starID)
{
	// Star Spawning
	TmpThreeIntArray[0] = *((volatile int*)(OBJ_020_ObjectAddress + OBJ_X_LOC_OFFSET));
	TmpThreeIntArray[1] = *((volatile int*)(OBJ_020_ObjectAddress + OBJ_Y_LOC_OFFSET)); 
	TmpThreeIntArray[2] = *((volatile int*)(OBJ_020_ObjectAddress + OBJ_Z_LOC_OFFSET));
	unsigned short int param01 = (starID | (byte)(0x40));
	
	SpawnActor(OBJ_061_ACTOR_ID, param01, TmpThreeIntArray, 0x00000000);
}

void FinishAction_SPCMSN_NONE()
{
	return;
}

void FinishAction_SPCMSN_GB_WATERMELONS()
{
	bool handedOutPrize = *((volatile byte*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET + 0x02)) & 0x01;
	
	if (!GB_CompetitionWon)
	{
		if (GB_WatermelonCurrentEntry != 0xFFFFFFFF)
		{
			*((volatile byte*)(GB_WatermelonCurrentEntry + OBJ_014_FOUR_FREE_BYTES_01_OFFSET + 0x01)) |= 0x40;
			
			GB_WatermelonCurrentEntry = 0xFFFFFFFF;
		}
		
		return;
	}
	else
	{
		if (handedOutPrize)
		{
			return;
		}
		else
		{
			OBJ_020_SpawnStar(GB_WatermelonsMissionStarID);
			
			*((volatile byte*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET + 0x02)) |= 0x01;
		}
	}
}

void FinishAction_SPCMSN_RH_ACTIVATE_CRANES()
{
	bool handedOutStar = *((volatile byte*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET + 0x02)) & 0x01;
	
	if (!RH_AreCranesActivated)
	{
		return;
	}
	else
	{
		if (STAR_ID == RH_ActivateCranesMissionStarID)
		{
			if (handedOutStar)
			{
				return;
			}
			else
			{
				OBJ_020_SpawnStar(RH_ActivateCranesMissionStarID);
				
				*((volatile byte*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET + 0x02)) |= 0x01;
			}
		}
		else
		{
			byte starsCollected = *((volatile byte*)(STAR_COMPLETEION_BITMAP_ADDRESS + RH_ActSelectorID));
			bool craneStarCollected = (starsCollected & RH_ActivateCranesMissionStarID) > 0;
			
			if (craneStarCollected)
			{
				return;
			}
			else
			{
				if (handedOutStar)
				{
					return;
				}
				else
				{
					OBJ_020_SpawnStar(RH_ActivateCranesMissionStarID);
					
					*((volatile byte*)(OBJ_020_ObjectAddress + OBJ_020_FREE_FOUR_BYTES_OFFSET + 0x02)) |= 0x01;
				}
			}
		}
	}
}

void FinishAction_SPCMSN_RHS_SEWER_RACE()
{
	if (STR_RaceFinished && !STR_RaceWon)
	{
		LoadLevel(CURRENT_ENTRANCE_LEVEL_LEVEL_ID, CURRENT_ENTRANCE_LEVEL_ENTRANCE_ID, 0xFFFFFFFF, 0);
	}
}