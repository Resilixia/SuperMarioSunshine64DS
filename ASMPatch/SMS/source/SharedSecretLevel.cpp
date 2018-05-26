#include "SharedSecretLevel.h"
#include "GelatoBeach.h"

byte SSL_LevelID = LEVEL_PlayRoom;

LevelSettings SSL_RequiredLevelSettings = 
{
	{ 0, 0, 5, 1, 4, 0, 0, 1 }, 
	1958, 
	1956, 
	1959, 
	1960, 
	false, 
	1600, 
	1, 
	{ 3, 32, 57 }, 
	29
};
byte SSL_RequiredObjectBank07 = 0x01;
byte SSL_RequiredStarID = 0x01;

// Change level settings depending on previous level
void hook_0214EAAC_ov_99()
{
	if (PreviousLevelID == GB_LevelID)
	{
		SSL_RequiredLevelSettings = GB_SecretLevel_Settings;
		SSL_RequiredStarID = GB_SecretLevel_StarID;
	}
	
	// Object banks
	for (int i = 0; i < 7; i++)
	{
		*((volatile byte*)(LEVEL_OVERLAY_LOAD_ADDRESS + OBJECT_BANK_0_OFFSET + i)) = 
			SSL_RequiredLevelSettings.ObjectBankSettings[i];
	}
	SSL_RequiredObjectBank07 = SSL_RequiredLevelSettings.ObjectBankSettings[7];
	*((volatile byte*)(LEVEL_OVERLAY_LOAD_ADDRESS + OBJECT_BANK_7_OFFSET)) = 
		SSL_RequiredObjectBank07;
	// File ID's
	*((volatile unsigned short*)(LEVEL_OVERLAY_LOAD_ADDRESS + BMD_FILE_ID_OFFSET)) = 
		SSL_RequiredLevelSettings.BMD_FileID;
	*((volatile unsigned short*)(LEVEL_OVERLAY_LOAD_ADDRESS + KCL_FILE_ID_OFFSET)) = 
		SSL_RequiredLevelSettings.KCL_FileID;
	*((volatile unsigned short*)(LEVEL_OVERLAY_LOAD_ADDRESS + ICG_FILE_ID_OFFSET)) = 
		SSL_RequiredLevelSettings.ICG_FileID;
	*((volatile unsigned short*)(LEVEL_OVERLAY_LOAD_ADDRESS + ICL_FILE_ID_OFFSET)) = 
		SSL_RequiredLevelSettings.ICL_FileID;
	// Start camera zoomed out
	*((volatile byte*)(LEVEL_OVERLAY_LOAD_ADDRESS + CAMERA_START_ZOOMED_OUT_OFFSET)) = 
		SSL_RequiredLevelSettings.CameraStartZoomedOut;
	// Minimap scale factor
	*((volatile unsigned short*)(LEVEL_OVERLAY_LOAD_ADDRESS + MINIMAP_SCALE_OFFSET)) = 
		SSL_RequiredLevelSettings.MinimapScaleFactor;
	// Skybox
	byte sbValue = 0x0F | (SSL_RequiredLevelSettings.SkyBox << 4);
	*((volatile byte*)(LEVEL_OVERLAY_LOAD_ADDRESS + SKYBOX_OFFSET)) = sbValue;
	// Music settings
	for (int i = 0; i < 3; i++)
	{
		*((volatile byte*)(LEVEL_OVERLAY_LOAD_ADDRESS + MUSIC_SETTING_0_OFFSET + i)) = 
			SSL_RequiredLevelSettings.MusicSettings[i];
	}
	
	// Star ID
	STAR_ID = SSL_RequiredStarID;
	
	asm
	(
		"pop {r0}							\t\n"
		"ldr r0, =SSL_RequiredObjectBank07	\t\n"
		"ldrb r0, [r0]						\t\n"
		"push {r0}							\t\n"
		
		"cmp r0, #0x00						\t\n" // Original instruction
	);
}

// Load level objects programmatically in addition to overlay objects
void hook_020FE7D8()
{
	if (CurrentLevelID != SSL_LevelID) return;
	
	if (PreviousLevelID == GB_LevelID)
	{
		GB_LoadSecretLevel();
	}
}

