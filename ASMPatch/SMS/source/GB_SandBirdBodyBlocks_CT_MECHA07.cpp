#include "SM64DS.h"

/* From: https://stackoverflow.com/questions/16941094/how-to-calculate-x-and-y-coordinates-of-a-rotated-3d-cube-in-javascript
 * 
 * θ and δ the angles defining the rotation you want to apply, resp. the pitch angle and the yaw angle
 * 
 * x_0, y_0 and z_0 give the starting location of the block
 * 
 * x = sin(δ) * y_0 + cos(δ) * x_0
 * y = sin(θ) * z_0 + cos(θ) * (cos(δ) * y_0 − sin(δ) * x_0)
 * z = cos(θ) * z_0 + sin(θ) * (cos(δ) * y_0 − sin(δ) * x_0)
 *
 * Here, θ = 0, δ = the Y rotation increment.
 * We aren't concerned with the Z (up) position so we can use:
 * 
 * x = sin(δ) * y_0 + cos(δ) * x_0
 * y = cos(δ) * y_0 − sin(δ) * x_0
 */
 
/* From: http://board.flashkit.com/board/showthread.php?547056-making-an-object-move-in-a-circle
 *
 * Basic circular motion is given by the following formula:
 * 
 * x = cx + Math.sin(time)*rad;
 * y = cy + Math.cos(time)*rad;
 * 
 * cx, cy are the centre of the circle and
 * 
 * rad is the radius of the circle.
 * 
 * As time goes from 0 to 2*PI (or multiples thereof), the x,y values will describe a complete circle.
 */


int const OBJ_093_BLOCK_SIZE = 1310720; // 0.32

// IMPORTANT: Object Rotation = ((Value in degrees / 22.5) * 4096) or (2 * BRadians value)

// 		 Y
//       ^
//		 |
// X <---0
// _____________
// | H | E | J |
// |___|___|___|	H(0.32,0.32)   E(0,0.32) J(-0.32,-0.32)
// | D | A | B |
// |___|___|___|	D(0.32,0)	   A(0,0)	B(-0.32,0)
// | G | C | F |
// |___|___|___|	G(0.32,-0.32)  C(0,-0.32)F(-0.32,-0.32)
//     | L | K |			
//     |___|___|	K(-0.32,-0.64) L(0,-0.64)
// A Move = (0,0)
// B Move = (-1 * X_AXIS_POS_MOVE_X, -1 * X_AXIS_POS_MOVE_Z)
// C Move = (-1 * Y_AXIS_POS_MOVE_X, -1 * Y_AXIS_POS_MOVE_Z)
// D Move = (X_AXIS_POS_MOVE_X, X_AXIS_POS_MOVE_Z)
// E Move = (Y_AXIS_POS_MOVE_X, Y_AXIS_POS_MOVE_Z)
// F Move = (-1 * DIAG_POS_POS_MOVE_X, -1 * DIAG_POS_POS_MOVE_Z)
// G Move = (DIAG_POS_NEG_MOVE_X, DIAG_POS_NEG_MOVE_Z)
// H Move = (DIAG_POS_POS_MOVE_X, DIAG_POS_POS_MOVE_Z)
// J Move = (-1 * DIAG_POS_NEG_MOVE_X, -1 * DIAG_POS_NEG_MOVE_Z)
// K Move = F Move + (-1 * Y_AXIS_POS_MOVE_X, -1 * Y_AXIS_POS_MOVE_Z)
// L Move = (2 * (-1 * Y_AXIS_POS_MOVE_X), 2 * (-1 * Y_AXIS_POS_MOVE_Z))
short int const BRADIANS_INCREMENT = degreesToAngle(1) / 8;
int X_AXIS_POS_MOVE_X = 0; // Move X value for B.X = (A.X + 1), multiply by -1 for B.X = (A.X - 1)
int X_AXIS_POS_MOVE_Z = 0; // Move Z value for B.X = (A.X + 1), multiply by -1 for B.X = (A.X - 1)
int Y_AXIS_POS_MOVE_X = 0; // Move X value for B.Z = (A.Z + 1), multiply by -1 for B.Z = (A.Z - 1)
int Y_AXIS_POS_MOVE_Z = 0; // Move Z value for B.Z = (A.Z + 1), multiply by -1 for B.Z = (A.Z - 1)
int DIAG_POS_POS_MOVE_X = 0; // Multiply by -1 for NEG NEG
int DIAG_POS_POS_MOVE_Z = 0; // Multiply by -1 for NEG NEG
int DIAG_POS_NEG_MOVE_X = 0; // Multiply by -1 for NEG POS
int DIAG_POS_NEG_MOVE_Z = 0; // Multiply by -1 for NEG POS
short int OBJ_093_CurrentRotation = 0;


