#include "GelatoBeach.h"

unsigned int OBJ_281_ObjectAddress = 0xFFFFFFFF;

void hook_0211A1FC_ov_3E()
{
	asm
	(
		"ldr r1, =OBJ_281_ObjectAddress		\t\n"
		"str r0, [r1]						\t\n"
	);
}

// Increase the running speed 1/2
void repl_0211A480_ov_3E()
{
	asm
	(
		"mov r1, #0x017000	\t\n"
	);
}

// Increase the running speed 2/2 
void repl_0211A474_ov_3E()
{
	asm
	(
		"mov r2, #0x0800	\t\n"
	);
}

// Disable Wait 2 state when talking to Mario before race
void repl_0211A7A8_ov_3E() { }

// Disable Wait 2 state when talking to Mario after race 
void repl_02119CE4_ov_3E() { }

// Disable Wait 2 state when talking to Mario after admitting defeat
void repl_02119FB4_ov_3E() { }
