/* Constants for the Gelato Beach level
 */

#ifndef GELATO_BEACH
#define GELATO_BEACH

#include "SM64DS.h"
#include "atan2.h"

extern byte GB_LevelID;

extern byte GB_AlignMirrorsMissionStarID;
extern byte GB_AlignMirrorsMissionLevelID;
extern int GB_AlignedMirrorsStarSpawnStartPos[3];

extern byte GB_SandBirdMissionStarID;
extern byte GB_SandBirdMissionLevelID;

extern byte GB_WatermelonsMissionStarID;
extern byte GB_NumWatermelons;
extern unsigned int GB_Watermelons[3];
extern unsigned int GB_WatermelonGoal;
extern unsigned int GB_WatermelonCurrentEntry;
extern bool GB_CompetitionWon;

short int const OBJ_014_FOUR_FREE_BYTES_01_OFFSET = 0x84;
short int const OBJ_014_FOUR_FREE_BYTES_02_OFFSET = 0xD8;
short int const OBJ_014_TWELVE_FREE_BYTES_OFFSET = 0xDC;

extern byte GB_WigglerMissionStarID;
extern byte GB_WigglerMissionObjectBank7Value;

extern LevelSettings GB_SecretLevel_Settings;
extern byte GB_SecretLevel_StarID;

void GB_LoadSecretLevel();

#endif