// Parameters:
// 
// CT_MECHA07 (093)
// 		Parameter 1: 
// 		Byte (1 = Rightmost bit)	Desc.
// 		1							Controller: Number of blocks if this is the controller object
// 									Controlled block: 0 - false, 1 - true: whether this block is to be part of Sand Bird
// 		2							0 - false, 1 - true: whether this is the controller object
//		Eg. If controller: 		0x0137: 0x01: is controller, 0x37: 57 blocks
// 			If not controller: 	0x0001: is part of Sand Bird
// 		
// 		Parameter 2:
//		Byte						Desc.
//		1							Controller: Radius of circular path in blocks
// 									Controlled block: 0 - false, 1 - true: whether this is the head block
// 		2							Controller: Climb height in blocks
// 		Eg. If controller: 		0x1814: Climb height of 24 blocs, radius of 20 blocks
//
// CT_MECHA05 (091)
// 		Byte (1 = Rightmost bit)	Desc.
// 		1							0 - false, 1 - true: whether this is the head block

short int const OBJ_093_OBJ_PARAMETER_01_OFFSET = 0x08;
short int const OBJ_093_OBJ_PARAMETER_02_OFFSET = 0x92;
short int const OBJ_093_OBJ_PARAMETER_03_OFFSET = 0x90;

short int const OBJ_091_OBJ_PARAMETER_01_OFFSET = 0x08;
short int const OBJ_091_OBJ_PARAMETER_02_OFFSET = 0x92;
short int const OBJ_091_OBJ_PARAMETER_03_OFFSET = 0x90;

short int const OBJ_091_COLLISION_HEIGHT_OFFSET = 0x130;
short int const OBJ_091_COLLISION_WIDTH_OFFSET = 0x134;

// OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET:
// Offset 	Size	Desc.
// 0x00		1		Bit 1: 0 - false, 1 - true: whether this is the controller object
// 					Bit 2: 0 - false, 1 - true: whether this object has been initialised
// 					Bit 3: 0 - false, 1 - true: whether this block is part of wings
// 					Bit 4: 0 - false, 1 - true: whether this block is part of tail
// 					Bit 5: 0 - false, 1 - true: whether this block is part of neck
// 					Bit 6: 0 - false, 1 - true: whether this block is the head block
// 0x01		1		Bit 1: 0 - false, 1 - true: whether Block.x - Controller.x is positive
// 					Bit 2: 0 - false, 1 - true: whether Block.z - Controller.z is positive
// 0x02 	1		Difference in number of blocks between Block.x and Controller.x (always positive)
// 0x03		1		Difference in number of blocks between Block.z and Controller.z (always positive)
// 0x04		2		Multiplier for maximum number of steps and number of increments if part of wings or tail

short int const OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET = 0xA0;

byte const OBJ_093_NUM_BLOCKS = 57;

int const OBJ_093_WING_STEP_MAX_VALUE = 32768; // 0.008
int const OBJ_093_WING_MOVEMENT_NUM_FRAMES = 32;
int const OBJ_093_WING_TOTAL_NUM_FRAMES = OBJ_093_WING_MOVEMENT_NUM_FRAMES * 4;
int const OBJ_093_WING_INCREMENT_VALUE = OBJ_093_WING_STEP_MAX_VALUE / OBJ_093_WING_MOVEMENT_NUM_FRAMES;
int const OBJ_093_TAIL_STEP_MAX_VALUE = 16384; // 0.004
int const OBJ_093_TAIL_INCREMENT_VALUE = OBJ_093_TAIL_STEP_MAX_VALUE / OBJ_093_WING_MOVEMENT_NUM_FRAMES;
int const OBJ_093_NECK_STEP_MAX_VALUE = -8192; // -0.002
int const OBJ_093_NECK_INCREMENT_VALUE = OBJ_093_NECK_STEP_MAX_VALUE / OBJ_093_WING_MOVEMENT_NUM_FRAMES;

