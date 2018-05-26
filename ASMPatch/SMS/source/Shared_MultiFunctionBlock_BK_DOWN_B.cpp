#include "SM64DS.h" 

// Modifies the falling bridge from WF to allow different colours, 
// falling speeds and no falling:
// Colour		|	Behaviour
// Yellow			Solid, non-moving
// Light Blue		Rises up when stepped on for a set distance, falls back down 
// 					once player no longer standing on it
// Pink				Original behaviour with slight delay
// Orange			Sit still until stepped on, then move quickly side-to-side
// Purple			Moves vertically in a circle, can be C/CW and along X/Z axis
// Green			Moves positions, flipping back and forth at a fixed interval, 
// 					can specify vertical or horizontal and length
// Dark Blue		Falls down when stepped on for a set distance, rises back up 
// 					once player no longer standing on it

short int const OBJ_199_OBJ_PARAMETER_01_OFFSET = 0x08;
short int const OBJ_199_OBJ_PARAMETER_02_OFFSET = 0x92;
short int const OBJ_199_OBJ_PARAMETER_03_OFFSET = 0x90;

// Collision bounds value stored at 0xB8, when read is lsl 3 
// to get value to use
short int const OBJ_199_COLLISION_BOUNDS_OFFSET = 0xB8;
int const OBJ_199_COLLISION_BOUNDS_VALUE_RSHIFT3 = 0x25800;

short int const OBJ_199_OBJ_DRAW_DISTANCE_OFFSET = 0xBC;
int const OBJ_199_DRAW_DISTANCE_VALUE = 0x00600000;

// Parameter 01:
// Shared: 
// 		byte 1: Block colour

// Parameter 02:
// Pink:
// 		byte 1: safety delay in frames
// Orange:
// 		byte 1: range limit in frames
// 		byte 2: value by which 1 should be shifted left to get movement speed
// Light/Dark Blue:
// 		byte 1: range limit in frames
// 		byte 2: value by which 1 should be shifted left to get movement speed
// Purple: 
// 		byte 1: rotation increment each frame
// 		byte 2: path radius in blocks
// Green: 
// 		byte 1: number of blocks
// 		byte 2: movement axis X = 0, Y = 1, Z = 2
// Green Outline:
// 		byte 1: current position in blocks


// Parameter 03: 
// Orange: 
// 		byte 1: movement axis X = 0, Y = 1, Z = 2
// Purple:
// 		byte 1: 
// 				bits 1-4: 0 = CCW, 1 = CW
//				bits 5-8: 0 = along X axis, 1 = along Z axis
// Green:
// 		byte 1: number of frames before flipping

short int const UPDATE_MARIO_POS_OFFSET = 0x13C;
unsigned int const UPDATE_MARIO_POS_METHOD = 0x0203923C;

unsigned int OBJ_199_ObjectAddress = 0xFFFFFFFF;
unsigned int OBJ_199_BMDAddress = 0xFFFFFFFF;

int const OBJ_199_BLOCK_SIZE = 0x12C000; // 0.3
int const OBJ_199_BLOCK_SIZE_OUTLINE_PADDING = 0x12D000;

short int const OBJ_199_ACTOR_ID = 45;
short int const OBJ_243_ACTOR_ID = 287;

unsigned int OBJ_199_CollisionLoadSkipJumpAddress = 0x0213A7D8;
unsigned int OBJ_199_SpeedValuesUpdateSkipJumpAddress = 0x0213A480;

short int const OBJ_199_FREE_FOUR_BYTES_OFFSET = 0x84;
// Shared: 			byte 1, bit 1: whether Mario is on top of block
// Light Blue:		byte 2: range counter
// 		   			byte 2: range counter
// 					byte 3: max. range count
// Dark Blue:		byte 2: range counter
// 		   			byte 2: range counter
// 					byte 3: max. range count
// Pink: 			byte 1, bit 2: has been stepped on
// 					byte 2: safety delay counter
// 					byte 3: safety delay count
// Orange: 			byte 1, bit 8: movement direction, 1 = up, 0 = down 
// 		   			byte 2: range counter
// 					byte 3: max. range count
// 					byte 4: speed multiplier
// Purple: 			byte 1: bit 2: 0 = CCW, 1 = CW
// 					byte 1: bit 3: 0 = X axis rotate, 1 = Z axis rotate
// 					byte 2: rot speed 
// 					byte 3: path radius
// Green: 			byte 1, bit 2: whether outline blocks created yet
// 					byte 1, bit 3: whether outline block created at start
// 					byte 2: movement axis X = 0, Y = 1, Z = 2
// 					byte 3: number of blocks
// 					byte 4: current block
// Green Outline: 	byte 2: block index

