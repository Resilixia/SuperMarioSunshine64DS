#ifndef FLUDD
#define FLUDD

#include "SM64DS.h"

enum FLUDD_Nozzle
{
	FLUDD_NozzleHover = 0,
	FLUDD_NozzleRocket = 1,
	FLUDD_NozzleTurbo = 2,
	FLUDD_NozzleNone = 3
};

// Shared variables
extern byte FLUDD_WaterLevel;
extern byte FLUDD_WaterLevelDecrementUnitCounter;
extern byte FLUDD_WaterLevelIncrementUnitCounter;

extern FLUDD_Nozzle FLUDD_CurrentNozzle;
extern bool FLUDD_IsActive;

extern byte FLUDD_SelectButtonHeldCount;
extern byte FLUDD_SelectButtonReleasedCount;

extern bool FLUDD_NozzleTurboActive;
extern byte FLUDD_TurboBuildUpCount;
extern short int FLUDD_TurboDuration;
extern byte FLUDD_TurboSoundEffectCounter;
extern bool FLUDD_NozzleTurboWallCrash;
extern bool FLUDD_NozzleTurboBuildingUpOrInUse;

extern bool FLUDD_NozzleRocketActive;
extern byte FLUDD_RocketBuildUpCount;
extern byte FLUDD_ROCKET_REQUIRED_BUILD_UP;
extern byte FLUDD_RocketDuration;
extern bool FLUDD_NozzleRocketJump;

extern bool FLUDD_NozzleHoverActive;
extern byte FLUDD_HoverDuration;
extern byte FLUDD_HOVER_DURATION_LIMIT;
extern bool FLUDD_NozzleHoverSideFlip;
extern byte FLUDD_HoverNextUseWaitTimer;
extern byte FLUDD_HOVER_NEXT_USE_WAIT_LIMIT;

extern bool FLUDD_PlayerStandingInWater;
extern bool FLUDD_PlayerSwimming;
extern bool FLUDD_PlayerOnGround;
extern bool FLUDD_PlayerCollidingWithWall;

extern unsigned int FLUDD_KeysAddress;
extern bool FLUDD_PlayerUpPressed;
extern bool FLUDD_PlayerLeftPressed;
extern bool FLUDD_PlayerRightPressed;
extern bool FLUDD_PlayerDownPressed;
extern bool FLUDD_PlayerDPadPressed;

// Constants
const byte FLUDD_SELECT_BUTTON_REQUIRED_DURATION = 0x02;
const byte FLUDD_SELECT_BUTTON_REQUIRED_RELEASE = 0x02;

const byte FLUDD_TURBO_REQUIRED_BUILD_UP = 40;
const short int FLUDD_TURBO_DURATION_LIMIT = 300;

const byte FLUDD_ROCKET_DURATION_LIMIT_B_KEY = 10;
const byte FLUDD_ROCKET_DURATION_LIMIT_JUMP = 80;

const int FLUDD_HOVER_VERTICAL_SPEED = 0x00008000;
const int FLUDD_HOVER_MAX_FORWARD_SPEED = 0x010000;
const int FLUDD_HOVER_FORWARD_SPEED_INCREMENT = 0x002000;
const short int FLUDD_HOVER_TURN_INCREMENT = 0x0200;
const short int FLUDD_HOVER_TURN_WATER_NOZZLE_ROTATION = 0x0500;

const byte FLUDD_HOVER_SOUND_EFFECT = 0x17;
const byte FLUDD_ROCKET_BUILD_UP_SOUND_EFFECT = 0x17;
const byte FLUDD_ROCKET_BLAST_OFF_SOUND_EFFECT = 0xD5;
const byte FLUDD_TURBO_SOUND_EFFECT = 0x17;
const byte FLUDD_REFILL_SOUND_EFFECT = 0x17;

const byte FLUDD_BCA_SCALE_X_OFFSET = 0x0C;
const byte FLUDD_BCA_SCALE_Y_OFFSET = 0x10;
const byte FLUDD_BCA_SCALE_Z_OFFSET = 0x14;
const byte FLUDD_BCA_ROTATION_X_OFFSET = 0x1A;
const byte FLUDD_BCA_ROTATION_Y_OFFSET = 0x1C;
const byte FLUDD_BCA_ROTATION_Z_OFFSET = 0x1E;
const byte FLUDD_BCA_TRANSLATION_X_OFFSET = 0x20;
const byte FLUDD_BCA_TRANSLATION_Y_OFFSET = 0x24;
const byte FLUDD_BCA_TRANSLATION_Z_OFFSET = 0x28;

extern byte FLUDD_CHARACTER_FIRST_FLUDD_BONE_INDEX;

const byte FLUDD_CHARACTER_SKL_ROOT_BONE_INDEX = 0x01;
const byte FLUDD_CHARACTER_SPIN_BONE_INDEX = 0x08;
const byte FLUDD_CHARACTER_ARM_L1_BONE_INDEX = 0x09;
const byte FLUDD_CHARACTER_ARM_L2_BONE_INDEX = 0x0A;
const byte FLUDD_CHARACTER_WRIST_L1_BONE_INDEX = 0x0B;
const byte FLUDD_CHARACTER_ARM_R1_BONE_INDEX = 0x0C;
const byte FLUDD_CHARACTER_ARM_R2_BONE_INDEX = 0x0D;
const byte FLUDD_CHARACTER_WRIST_R1_BONE_INDEX = 0x0E;
const byte FLUDD_CHARACTER_FACE_BONE_INDEX = 0x0F;
const byte FLUDD_CHARACTER_HANDLE_LEFT_BONE_INDEX = 0x11;
const byte FLUDD_CHARACTER_HANDLE_RIGHT_BONE_INDEX = 0x12;

const byte FLUDD_MARIO_ALL_ROOT_BONE_INDEX = 0x00;
const byte FLUDD_MARIO_WATER_NOZZLE_L_BONE_INDEX = 0x14;
const byte FLUDD_MARIO_WATERBOOST_L_BONE_INDEX = 0x15;
const byte FLUDD_MARIO_WATER_NOZZLE_R_BONE_INDEX = 0x16;
const byte FLUDD_MARIO_WATERBOOST_R_BONE_INDEX = 0x17;

const byte FLUDD_LUIGI_WATERBOOST_1_BONE_INDEX = 0x14;

const byte FLUDD_WARIO_TURBO_BLADES_BONE_INDEX = 0x14;
const byte FLUDD_WARIO_WATERBOOST_1_BONE_INDEX = 0x15;

const unsigned int FLUDD_ADDRESS_OF_POINTER_TO_JUMP_FUNCTION = 0x0211019C;
const unsigned int FLUDD_ADDRESS_OF_POINTER_TO_JUMP_DOWN_FUNCTION = 0x02110424;
const unsigned int FLUDD_ADDRESS_OF_POINTER_TO_FALL_FUNCTION = 0x021101B4;

// Methods 
void DecrementWaterLevel();
void IncrementWaterLevel();
void SubtractFullUnitsFromWaterLevel(byte amount);
bool IsPlayerStandingInWater();
bool IsPlayerSwimming();
bool IsPlayerOnGround();
bool IsPlayerCollidingWithWall();
void PlaySoundEffect(byte sound_effect);
void RotatePlayerOnLeftRightPress(short int left_increment, short int right_increment);
bool NozzleRocketBuildingUpOrInUse();
bool NozzleTurboBuildingUpOrInUse();
void CrashPlayerIntoWall();

#endif