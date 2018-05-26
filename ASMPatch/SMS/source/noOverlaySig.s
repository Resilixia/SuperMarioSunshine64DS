@ Disable Overlay signature checking. Based on Dirbaio's noOverlaySig hack.
@ This should allow us to use ASM hacks freely in Download Play.
@ Hook address is SM64DS-specific but it can be changed for all games.
@ It simply hooks FSi_CompareDigest to return always true.

nsub_0205DE9C:
	mov r0, #1
	bx lr