short int const OBJ_199_FREE_TWELVE_BYTES_OFFSET = 0xA4;
// Orange: 			byte 0x00: movement axis X = 0, Y = 1, Z = 2
// Purple: 			int 0x00: original X pos
// 		 			int 0x04: original Y pos
// 					short int 0x08: rotation count
// Green:			byte 0x00: flip timer limit
// 					byte 0x01: flip timer
// 					int 0x04: starting position on movement axis
// Green Outline: 	int 0x00: parent green block address

enum OBJ_199_BlockColour 
{
	OBJ_199_BLKCLR_YELLOW = 0, 
	OBJ_199_BLKCLR_LIGHT_BLUE = 1, 
	OBJ_199_BLKCLR_PINK = 2, 
	OBJ_199_BLKCLR_ORANGE = 3, 
	OBJ_199_BLKCLR_PURPLE = 4, 
	OBJ_199_BLKCLR_GREEN = 5, 
	OBJ_199_BLKCLR_OUTLINE_GREEN = 6, 
	OBJ_199_BLKCLR_DARK_BLUE = 7
};

enum OBJ_199_Axis
{
	OBJ_199_AXS_X = 0, 
	OBJ_199_AXS_Y = 1, 
	OBJ_199_AXS_Z = 2
};

void SpawnOutlineGreenBlock(unsigned int parentAddress, int startingLocation, 
	OBJ_199_Axis axis, int blockIndex);

