#include "Shared_TimeRace.h"

// Allows overriding of the value in Player +0x664 in method 020C0FB4 which checks the 
// value and: if 0x0F starts timer, if 0x10 stops timer and spawns Star if you were 
// fast enough

bool STR_ShouldOverrideBehaviour = false;
int STR_OverridePlayerPlus664Value = 0;
bool STR_ShouldOverrideStarID = false;
byte STR_OverrideStarID = 1;
bool STR_ShouldOverrideMaxTime = false;
int STR_OverrideMaxTimeSeconds = 0;

int STR_START_TIMER_BEHAVIOUR = 0x0F;
int STR_STOP_TIMER_BEHAVIOUR = 0x10;

bool STR_RaceStarted = false;
bool STR_RaceFinished = false;
bool STR_RaceWon = false;

// Allow for overriding the value in Player +0x664 to trigger a certain behaviour
void repl_020C0FC0()
{
	asm
	(
		"ldr r1, =STR_ShouldOverrideBehaviour		\t\n"
		"ldrb r2, [r1]								\t\n"
		"cmp r2, #0x01								\t\n"
		"ldrne r1, [r4, #0x664]						\t\n"
		"bxne r14									\t\n"
		"mov r2, #0x00								\t\n"
		"strb r2, [r1]								\t\n"
		"ldr r1, =STR_OverridePlayerPlus664Value	\t\n"
		"ldr r1, [r1]								\t\n"
	);
}

void hook_020C10A4()
{
	STR_RaceStarted = true;
}

// Allow overriding the default time limit
void repl_020C10F4()
{
	asm
	(
		"ldr r6, =STR_RaceFinished					\t\n"
		"mov r7, #0x01								\t\n"
		"strb r7, [r6]								\t\n"
		"ldr r6, =STR_ShouldOverrideMaxTime			\t\n"
		"ldrb r7, [r6]								\t\n"
		"cmp r7, #0x01								\t\n"
		"bne no_override_020C10F4					\t\n"
		"mov r7, #0x00								\t\n"
		"strb r7, [r6]								\t\n"
		"ldr r6, =STR_OverrideMaxTimeSeconds		\t\n"
		"ldr r6, [r6]								\t\n"
		"b exit_020C10F4							\t\n"
		
		"no_override_020C10F4: 						\t\n"
		"mov r6, #0x14								\t\n"
		
		"exit_020C10F4:								\t\n"
		"cmp r0, r6									\t\n"
	);
}

// Allow overriding the default Star ID
void repl_020C112C()
{
	asm
	(
		"ldr r6, =STR_RaceWon						\t\n"
		"mov r1, #0x01								\t\n"
		"strb r1, [r6]								\t\n"
		"ldr r6, =STR_RaceFinished					\t\n"
		"strb r1, [r6]								\t\n"
		"ldr r6, =STR_ShouldOverrideStarID			\t\n"
		"ldrb r1, [r6]								\t\n"
		"cmp r1, #0x01								\t\n"
		"bne original_020C112C						\t\n"
		"mov r1, #0x00								\t\n"
		"strb r1, [r6]								\t\n"
		"ldr r6, =STR_OverrideStarID				\t\n"
		"ldrb r1, [r6]								\t\n"
		"orr r1, #0x40								\t\n"
		"b exit_020C112C							\t\n"
		
		"original_020C112C:							\t\n"
		"mov r1, #0x42								\t\n"
		
		"exit_020C112C:								\t\n"
	);
}
