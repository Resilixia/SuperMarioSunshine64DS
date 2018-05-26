#include "RiccoHarbour.h"
#include "PositionAndRotationHelper.h"

unsigned int OBJ_106_ObjectAddress = 0xFFFFFFFF;
unsigned int OBJ_107_ObjectAddress = 0xFFFFFFFF;

short int const OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 = 0xA0;
short int const OBJ_106_107_FREE_FOUR_BYTES_OFFSET_02 = 0xB4;
short int const OBJ_106_107_FREE_TWELVE_BYTES_OFFSET = 0xA4;

const int OBJ_106_CRANE_ARM_LENGTH = 0x3E8000; // 1.0
const short int OBJ_106_VERTICAL_CRANE_ANGLE = 0x2A00;

// Parameters: 
// 	Parameter 1: rotation size in BRadians
// 	Parameter 2: 
// 		byte 1: (rotation increment in BRadians) >> 1, ie. value of 2: shift left to get 4
// 		byte 2: 
// 				bits 1 - 4: 0 = horizontal, 1 = vertical
// 				bits 5 - 8: y rotation: 0 = 0 deg, 1 = 90 deg, 2 = 180 deg, 3 = 270
// OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01: 
// 		bytes 1 - 2: current rotation offset in BRadians
// 		byte 3: (rotation increment in BRadians) >> 1, ie. value of 2: shift left to get 4
// 		byte 4: 
//				bit 1: bool, 0 = decrement, 1 = increment (direction of rotation)
// 				bit 2: bool, 0 = horizontal, 1 = vertical
// OBJ_106_107_FREE_TWELVE_BYTES_OFFSET: 
// 		int 0x00: hor.: starting X pos.
// 		int 0x04: hor.: starting Y pos.
// 		int 0x08: hor.: starting Z pos.
// OBJ_106_107_FREE_FOUR_BYTES_OFFSET_02: 
// 		sint 0x00: starting y rot. for horizontal cranes
// 		byte 0x02: y rotation: 0 = 0 deg, 1 = 90 deg, 2 = 180 deg, 3 = 270

//////////////////////////// SHARED METHODS ////////////////////////////

void InitialiseSettings(unsigned int objectAddress)
{
	SetMarioPositionUpdate(objectAddress);
	
	short int param01 = *((volatile short int*)(objectAddress + OBJ_PARAMETER_01_OFFSET));
	short int param02 = *((volatile short int*)(objectAddress + OBJ_PARAMETER_02_OFFSET));
	
	*((volatile byte*)(objectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x02)) = (byte)(param02 & 0x00FF);
	*((volatile byte*)(objectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x03)) |= 0x01;
	bool isVertical = (((param02 & 0x0F00) >> 8) >= 1);
	byte yRotDir = (byte)((param02 & 0xF000) >> 12);
	if (isVertical)
	{
		param01 = OBJ_106_VERTICAL_CRANE_ANGLE;
		*((volatile short int*)(objectAddress + OBJ_PARAMETER_01_OFFSET)) = param01;
		*((volatile short int*)(objectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01)) = (short int)(param01 / 2);
		*((volatile byte*)(objectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x03)) |= 0x02;
	}
	else
	{
		*((volatile short int*)(objectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01)) = 0;
	}
	// For hor. changing y rotation can be done by simply setting the Y rot. value as rotation is done by 
	// += or -= with a separate count for knowing how many degrees turned. However, vertical 
	// cranes will require different combinations of flipping x, y and z locations
	short int yRot = (short int)(0x2000 * yRotDir);
	*((volatile short int*)(objectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_02)) = yRot;
	*((volatile short int*)(objectAddress + OBJ_Y_ROT_OFFSET)) = (short int)(yRot * 2);
	*((volatile byte*)(objectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_02 + 0x02)) = yRotDir;
	
	*((volatile unsigned short int*)(objectAddress + OBJ_PARAMETER_02_OFFSET)) = 0x0000;
}

//////////////////////////// WL_SUBMARINE - CRANE ARM ////////////////////////////

// OBJ 106: 
// - Ensure that object always loads and isn't dependent on Stars acquired
void repl_02111A30_ov_1A()
{
	asm
	(
		"cmp r0, r0		\t\n"
	);
}