// Based on parameter 1, change the texture and palette used, 
// also any colour-specific initialisation
void hook_0213A7B8_ov_62()
{
	asm
	(
		"ldr r2, =OBJ_199_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
		"ldr r2, =OBJ_199_BMDAddress		\t\n"
		"str r1, [r2]						\t\n"
	);

	unsigned short int param01 = *((volatile unsigned short int*)(OBJ_199_ObjectAddress + OBJ_199_OBJ_PARAMETER_01_OFFSET));
	unsigned short int param02 = *((volatile unsigned short int*)(OBJ_199_ObjectAddress + OBJ_199_OBJ_PARAMETER_02_OFFSET));
	unsigned short int param03 = *((volatile unsigned short int*)(OBJ_199_ObjectAddress + OBJ_199_OBJ_PARAMETER_03_OFFSET));
	
	OBJ_199_BlockColour colour = OBJ_199_BlockColour((byte)(param01 & 0x00FF));

	if (colour == OBJ_199_BLKCLR_PINK)
	{
		byte safetyDelay = (byte)(param02 & 0x00FF);
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) = 
			safetyDelay;
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x02)) = 
			safetyDelay;
	}
	else if (colour == OBJ_199_BLKCLR_ORANGE)
	{
		byte rangeLimit = (byte)(param02 & 0x00FF);
		byte speedShift = (byte)(param02 >> 8);
		OBJ_199_Axis axis = OBJ_199_Axis((byte)(param03 & 0x00FF));
		
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x02)) = 
			rangeLimit;
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x03)) = 
			speedShift;
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x00)) = 
			(byte)axis;
		
		// range count mid-way so starting position gives middle position
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) = 
			(byte)(rangeLimit / 2);
		// dir = up
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) |= 0x80;
	}
	else if (colour == OBJ_199_BLKCLR_LIGHT_BLUE || colour == OBJ_199_BLKCLR_DARK_BLUE)
	{
		byte rangeLimit = (byte)(param02 & 0x00FF);
		byte speedShift = (byte)(param02 >> 8);
		
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x02)) = 
			rangeLimit;
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x03)) = 
			speedShift;
		
		// range count at zero so it starts at bottom/top
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) = 
			0x00;
	}
	else if (colour == OBJ_199_BLKCLR_PURPLE)
	{
		byte rotSpeed = (byte)(param02 & 0x00FF);
		byte pathRadius = (byte)(param02 >> 8);
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) = 
			rotSpeed;
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x02)) = 
			pathRadius;
		
		bool isCW = ((byte)(param03 & 0x000F) == 0x01);
		bool useZAxis = ((byte)((param03 & 0x00F0) >> 4) == 0x01);
		
		if (isCW) *((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) |= 0x02;
		if (useZAxis) *((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) |= 0x04;
		
		*((volatile unsigned int*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x00)) = 
			*((volatile int*)(OBJ_199_ObjectAddress + OBJ_X_LOC_OFFSET));
		*((volatile unsigned int*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x04)) = 
			*((volatile int*)(OBJ_199_ObjectAddress + OBJ_Y_LOC_OFFSET));
	}
	else if (colour == OBJ_199_BLKCLR_GREEN)
	{
		OBJ_199_Axis axis = OBJ_199_Axis((byte)((param02 & 0xFF00) >> 8));
		byte nBlocks = (byte)(param02 & 0x00FF);
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) = 
			(byte)axis;
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x02)) = 
			(byte)nBlocks;
		
		*((volatile int*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x04)) = 
			*((volatile int*)(OBJ_199_ObjectAddress + OBJ_X_LOC_OFFSET + ((byte)axis * 4)));
		
		byte flipLimit = (byte)(param03 & 0x00FF);
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x00)) = 
			(byte)flipLimit;
		
		// Outline blocks aren't spawned until after the green block's constructor finishes 
		// to ensure that the green block updates its current index before the outline 
		// blocks read it
	}
	else if (colour == OBJ_199_BLKCLR_OUTLINE_GREEN)
	{
		byte blockIndex = (byte)(param02 & 0x00FF);
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) = blockIndex;
	}
	
	int textureID, paletteID;
	textureID = paletteID = (int)colour;
	
	int nMaterials = *((volatile int*)(OBJ_199_BMDAddress + 0x24));
	unsigned int offMaterials = 
		*((volatile unsigned int*)(OBJ_199_BMDAddress + 0x28));
	
	unsigned int adrMaterial = offMaterials;
	for (int i = 0; i < nMaterials; i++)
	{
		*((volatile int*)(adrMaterial + 0x04)) = textureID;
		*((volatile int*)(adrMaterial + 0x08)) = paletteID;
		adrMaterial += 48;
	}
	
	// Set param 02 and 03 values to zero as this object supports X and Z rotation
	*((volatile unsigned short int*)(OBJ_199_ObjectAddress + OBJ_X_ROT_OFFSET)) = 0x0000;
	*((volatile unsigned short int*)(OBJ_199_ObjectAddress + OBJ_Z_ROT_OFFSET)) = 0x0000;
}

// If green outline block, don't load a collision map
void repl_0213A7C0_ov_62()
{
	asm
	(
		// Original instruction
		"mov r0, r4										\t\n"
		
		"ldrb r3, [r4, #0x08]							\t\n"
		"cmp r3, #0x06									\t\n"
		"bne exit_0213A7C0								\t\n"
		
		"ldr r3, =OBJ_199_CollisionLoadSkipJumpAddress	\t\n"
		"ldr r3, [r3]									\t\n"
		"mov r15, r3									\t\n"
		
		"exit_0213A7C0:									\t\n"
	);
}

// Increase the collision detection bounds to 0.3 from 0.24 and 
// increase the draw distance from 4.096 to 12.288
void hook_0213A7DC_ov_62()
{
	*((volatile int*)(OBJ_199_ObjectAddress + OBJ_199_COLLISION_BOUNDS_OFFSET)) = 
		OBJ_199_COLLISION_BOUNDS_VALUE_RSHIFT3;
	
	*((volatile int*)(OBJ_199_ObjectAddress + OBJ_199_OBJ_DRAW_DISTANCE_OFFSET)) = 
		OBJ_199_DRAW_DISTANCE_VALUE;
}

