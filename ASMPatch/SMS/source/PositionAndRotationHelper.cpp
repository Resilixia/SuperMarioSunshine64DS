#include "SM64DS.h"
#include "PositionAndRotationHelper.h"
#include "atan2.h"
#include "MathsHelper.h"

void SetMarioPositionUpdate(unsigned int object_address)
{
	*((volatile unsigned int*)(object_address + OBJ_UPDATE_MARIO_POS_OFFSET)) = 
		UPDATE_MARIO_POS_ON_ROT_METHOD;
}

__attribute__((noinline)) void SetObjectPositionASR3(unsigned int object_address, short int position_asr3_offset) 
{
	int xPos_asr3 = (int)(*((volatile int*)(object_address + OBJ_X_LOC_OFFSET)) >> 3);
	int yPos_asr3 = (int)(*((volatile int*)(object_address + OBJ_Y_LOC_OFFSET)) >> 3);
	int zPos_asr3 = (int)(*((volatile int*)(object_address + OBJ_Z_LOC_OFFSET)) >> 3);
	
	*((volatile int*)(object_address + (position_asr3_offset + 0x00))) = xPos_asr3;
	*((volatile int*)(object_address + (position_asr3_offset + 0x04))) = yPos_asr3;
	*((volatile int*)(object_address + (position_asr3_offset + 0x08))) = zPos_asr3;
}

__attribute__((noinline)) void UpdateObjectModelPositionAndRotation(unsigned int object_address, 
																	unsigned short model_rotation_offset, 
																	short int position_asr3_offset) 
{
	short int xRot = *((volatile short int*)(object_address + OBJ_X_ROT_OFFSET));
	short int rot = *((volatile short int*)(object_address + OBJ_Y_ROT_OFFSET));
	short int zRot = *((volatile short int*)(object_address + OBJ_Z_ROT_OFFSET));
	
	OBJ_UpdateObjectModelRotation((unsigned int)(object_address + model_rotation_offset), xRot, rot, zRot);
	SetObjectPositionASR3(object_address, position_asr3_offset);
}

__attribute__((noinline)) void UpdateObjectCollisionPositionAndRotation(unsigned int object_address, 
																		short int position_asr3_offset) 
{
	SetObjectPositionASR3(object_address, position_asr3_offset);
	OBJ_UpdateObjectCollisionRotation(object_address);
}

__attribute__((noinline)) void UpdateObjectPositionAndRotation(unsigned int object_address, 
															   unsigned short model_rotation_offset, 
															   short int position_asr3_offset)
{
	UpdateObjectModelPositionAndRotation(object_address, model_rotation_offset, position_asr3_offset);
	UpdateObjectCollisionPositionAndRotation(object_address, position_asr3_offset);
}

__attribute__((noinline)) void UpdatePositionFromSpeed(unsigned int object_address) 
{
	*((volatile int*)(object_address + OBJ_X_LOC_OFFSET)) += *((volatile int*)(object_address + OBJ_X_SPEED_OFFSET));
	*((volatile int*)(object_address + OBJ_Y_LOC_OFFSET)) += *((volatile int*)(object_address + OBJ_Y_SPEED_OFFSET));
	*((volatile int*)(object_address + OBJ_Z_LOC_OFFSET)) += *((volatile int*)(object_address + OBJ_Z_SPEED_OFFSET));
}

__attribute__((noinline)) void TurnToFaceMario(unsigned int object_address, int angle_increment, AXIS rotation_axis) 
{
	AXIS firstPosAxis, secondPosAxis;
	int playerFirstAxisPos, playerSecondAxisPos = -1;
	switch (rotation_axis) 
	{
		case AXIS_Z: // MAY NOT WORK
			firstPosAxis = AXIS_X;
			secondPosAxis = AXIS_Y;
			playerFirstAxisPos = PLAYER->xPos;
			playerSecondAxisPos = PLAYER->yPos;
			break;
		default: 
		case AXIS_Y: 
			firstPosAxis = AXIS_X;
			secondPosAxis = AXIS_Z;
			playerFirstAxisPos = PLAYER->xPos;
			playerSecondAxisPos = PLAYER->zPos;
			break;
		case AXIS_X: // MAY NOT WORK
			firstPosAxis = AXIS_Z;
			secondPosAxis = AXIS_Y;
			playerFirstAxisPos = PLAYER->zPos;
			playerSecondAxisPos = PLAYER->yPos;
			break;
	}
	
	int posOnFirstAxis = *((volatile int*)(object_address + (OBJ_X_LOC_OFFSET + (firstPosAxis * 4))));
	int posOnSecondAxis = *((volatile int*)(object_address + (OBJ_X_LOC_OFFSET + (secondPosAxis * 4))));
	
	unsigned short int angle = (unsigned short int)atan2Cordic(posOnSecondAxis - playerSecondAxisPos, posOnFirstAxis - playerFirstAxisPos);
	short int targetRot = (short int)((angle * 2) + 0x8000);
	short int rotationOffset = (OBJ_X_ROT_OFFSET + (rotation_axis * 2));
	
	if (angle_increment <= 0) 
	{
		*((volatile short int*)(object_address + rotationOffset)) = targetRot;
	}
	else 
	{	
		short int rot = *((volatile short int*)(object_address + rotationOffset)); 
		short int rotDiff = CalculateSmallestObjectAngleDifference(rot, targetRot);
		
		if (rotDiff != 0)
		{
			short int rotIncrement = (rotDiff > 0) ? 
				((rotDiff > angle_increment) ? angle_increment : rotDiff) : 
				((rotDiff < 0 - angle_increment) ? (0 - angle_increment) : rotDiff);
			
			*((volatile short int*)(object_address + rotationOffset)) += rotIncrement;
		}
	}
	
	if (rotation_axis == AXIS_Y) 
	{
		*((volatile short int*)(object_address + OBJ_FORWARD_DIRECTION_OFFSET)) = 
			*((volatile short int*)(object_address + rotationOffset));
	}
	
	return;
}
