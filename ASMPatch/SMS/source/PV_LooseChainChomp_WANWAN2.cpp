#include "PiantaVillage.h"

const unsigned int OBJ_048_NEXT_NODE_POINTER_ADDRESS = 0x020A0D88;
short int OBJ_048_MarioPositionAsObjectCoord[3] = { 0, 0, 0 }; 



/* 
// Set the pointer to the first path node to be Mario's position
void hook_021440AC_ov_64()
{
	*((volatile unsigned int*)(OBJ_048_NEXT_NODE_POINTER_ADDRESS)) = (unsigned int)(&OBJ_048_MarioPositionAsObjectCoord[0]);
}

// Next path node is always at index zero
void repl_02143F90_ov_64()
{
	asm 
	(
		"mov r2, #0x00		\t\n"
	);
}

// Update next node's position to be same as Mario's to make Chain Chomp 
// follow Mario
// Mario's position is co-ordinate * 1000 * 4096, 
// object positions are co-ordinate * 1000, 
// to convert to object co-ordinate, /4096 (>>12)
void hook_02143F98_ov_64()
{
	OBJ_048_MarioPositionAsObjectCoord[0] = (short int)(PLAYER->xPos >> 0x0C);
	OBJ_048_MarioPositionAsObjectCoord[1] = (short int)(PLAYER->yPos >> 0x0C);
	OBJ_048_MarioPositionAsObjectCoord[2] = (short int)(PLAYER->zPos >> 0x0C);
}
*/