// Don't update X, Y, Z speed values unless Pink so that the 12 bytes 
// may be used for other purposes
void hook_0213A148_ov_62()
{
	asm
	(
		"ldrb r2, [r5, #0x08]								\t\n"
		"cmp r2, #0x02										\t\n"
		"beq exit_0213A148									\t\n"
		"pop {r0-r12, r14}									\t\n"
		"ldr r2, =OBJ_199_SpeedValuesUpdateSkipJumpAddress	\t\n"
		"ldr r15, [r2]										\t\n"
	
		"exit_0213A148:										\t\n"
	);
}

// Add method address at +0x13C so that Mario's position is updated 
// if block moves whilst he's standing on it
void hook_0213A800_ov_62()
{
	asm
	(
		"ldr r2, =OBJ_199_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	);
	
	OBJ_199_BlockColour colour = 
		OBJ_199_BlockColour(*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_OBJ_PARAMETER_01_OFFSET)));
	
	if (colour == OBJ_199_BLKCLR_GREEN)
	{
		OBJ_199_Axis axis = 
			OBJ_199_Axis(*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)));
		
		if (axis != OBJ_199_AXS_Y)
		{
			return;
		}
	}
	
	*((volatile unsigned int*)(OBJ_199_ObjectAddress + UPDATE_MARIO_POS_OFFSET)) = 
		UPDATE_MARIO_POS_METHOD;
}

// Set onBlock to false by default, if colliding it'll be set in below 
// method (hook_0213A4D0_ov_62)
void hook_0213A488_ov_62()
{
	asm
	(
		"ldr r2, =OBJ_199_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	);
	
	// onBlock = false
	*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) &= 0xFE;
}

// Set whether the block is being stepped on
byte OBJ_199_OnBlock = 0x00;
void hook_0213A4A8_ov_62()
{
	asm
	(
		"ldr r2, =OBJ_199_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
		"ldr r2, =OBJ_199_OnBlock			\t\n"
		"str r0, [r2]						\t\n"
	);
	
	// onBlock = true
	if (OBJ_199_OnBlock)
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) |= 0x01;
	
	OBJ_199_BlockColour colour = 
		OBJ_199_BlockColour(*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_OBJ_PARAMETER_01_OFFSET)));
	
	if (colour == OBJ_199_BLKCLR_PINK)
	{
		if (OBJ_199_OnBlock)
		{		
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) |= 0x02;
		}
		
		bool steppedOn = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) & 0x02;
		
		if (steppedOn)
		{
			asm
			(
				"pop {r0}		\t\n"
				"mov r0, #0x01	\t\n"
				"push {r0}		\t\n"
				"bx r14			\t\n"
			);
		}
	}
}

// Disable original behaviour unless pink
void hook_0213A4D0_ov_62()
{
	asm
	(
		"ldr r2, =OBJ_199_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	);
	
	OBJ_199_BlockColour colour = 
		OBJ_199_BlockColour(*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_OBJ_PARAMETER_01_OFFSET)));
	
	if (colour == OBJ_199_BLKCLR_PINK)
	{
		byte safetyDelayCounter = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01));
		
		if (safetyDelayCounter <= 0)
		{
			// Reset settings
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) = 
				*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x02));
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) &= 0xFD;
			
			// Set its state to 'start collapse'
			asm
			(
				"pop {r0}		\t\n"
				"mov r0, #0x01	\t\n"
				"push {r0}		\t\n"
			);
			return;
		}
	}

	asm
	(
		"pop {r0}		\t\n"
		"mov r0, #0x00	\t\n"
		"push {r0}		\t\n"
	);
}