unsigned int OBJ_093_ObjectAddress = 0xFFFFFFFF;
unsigned int OBJ_091_ObjectAddress = 0xFFFFFFFF;

byte OBJ_093_NumBlocksInitialised = 0;
bool OBJ_093_ControllerInitialised = false;

int OBJ_093_ControllerLocationX = 0x00000000;
int OBJ_093_ControllerLocationY = 0x00000000;
int OBJ_093_ControllerLocationZ = 0x00000000;

short int OBJ_093_CurrentFrame = 0;

bool OBJ_093_Climbing = true;
int OBJ_093_ClimbHeight = 0;
int OBJ_093_ClimbStart = 0;
// Climb increment should be (12.8 / (360 / rotation increment)) to ensure climbing for exactly one loop
int const OBJ_093_CLIMB_INCREMENT_VALUE = 18204; // ~0.00444444444444444444444444444444

//////////////////// CT_MECHA07 (093) ////////////////////

// Disable the original Y Rotation behaviour
void repl_0211B5E4_ov_41() { }
void repl_0211B5E8_ov_41() { }
void repl_0211B5EC_ov_41() { }

void hook_0211B5DC_ov_41()
{
	asm
	(
		"ldr r0, =OBJ_093_ObjectAddress		\t\n"
		"str r5, [r0]						\t\n"
	);
	
	short int param01 = *((volatile short int*)(OBJ_093_ObjectAddress + OBJ_093_OBJ_PARAMETER_01_OFFSET));
	short int param02 = *((volatile short int*)(OBJ_093_ObjectAddress + OBJ_093_OBJ_PARAMETER_02_OFFSET));
	//short int param03 = *((volatile short int*)(OBJ_093_ObjectAddress + OBJ_093_OBJ_PARAMETER_03_OFFSET));
	
	bool isController = (param01 >> 8 == 0x01);
	byte numBlocks = param01 & 0x00FF;
	byte radius = (isController) ? (param02 & 0x00FF) : -1;
	byte climbHeight = (isController) ? ((param02 & 0xFF00) >> 8) : -1;
	
	// If block not part of Sand Bird, return
	if (!isController && numBlocks != 0x01) return;
	
	bool initialised = *((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET)) & 0x01;
	if (!initialised)
	{
		if (OBJ_093_NumBlocksInitialised + 1 > OBJ_093_NUM_BLOCKS) OBJ_093_NumBlocksInitialised = 0;
		
		*((volatile long*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET)) = 0x00000000;
		
		if (isController)
		{
			*((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x03)) = (byte)radius;
			
			OBJ_093_ControllerLocationX = *((volatile int*)(OBJ_093_ObjectAddress + OBJ_X_LOC_OFFSET));
			OBJ_093_ControllerLocationY = *((volatile int*)(OBJ_093_ObjectAddress + OBJ_Y_LOC_OFFSET));
			OBJ_093_ControllerLocationZ = *((volatile int*)(OBJ_093_ObjectAddress + OBJ_Z_LOC_OFFSET));
			
			OBJ_093_CurrentRotation = 0;
			
			OBJ_093_CurrentFrame = 0;
			
			OBJ_093_Climbing = true;
			OBJ_093_ClimbStart = OBJ_093_ControllerLocationY;
			OBJ_093_ClimbHeight = (climbHeight * OBJ_093_BLOCK_SIZE) + OBJ_093_ControllerLocationY;
			
			OBJ_093_NumBlocksInitialised++;
			
			OBJ_093_ControllerInitialised = true;
		}
		else
		{
			if (!OBJ_093_ControllerInitialised) return; // Wait for controller to be initialised first
			
			*((volatile long*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET)) = 0x00000000;
			
			if (OBJ_093_NumBlocksInitialised + 1 > numBlocks) OBJ_093_NumBlocksInitialised = 0;
			
			int diffX = *((volatile int*)(OBJ_093_ObjectAddress + OBJ_X_LOC_OFFSET)) - OBJ_093_ControllerLocationX;
			int diffZ = *((volatile int*)(OBJ_093_ObjectAddress + OBJ_Z_LOC_OFFSET)) - OBJ_093_ControllerLocationZ;
			
			int numBlocksDiffX = 0;
			int numBlocksDiffZ = 0;
			int tmpX = (diffX >= 0) ? (diffX) : (0 - diffX);
			int tmpZ = (diffZ >= 0) ? (diffZ) : (0 - diffZ);
			while (tmpX > 0)
			{
				numBlocksDiffX++;
				tmpX -= OBJ_093_BLOCK_SIZE;
			}
			while (tmpZ > 0)
			{
				numBlocksDiffZ++;
				tmpZ -= OBJ_093_BLOCK_SIZE;
			}
			
			bool posXAxis = (diffX > 0);
			bool posYAxis = (diffZ > 0);
			
			if (posXAxis) *((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x01)) |= 0x01;
			if (posYAxis) *((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x01)) |= 0x02;
			
			*((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x02)) = (byte)numBlocksDiffX;
			*((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x03)) = (byte)numBlocksDiffZ;
			
			if (numBlocksDiffX > 1) // Wings
			{
				short int stepMultiplierY = numBlocksDiffX * numBlocksDiffX;
				*((volatile short int*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x04)) = stepMultiplierY;
				*((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET)) |= 0x04;
			}
			else if (numBlocksDiffZ > 1)
			{
				short int stepMultiplierY = numBlocksDiffZ * numBlocksDiffZ;
				*((volatile short int*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x04)) = stepMultiplierY;
				
				if (!posYAxis) // Tail
				{
					*((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET)) |= 0x08;
				}
				else // Neck/Head
				{
					*((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET)) |= 0x10;
				}
			}
			
			bool isHead = (!isController) ? ((param02 & 0xFF) == 0x01) : false;
			if (isHead) *((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET)) |= 0x20;
			
			OBJ_093_NumBlocksInitialised++;
		}
		
		*((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET)) |= 0x01;
		
		//Return here so every block gets a chance to initialise before starting to rotate and translate them
		if (OBJ_093_NumBlocksInitialised < OBJ_093_NUM_BLOCKS) return; 
	}
	
	OBJ_093_ControllerInitialised = false; // To allow re-entry into level
	
	*((volatile short int*)(OBJ_093_ObjectAddress + OBJ_Y_ROT_OFFSET)) = OBJ_093_CurrentRotation * 2;
	
	if (isController)
	{
		int sinRot = sinLerp(OBJ_093_CurrentRotation);
		int cosRot = cosLerp(OBJ_093_CurrentRotation);
		
		int xLocNumBlocks = (cosRot * radius);
		int xLoc = xLocNumBlocks * (0 - (OBJ_093_BLOCK_SIZE / 4096));
		int zLocNumBlocks = (sinRot * radius);
		int zLoc = zLocNumBlocks * (OBJ_093_BLOCK_SIZE / 4096);
		
		*((volatile int*)(OBJ_093_ObjectAddress + OBJ_X_LOC_OFFSET)) = xLoc;
		OBJ_093_ControllerLocationX = xLoc;
		*((volatile int*)(OBJ_093_ObjectAddress + OBJ_Z_LOC_OFFSET)) = zLoc;
		OBJ_093_ControllerLocationZ = zLoc;
		
		X_AXIS_POS_MOVE_X = (sinRot * 0 + cosRot * OBJ_093_BLOCK_SIZE) / 4096;
		X_AXIS_POS_MOVE_Z = (cosRot * 0 - sinRot * OBJ_093_BLOCK_SIZE) / 4096;
		Y_AXIS_POS_MOVE_X = (sinRot * OBJ_093_BLOCK_SIZE + cosRot * 0) / 4096;
		Y_AXIS_POS_MOVE_Z = (cosRot * OBJ_093_BLOCK_SIZE - sinRot * 0) / 4096;
		DIAG_POS_POS_MOVE_X = (sinRot * OBJ_093_BLOCK_SIZE + cosRot * OBJ_093_BLOCK_SIZE) / 4096;
		DIAG_POS_POS_MOVE_Z = (cosRot * OBJ_093_BLOCK_SIZE - sinRot * OBJ_093_BLOCK_SIZE) / 4096;
		DIAG_POS_NEG_MOVE_X = (sinRot * (0 - OBJ_093_BLOCK_SIZE) + cosRot * OBJ_093_BLOCK_SIZE) / 4096;
		DIAG_POS_NEG_MOVE_Z = (cosRot * (0 - OBJ_093_BLOCK_SIZE) - sinRot * OBJ_093_BLOCK_SIZE) / 4096;
		
		OBJ_093_CurrentRotation = (OBJ_093_CurrentRotation + BRADIANS_INCREMENT) % DEGREES_IN_CIRCLE;
		
		OBJ_093_CurrentFrame = (OBJ_093_CurrentFrame + 1) % OBJ_093_WING_TOTAL_NUM_FRAMES;
		
		// Climbing/descending
		int currentLocY = *((volatile int*)(OBJ_093_ObjectAddress + OBJ_Y_LOC_OFFSET));
		if (currentLocY >= OBJ_093_ClimbHeight) OBJ_093_Climbing = false;
		if (currentLocY <= OBJ_093_ClimbStart) OBJ_093_Climbing = true;
		
		int yLoc = currentLocY + ((OBJ_093_Climbing) ? (OBJ_093_CLIMB_INCREMENT_VALUE) : (0 - OBJ_093_CLIMB_INCREMENT_VALUE));
		*((volatile int*)(OBJ_093_ObjectAddress + OBJ_Y_LOC_OFFSET)) = yLoc;
		OBJ_093_ControllerLocationY = yLoc;
	}
	else
	{
		byte numBlocksDiffX = *((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x02));
		byte numBlocksDiffZ = *((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x03));
		bool posXAxis = *((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x01)) & 0x01;
		bool posYAxis = *((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x01)) & 0x02;
		
		int moveX = 0, moveZ = 0;
		byte x = numBlocksDiffX, z = numBlocksDiffZ;
		byte diagX = 0, diagZ = 0;
		while ( (diagX + 1 <= x) && (diagZ + 1 <= z) ) // Diagonal from controller
		{
			if ( (posXAxis && posYAxis) || (!posXAxis && !posYAxis) ) // POS POS or NEG NEG
			{
				moveX += (posXAxis) ? (DIAG_POS_POS_MOVE_X) : (0 - DIAG_POS_POS_MOVE_X);
				moveZ += (posXAxis) ? (DIAG_POS_POS_MOVE_Z) : (0 - DIAG_POS_POS_MOVE_Z);
			}
			else if ( (posXAxis && !posYAxis) || (!posXAxis && posYAxis) ) // POS NEG or NEG POS
			{
				moveX += (posXAxis) ? (DIAG_POS_NEG_MOVE_X) : (0 - DIAG_POS_NEG_MOVE_X);
				moveZ += (posXAxis) ? (DIAG_POS_NEG_MOVE_Z) : (0 - DIAG_POS_NEG_MOVE_Z);
			}
			
			diagX++;
			diagZ++;
		}
		x -= diagX;
		z -= diagZ;
		if (x > z) // Horizontal from controller
		{
			moveX += (posXAxis) ? (x * X_AXIS_POS_MOVE_X) : (x * (0 - X_AXIS_POS_MOVE_X));
			moveZ += (posXAxis) ? (x * X_AXIS_POS_MOVE_Z) : (x * (0 - X_AXIS_POS_MOVE_Z));
		}
		else if (z > x) // Vertical from controller
		{
			moveX += (posYAxis) ? (z * Y_AXIS_POS_MOVE_X) : (z * (0 - Y_AXIS_POS_MOVE_X));
			moveZ += (posYAxis) ? (z * Y_AXIS_POS_MOVE_Z) : (z * (0 - Y_AXIS_POS_MOVE_Z));
		}
		
		*((volatile int*)(OBJ_093_ObjectAddress + OBJ_X_LOC_OFFSET)) = OBJ_093_ControllerLocationX + moveX;
		*((volatile int*)(OBJ_093_ObjectAddress + OBJ_Z_LOC_OFFSET)) = OBJ_093_ControllerLocationZ + moveZ;
		
		bool wing = *((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET)) & 0x04;
		bool tail = *((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET)) & 0x08;
		bool neck = *((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET)) & 0x18;
		short int stepMultiplierY = *((volatile short int*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET + 0x04));
		int moveY = 0;
		
		if (wing || tail || neck)
		{
			bool firstHalf = (OBJ_093_CurrentFrame < (OBJ_093_WING_MOVEMENT_NUM_FRAMES * 2));
			short int frameMultiplier = OBJ_093_CurrentFrame % OBJ_093_WING_MOVEMENT_NUM_FRAMES;
			int incrementValue = (wing) ? 
				(OBJ_093_WING_INCREMENT_VALUE) : ((tail) ? (OBJ_093_TAIL_INCREMENT_VALUE) : (OBJ_093_NECK_INCREMENT_VALUE));
			
			if (firstHalf)
			{
				bool up = (OBJ_093_CurrentFrame < OBJ_093_WING_MOVEMENT_NUM_FRAMES);
				
				if (up)
				{
					moveY += (stepMultiplierY * incrementValue * frameMultiplier);
				}
				else
				{
					int maxY = (stepMultiplierY * incrementValue * OBJ_093_WING_MOVEMENT_NUM_FRAMES);
					moveY = maxY + (stepMultiplierY * incrementValue * (0 - frameMultiplier));
				}
			}
			else
			{
				bool down = (OBJ_093_CurrentFrame < (OBJ_093_WING_MOVEMENT_NUM_FRAMES * 3));
				
				if (down)
				{
					moveY += (stepMultiplierY * incrementValue * (0 - frameMultiplier));
				}
				else
				{
					int minY = (stepMultiplierY * incrementValue * (0 - OBJ_093_WING_MOVEMENT_NUM_FRAMES));
					moveY = minY + (stepMultiplierY * incrementValue * frameMultiplier);
				}
			}
		}
		
		*((volatile int*)(OBJ_093_ObjectAddress + OBJ_Y_LOC_OFFSET)) = OBJ_093_ControllerLocationY + moveY;
		
		bool head = *((volatile byte*)(OBJ_093_ObjectAddress + OBJ_093_FREE_EIGHT_BYTES_OBJECT_OFFSET)) & 0x20;
		if (head)
		{
			*((volatile short int*)(OBJ_091_ObjectAddress + OBJ_Y_ROT_OFFSET)) = *((volatile short int*)(OBJ_093_ObjectAddress + OBJ_Y_ROT_OFFSET));
			*((volatile int*)(OBJ_091_ObjectAddress + OBJ_X_LOC_OFFSET)) = *((volatile int*)(OBJ_093_ObjectAddress + OBJ_X_LOC_OFFSET));
			*((volatile int*)(OBJ_091_ObjectAddress + OBJ_Y_LOC_OFFSET)) = *((volatile int*)(OBJ_093_ObjectAddress + OBJ_Y_LOC_OFFSET));
			*((volatile int*)(OBJ_091_ObjectAddress + OBJ_Z_LOC_OFFSET)) = *((volatile int*)(OBJ_093_ObjectAddress + OBJ_Z_LOC_OFFSET));
		}
	}
	
}

