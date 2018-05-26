#include "SM64DS.h"
#include "Buttons.h"

// Set 'Up' pressed to true
__attribute__((noinline)) void PressUpKey()
{
	*((volatile byte*)0x0209F49C + 0x00) |= 0x40;
	*((volatile byte*)0x0209F49C + 0x05) |= 0x10;
	*((volatile byte*)0x0209F49C + 0x09) |= 0xF0;
	*((volatile byte*)0x0209F49C + 0x0B) |= 0x80;
}

// Set 'Down' pressed to true
__attribute__((noinline)) void PressDownKey()
{
	*((volatile byte*)0x0209F49C + 0x00) |= 0x80;
	*((volatile byte*)0x0209F49C + 0x05) |= 0x10;
	*((volatile byte*)0x0209F49C + 0x09) |= 0x10;
}

// Set 'B' pressed to true
__attribute__((noinline)) void PressBKey()
{
	*((volatile byte*)0x0209F49C + 0x00) |= 0x02;
	*((volatile byte*)0x0209F49C + 0x02) |= 0x02;
}

// Release 'Up' key
__attribute__((noinline)) void ReleaseUpKey()
{
	*((volatile byte*)0x0209F49C + 0x00) &= 0xBF;
	*((volatile byte*)0x0209F49C + 0x05) &= 0xEF;
	*((volatile byte*)0x0209F49C + 0x09) &= 0x0F;
	*((volatile byte*)0x0209F49C + 0x0B) &= 0x7F;
}

// Release 'B' key
__attribute__((noinline)) void ReleaseBKey()
{
	*((volatile byte*)0x0209F49C + 0x00) &= 0xFD;
	*((volatile byte*)0x0209F49C + 0x02) &= 0xFD;
	//*((volatile byte*)0x020A0E58 + 0x00) &= 0xFD;
}

__attribute__((noinline)) bool IsDPadKeyPressed()
{
	return ( (*((volatile byte*)0x0209F49C) & 0xFC) != 0x00 );
}