// New behaviours
int OBJ_199_SinRot = 0x00000000;
int OBJ_199_CosRot = 0x00000000;
short int OBJ_199_LastRotAngle = 0x00;
void hook_0213A380_ov_62()
{
	asm
	(
		"ldr r2, =OBJ_199_ObjectAddress		\t\n"
		"str r4, [r2]						\t\n"
	);
	
	bool onBlock = 
		*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) & 0x01;
	
	OBJ_199_BlockColour colour = 
		OBJ_199_BlockColour(*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_OBJ_PARAMETER_01_OFFSET)));
	
	if (colour == OBJ_199_BLKCLR_LIGHT_BLUE)
	{
		byte rangeCount = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01));
		byte rangeLimit = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x02));
		byte speedShift = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x03));
		int movementSpeed = (1 << speedShift);
		
		if (onBlock)
		{
			if (rangeCount < rangeLimit)
			{
				// Move upwards
				*((volatile int*)(OBJ_199_ObjectAddress + OBJ_Y_LOC_OFFSET)) += 
					movementSpeed;
				// Range count ++
				*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) += 0x01;
			}
		}
		else
		{
			if (rangeCount > 0)
			{
				// Move downwards
				*((volatile int*)(OBJ_199_ObjectAddress + OBJ_Y_LOC_OFFSET)) -= 
					movementSpeed;
				// Range count --
				*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) -= 0x01;
			}
		}
	}
	else if (colour == OBJ_199_BLKCLR_DARK_BLUE)
	{
		byte rangeCount = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01));
		byte rangeLimit = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x02));
		byte speedShift = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x03));
		int movementSpeed = (1 << speedShift);
			
		if (!onBlock)
		{
			if (rangeCount > 0)
			{
				// Move upwards
				*((volatile int*)(OBJ_199_ObjectAddress + OBJ_Y_LOC_OFFSET)) += 
					movementSpeed;
				// Range count --
				*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) -= 0x01;
			}
		}
		else
		{
			if (rangeCount < rangeLimit)
			{
				// Move downwards
				*((volatile int*)(OBJ_199_ObjectAddress + OBJ_Y_LOC_OFFSET)) -= 
					movementSpeed;
				// Range count ++
				*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) += 0x01;
			}
		}
	}
	else if (colour == OBJ_199_BLKCLR_PINK)
	{
		bool steppedOn = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) & 0x02;
		
		if (steppedOn)
		{
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) -= 0x01;
		}
	}
	else if (colour == OBJ_199_BLKCLR_ORANGE)
	{
		if (onBlock)
		{
			byte rangeCount = 
				*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01));
			byte rangeLimit = 
				*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x02));
			byte speedShift = 
				*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x03));
			OBJ_199_Axis axis = 
				OBJ_199_Axis(*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x00)));
			int movementSpeed = (1 << speedShift);
			
			if (rangeCount >= rangeLimit)
			{
				// reset range count
				*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) = 0x00;
				// flip direction
				*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) ^= 0x80;
			}
			else
			{
				*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)) += 0x01;
			}
			
			bool dir = 
				*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) & 0x80;
			int posIncrement = (dir) ? movementSpeed : (0 - movementSpeed);
			*((volatile int*)(OBJ_199_ObjectAddress + OBJ_X_LOC_OFFSET + ((byte)axis * 4))) += posIncrement;
		}
	}
	else if (colour == OBJ_199_BLKCLR_PURPLE)
	{
		byte rotSpeed = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01));
		byte pathRadius = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x02));
		
		bool isCW = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) & 0x02;
		bool useZAxis = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) & 0x04;
		
		int startingXPos = 
			*((volatile int*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x00));
		int startingYPos = 
			*((volatile int*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x04));
		
		// Move in a circle (see CT_MECHA_07.cpp)
		short int currentRot = *((volatile short int*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x08));
		currentRot += rotSpeed;
		*((volatile short int*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x08)) = currentRot;

		if (OBJ_199_LastRotAngle != rotSpeed)
		{
			OBJ_199_SinRot = sinLerp(currentRot);
			OBJ_199_CosRot = cosLerp(currentRot);
		}
		
		int xLocNumBlocks = (OBJ_199_CosRot * pathRadius);
		int xLoc = xLocNumBlocks * (0 - (OBJ_199_BLOCK_SIZE / 4096));
		if (isCW) xLoc = 0 - xLoc;
		int yLocNumBlocks = (OBJ_199_SinRot * pathRadius);
		int yLoc = yLocNumBlocks * (OBJ_199_BLOCK_SIZE / 4096);
		
		if (!useZAxis) *((volatile int*)(OBJ_199_ObjectAddress + OBJ_X_LOC_OFFSET)) = startingXPos + xLoc;
		else *((volatile int*)(OBJ_199_ObjectAddress + OBJ_Z_LOC_OFFSET)) = startingXPos + xLoc;
		*((volatile int*)(OBJ_199_ObjectAddress + OBJ_Y_LOC_OFFSET)) = startingYPos + yLoc;
		
		OBJ_199_LastRotAngle = rotSpeed;
	}
	else if (colour == OBJ_199_BLKCLR_GREEN)
	{
		OBJ_199_Axis axis = 
			OBJ_199_Axis(*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)));
		byte nBlocks = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x02));
		byte currentBlock = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x03));
		
		byte flipLimit = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x00));
		byte flipTimer = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x01));
		
		int startingLocation = 
			*((volatile int*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x04));
		
		bool spawnedOutlineBlocks = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) & 0x02;
		if (!spawnedOutlineBlocks)
		{
			unsigned int old_OBJ_199_ObjectAddress = OBJ_199_ObjectAddress;
			
			for (int i = 1; i < nBlocks; i++)
			{
				SpawnOutlineGreenBlock(OBJ_199_ObjectAddress, startingLocation, axis, i);
				OBJ_199_ObjectAddress = old_OBJ_199_ObjectAddress; // because SpawnActor calls ctor.
			}
			
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET)) |= 0x02;
		}
		
		if (flipTimer == flipLimit)
		{
			currentBlock++;
			if (currentBlock >= nBlocks) currentBlock = 0;
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x03)) = currentBlock;
			
			int currentPosition = startingLocation + (currentBlock * OBJ_199_BLOCK_SIZE_OUTLINE_PADDING);
			*((volatile int*)(OBJ_199_ObjectAddress + OBJ_X_LOC_OFFSET + ((byte)axis * 4))) = currentPosition;
			
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x01)) = 0x00;
		}
		else
		{
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x01)) += 0x01;
		}
	}
	else if (colour == OBJ_199_BLKCLR_OUTLINE_GREEN)
	{
		unsigned int parentAddress = 
			*((volatile unsigned int*)(OBJ_199_ObjectAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x00));
		byte blockIndex = 
			*((volatile byte*)(OBJ_199_ObjectAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01));
		
		OBJ_199_Axis axis = 
			OBJ_199_Axis(*((volatile byte*)(parentAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x01)));
		//byte nBlocks = *((volatile byte*)(parentAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x02));
		byte parentCurrentBlock = 
			*((volatile byte*)(parentAddress + OBJ_199_FREE_FOUR_BYTES_OFFSET + 0x03));
		
		int parentStartPosition = 
			*((volatile int*)(parentAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET + 0x04));
		
		// If parent block = block index: Position = parent's starting position which is now empty
		// Else: // Position = parent start pos. + (blockIndex * BLOCK_SIZE)
		int position = parentStartPosition;
		if (parentCurrentBlock != blockIndex)
		{
			position += (int)((int)blockIndex * OBJ_199_BLOCK_SIZE_OUTLINE_PADDING);
		}
		*((volatile int*)(OBJ_199_ObjectAddress + OBJ_X_LOC_OFFSET + ((byte)axis * 4))) = position;
	}
}

void SpawnOutlineGreenBlock(unsigned int parentAddress, int startingLocation, 
	OBJ_199_Axis axis, int blockIndex)
{
	int posOffset = startingLocation + (blockIndex * OBJ_199_BLOCK_SIZE_OUTLINE_PADDING);
	
	int pos[] = { *((volatile int*)(parentAddress + OBJ_X_LOC_OFFSET)), 
				  *((volatile int*)(parentAddress + OBJ_Y_LOC_OFFSET)), 
				  *((volatile int*)(parentAddress + OBJ_Z_LOC_OFFSET)) };
	pos[(byte)axis] = posOffset;
	short int param02 = (short int)blockIndex;
	short int rot[] = { param02, 0x0000, 0x0000 };
	
	unsigned int outlineBlockAddress = SpawnActor(OBJ_199_ACTOR_ID, 0x0006, pos, rot);
	
	*((volatile unsigned int*)(outlineBlockAddress + OBJ_199_FREE_TWELVE_BYTES_OFFSET)) = 
		parentAddress;
}

// Change particle effect displayed when exploding to big dust clouds 
void repl_0213A1CC_ov_62()
{
	asm
	(
		"mov r0, #0x07		\t\n"
	);
}


