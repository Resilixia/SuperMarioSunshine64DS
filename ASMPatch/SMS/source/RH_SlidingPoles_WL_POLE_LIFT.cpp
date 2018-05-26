#include "RiccoHarbour.h"

unsigned int OBJ_105_FROZEN_JUMP_ADDRESS = 0x0211140C;

// Ensure that object is always loaded not based on Star count(?)
void repl_021115C8_ov_1A()
{
	asm
	(
		"ands r0, r0	\t\n"
	);
}

// Only move if the cranes in Ricco Harbour have been activated
void repl_02111404_ov_1A()
{
	asm
	(
		"ldr r2, =RH_AreCranesActivated			\t\n"
		"ldrb r2, [r2]							\t\n"
		"cmp r2, #0x01							\t\n"
		"bge exit_02111404						\t\n"
		"mov r2, r0								\t\n" // Original instruction 
		"ldr r0, =OBJ_105_FROZEN_JUMP_ADDRESS	\t\n"
		"ldr r15, [r0]							\t\n"
		
		"exit_02111404:							\t\n"
		"mov r2, r0								\t\n" // Original instruction 
		"bx r14									\t\n"
	);
}