//////////////////// CT_MECHA05 (091) ////////////////////

// If this is the head object, store its address and set the X speed to zero
void hook_0211B274_ov_41()
{
	asm
	(
		"ldr r0, =OBJ_091_ObjectAddress		\t\n"
		"str r5, [r0]						\t\n"
	);
	
	short int param01 = *((volatile byte*)(OBJ_091_ObjectAddress + OBJ_091_OBJ_PARAMETER_01_OFFSET));
	bool head = ((param01 & 0xFF) == 0x01);
	
	if (!head)
	{
		OBJ_091_ObjectAddress = 0xFFFFFFFF;
		return;
	}
	
	*((volatile int*)(OBJ_091_ObjectAddress + OBJ_X_SPEED_OFFSET)) = 0;
	
	// Increase collision detection boundary, part 1
	*((volatile int*)(OBJ_091_ObjectAddress + OBJ_091_COLLISION_HEIGHT_OFFSET)) = 0x00600000;
	*((volatile int*)(OBJ_091_ObjectAddress + OBJ_091_COLLISION_WIDTH_OFFSET)) = 0xFFFFFFFF;
}

// Set X speed to zero, gets called every sliding cycle
void repl_0211B0E8_ov_41()
{
	asm
	(
		"mov r0, #0x00		\t\n"
	);
}

// Increase collision detection boundary, part 2
void repl_0211B184_ov_41()
{
	asm
	(
		"mov r1, #0x600000		\t\n"
	);
}
