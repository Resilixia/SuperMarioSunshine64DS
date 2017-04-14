# Super Mario Sunshine 64 DS
by Fiachra

This is an incomplete port of many Super Mario Sunshine elements into Super Mario 64 DS. This repository contains the assets and source code required to import F.L.U.D.D. into the game as seen in [this](https://www.youtube.com/watch?v=OZ08eG4iXJs) video.

## Requirements
- The latest version of [Super Mario 64 DS Editor (SM64DSe)](http://kuribo64.net/board/thread.php?id=1593)
- The [Super Mario 64 DS ASM Hacking Template](http://kuribo64.net/board/thread.php?id=1758) and its required tools set up and running
- A clean (unmodified) copy of the "Super Mario 64 DS (E)(Wet 'N' Wild)" ROM, (MD5 checksum: 867B3D17AD268E10357C9754A77147E5)
- Blender (tested with v2.70a although newer versions should be fine)
- A hex editor eg. XVI32

## Instructions
1. Open your SM64DS ROM in SM64DSe. You will be prompted to make a backup before required patches are applied to the ROM.
2. Click on "More" in the top toolbar and select "Toggle Suitability for NSMBe ASM Patching". You should then see a confirmation message "ROM is now suitable for use with NSMBe's ASM patch insertion feature".
3. Navigate to "SMS/CustomObjects/F.L.U.D.D._PowerUp/" and follow the instructions to import the new character models found in "Instructions.Mario.txt", "Instructions.Luigi.txt" and "Instructions.Wario.txt".
4. Navigate to "SMS/CustomObjects/F.L.U.D.D._PowerUp/NozzleIcon" and follow the instructions to import the HUD icons found in "Instructions.NozzleIcon.txt".
5. Within a terminal navigate to "ASMPatch". Run "make" and ensure that you are able to compile the code successfully. If compilation fails ensure that you have correctly followed the steps to set up the ASM Hacking Template and its required tools.
6. Make a copy of your updated ROM.
7. Open the copy your updated ROM in NSMBe and select the "Tools/Options" tab.
8. Under the "ASM Tools" header click on "Run 'make clean'" followed by "Run 'make' and insert". Once finished you may close NSMBe.

## Licensing
You are free to make use of this code and its assets in a ROM hack of your own as long as you give credit for anything used.
