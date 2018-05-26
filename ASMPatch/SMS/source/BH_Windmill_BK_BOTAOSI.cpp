#include "SM64DS.h"

unsigned int objectAddress;
unsigned int objectXRotationAddress;

int const NUM_FRAMES = 128;
int const X_ROTATION_INCREMENT = 0xFF80; // -128

int frameNum = 0;

// Update the X Rotation each frame to produce continuous rotation
void hook_02111674_ov_0F()
{
	asm
	(
		"mov r0, r5							\t\n"
		"ldr r1, =objectAddress				\t\n"
		"str r0, [r1]						\t\n" // Address of object
		"add r0, r5, #0x8C					\t\n" 
		"ldr r1, =objectXRotationAddress	\t\n"
		"str r0, [r1]						\t\n" // Address of object X rotation
	);
	
	*((volatile short int*)objectXRotationAddress) = (frameNum * X_ROTATION_INCREMENT);
	
	if (frameNum < NUM_FRAMES)
	{
		frameNum++;
	}
	else
	{
		frameNum = 0;
	}
}