// OBJ 106: 
// - Change method value at +0x13C to ensure that Mario's position is updated when 
//   object's position and rotation are updated
// - Initialise parameters
void hook_02111A4C_ov_1A()
{
	asm
	(
		"ldr r2, =OBJ_106_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	);
	
	InitialiseSettings(OBJ_106_ObjectAddress);
}

// OBJ 106: 
// - Update model and collision maps based on X, Y and Z rotation 
// - Rotate crane about X/Y axis for vertical/horizontal for amount specified in 
//   param. 01 at increment specified in param. 02 
void hook_021119B0_ov_1A()
{
	asm
	(
		"ldr r1, =OBJ_106_ObjectAddress		\t\n"
		"str r5, [r1]						\t\n"
	);
	
	short int rotationSizeBRad = *((volatile short int*)(OBJ_106_ObjectAddress + OBJ_PARAMETER_01_OFFSET));
	short int currentYRotOffsetBRad = *((volatile short int*)(OBJ_106_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01));
	short int incrementValueBRad = (short int)(*((volatile byte*)(OBJ_106_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x02)) << 1);
	bool increment = *((volatile byte*)(OBJ_106_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x03)) & 0x01;
	bool isVertical = *((volatile byte*)(OBJ_106_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x03)) & 0x02;
	short int rotValToUpdateOffset = (isVertical) ? OBJ_X_ROT_OFFSET : OBJ_Y_ROT_OFFSET;
	
	UpdateObjectPositionAndRotation(OBJ_106_ObjectAddress, 0xF0, 0x114);
	
	if (!RH_AreCranesActivated || GAME_PAUSED)
	{
		// If the cranes aren't activated, they should wait in their initial positions 
		// (not the positions at which they're set in SM64DSe)
		return;
	}
	
	if (increment)
	{
		if (currentYRotOffsetBRad < rotationSizeBRad)
		{
			*((volatile short int*)(OBJ_106_ObjectAddress + rotValToUpdateOffset)) += (incrementValueBRad * 2);
			*((volatile short int*)(OBJ_106_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01)) += incrementValueBRad;
		}
		else
		{
			// Reset count and flip direction
			*((volatile short int*)(OBJ_106_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01)) = rotationSizeBRad;
			*((volatile byte*)(OBJ_106_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x03)) &= 0xFE;
		}
	}
	else
	{
		if (currentYRotOffsetBRad > 0)
		{
			*((volatile short int*)(OBJ_106_ObjectAddress + rotValToUpdateOffset)) -= (incrementValueBRad * 2);
			*((volatile short int*)(OBJ_106_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01)) -= incrementValueBRad;
		}
		else
		{
			// Reset count and flip direction
			*((volatile short int*)(OBJ_106_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01)) = 0x0000;
			*((volatile byte*)(OBJ_106_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x03)) |= 0x01;
		}
	}
}

//////////////////////////// WL_KUPPA_SHUTTER - CRANE PLATFORM ////////////////////////////

// OBJ 107: 
// - Ensure that object always loads and isn't dependent on Stars acquired
void repl_02111848_ov_1A()
{
	asm
	(
		"cmp r0, r0		\t\n"
	);
}

void hook_02111864_ov_1A()
{
	asm
	(
		"ldr r2, =OBJ_107_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	);
	
	InitialiseSettings(OBJ_107_ObjectAddress);

	*((volatile int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_TWELVE_BYTES_OFFSET + 0x00)) = 
		*((volatile int*)(OBJ_107_ObjectAddress + OBJ_X_LOC_OFFSET));
	*((volatile int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_TWELVE_BYTES_OFFSET + 0x04)) = 
		*((volatile int*)(OBJ_107_ObjectAddress + OBJ_Y_LOC_OFFSET));
	*((volatile int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_TWELVE_BYTES_OFFSET + 0x08)) = 
		*((volatile int*)(OBJ_107_ObjectAddress + OBJ_Z_LOC_OFFSET));
	
	*((volatile short int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_02)) = 
		*((volatile short int*)(OBJ_107_ObjectAddress + OBJ_Y_ROT_OFFSET));
}

int OBJ_107_SinRot = 0x00000000;
int OBJ_107_CosRot = 0x00000000;
short int OBJ_107_LastRotAngle = 0x0000;
void hook_021117C0_ov_1A()
{
	asm
	(
		"ldr r1, =OBJ_107_ObjectAddress		\t\n"
		"str r5, [r1]						\t\n"
	);
	
	short int rotationSizeBRad = *((volatile short int*)(OBJ_107_ObjectAddress + OBJ_PARAMETER_01_OFFSET));
	short int currentYRotOffsetBRad = *((volatile short int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01));
	short int incrementValueBRad = (short int)(*((volatile byte*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x02)) << 1);
	bool increment = *((volatile byte*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x03)) & 0x01;
	bool isVertical = *((volatile byte*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x03)) & 0x02;
	
	// Move in a circle (see CT_MECHA_07.cpp)	
	int startingXPos = 
		*((volatile int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_TWELVE_BYTES_OFFSET + 0x00));
	int startingYPos = 
		*((volatile int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_TWELVE_BYTES_OFFSET + 0x04));
	int startingZPos = 
		*((volatile int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_TWELVE_BYTES_OFFSET + 0x08));
	short int startingYRot = 
		*((volatile short int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_02));
	
	short int currentRot = currentYRotOffsetBRad;
	
	if (isVertical)
	{
		// + 0x4000 because arm doesn't line up with x rot. = 0, unable to find a 
		// pattern relating angle to the additional increase, 0xD00 found by trial-
		// and-error for 0x2A00
		currentRot += 0x4000 + 0xA00;
	}
	else
	{
		// angle of zero lines up with "15:00" but y rot. 0 lines up with "12:00"
		currentRot += 0x2000 + (startingYRot / 2);
	}

	if (OBJ_107_LastRotAngle != currentRot)
	{
		OBJ_107_SinRot = sinLerp(currentRot);
		OBJ_107_CosRot = cosLerp(currentRot);
	}
	
	int xLocNumBlocks = (OBJ_107_CosRot * 1);
	int xLoc = xLocNumBlocks * (0 - (OBJ_106_CRANE_ARM_LENGTH / 4096));
	int zLocNumBlocks = (OBJ_107_SinRot * 1);
	int zLoc = zLocNumBlocks * (OBJ_106_CRANE_ARM_LENGTH / 4096);
	
	if (!isVertical)
	{
		*((volatile int*)(OBJ_107_ObjectAddress + OBJ_X_LOC_OFFSET)) = startingXPos + xLoc;
		*((volatile int*)(OBJ_107_ObjectAddress + OBJ_Z_LOC_OFFSET)) = startingZPos + zLoc;
	}
	else
	{
		byte yRotDir = *((volatile byte*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_02 + 0x02));
		switch (yRotDir)
		{
			case 0:
			{
				zLoc = 0 - zLoc;
				*((volatile int*)(OBJ_107_ObjectAddress + OBJ_Y_LOC_OFFSET)) = startingYPos + xLoc;
				*((volatile int*)(OBJ_107_ObjectAddress + OBJ_Z_LOC_OFFSET)) = startingZPos + zLoc;
			}
			break;
			case 1:
			{
				zLoc = 0 - zLoc;
				*((volatile int*)(OBJ_107_ObjectAddress + OBJ_Y_LOC_OFFSET)) = startingYPos + xLoc;
				*((volatile int*)(OBJ_107_ObjectAddress + OBJ_X_LOC_OFFSET)) = startingXPos + zLoc;
			}
			break;
			case 2:
			{
				*((volatile int*)(OBJ_107_ObjectAddress + OBJ_Y_LOC_OFFSET)) = startingYPos + xLoc;
				*((volatile int*)(OBJ_107_ObjectAddress + OBJ_Z_LOC_OFFSET)) = startingZPos + zLoc;
			}
			break;
			case 3:
			{
				*((volatile int*)(OBJ_107_ObjectAddress + OBJ_Y_LOC_OFFSET)) = startingYPos + xLoc;
				*((volatile int*)(OBJ_107_ObjectAddress + OBJ_X_LOC_OFFSET)) = startingXPos + zLoc;
			}
			break;
		}
	}
	
	OBJ_107_LastRotAngle = currentRot;
	
	UpdateObjectPositionAndRotation(OBJ_107_ObjectAddress, 0xF0, 0x114);
	
	if (!RH_AreCranesActivated || GAME_PAUSED)
	{
		// If the cranes aren't activated, they should wait in their initial positions 
		// (not the positions at which they're set in SM64DSe)
		return;
	}
	
	if (increment)
	{
		if (currentYRotOffsetBRad < rotationSizeBRad)
		{
			if (!isVertical)
			{
				*((volatile short int*)(OBJ_107_ObjectAddress + OBJ_Y_ROT_OFFSET)) += (incrementValueBRad * 2);
			}
			*((volatile short int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01)) += incrementValueBRad;
		}
		else
		{
			// Reset count and flip direction
			*((volatile short int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01)) = rotationSizeBRad;
			*((volatile byte*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x03)) &= 0xFE;
		}
	}
	else
	{
		if (currentYRotOffsetBRad > 0)
		{
			if (!isVertical)
			{
				*((volatile short int*)(OBJ_107_ObjectAddress + OBJ_Y_ROT_OFFSET)) -= (incrementValueBRad * 2);
			}
			*((volatile short int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01)) -= incrementValueBRad;
		}
		else
		{
			// Reset count and flip direction
			*((volatile short int*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01)) = 0x0000;
			*((volatile byte*)(OBJ_107_ObjectAddress + OBJ_106_107_FREE_FOUR_BYTES_OFFSET_01 + 0x03)) |= 0x01;
		}
	}
}
