# Super Mario Sunshine 64 DS
by Fiachra

This is an incomplete port of many Super Mario Sunshine elements into Super Mario 64 DS. This repository contains the assets and source code required to import F.L.U.D.D. into the game as seen in [this](https://www.youtube.com/watch?v=OZ08eG4iXJs) video.

## Requirements
- The latest version of [Super Mario 64 DS Editor (SM64DSe)](http://kuribo64.net/board/thread.php?id=1593)
- The [Super Mario 64 DS ASM Hacking Template](http://kuribo64.net/board/thread.php?id=1758) and its required tools set up and running
- A clean (unmodified) copy of the "Super Mario 64 DS (E)(Wet 'N' Wild)" ROM, (MD5 checksum: 867B3D17AD268E10357C9754A77147E5)
- Blender (tested with v2.70a although newer versions should be fine)
- A hex editor eg. XVI32
- devkitPro 1.6.0 (devkitARM)

## Instructions
1. Open your SM64DS ROM in SM64DSe. You will be prompted to make a backup before required patches are applied to the ROM.
2. Click on "More" in the top toolbar and select "Toggle Suitability for NSMBe ASM Patching". You should then see a confirmation message "ROM is now suitable for use with NSMBe's ASM patch insertion feature".
3. Navigate to "SMS/CustomObjects/F.L.U.D.D._PowerUp/" and follow the instructions to import the new character models found in "Instructions.Mario.txt", "Instructions.Luigi.txt" and "Instructions.Wario.txt".
4. Navigate to "SMS/CustomObjects/F.L.U.D.D._PowerUp/NozzleIcon" and follow the instructions to import the HUD icons found in "Instructions.NozzleIcon.txt".
5. Within a terminal navigate to "ASMPatch/F.L.U.D.D.Only". Run "make" and ensure that you are able to compile the code successfully. If compilation fails ensure that you have correctly followed the steps to set up the ASM Hacking Template and its required tools.
6. Make a copy of your updated ROM.
7. Open the copy your updated ROM in NSMBe and select the "Tools/Options" tab.
8. Under the "ASM Tools" header click on "Run 'make clean'" followed by "Run 'make' and insert". Once finished you may close NSMBe.

## Known Issues
The following levels crash upon loading. I think most, if not all, are due to too many polygons and/or textures.
- Sirena Beach
- Hotel Delfino
- Casino Upper
- Noki Bay
- Noki Bay Underwater City
- Gelato Beach: version without vertex colours (used) works but not the version with vertex colours (.blend and settings included)
- Pinna Park: should really be split in two like the original
- Hillside Cave v2 (by "Nomura")

There is a bug where you can't refill F.L.U.D.D. once the water level reaches zero.

## SM64DSe Issues
- I had to use an older version of SM64DSe to import the Petey Piranha IMD model (bosspaku_model_Base_Use_RaisedAndBack.imd): SM64DSe_20170530_R100
- The imported Petey Piranha BMD does not load correctly in the model editor (load internal model)
- Can't import a 256x256 minimap over an existing 256x256 minimap using the minimap editor
- When copying CLPS from a different level the changes aren't saved

## Other notes
- The ASM patch "Shared_MinimapSizeSetter.cpp" will make all levels' minimaps 256x256 with the exception of the 4 levels used in multi-player and Test Map B (any others?). This means that the minimaps won't display properly in-game until it's patched.

## Licensing
You are free to make use of this code and its assets in a ROM hack of your own as long as you give credit for anything used.

"Stomatol" has provided imported music for Bianco Hills, bonus levels and Gelato Beach. Files under SMS/Music_Stomatol.
kuribo64
"Nomura" has provided three custom levels: Hillside Cave; Rainbow Tunnel; The Hidden Slope. Files under SMS/OtherLevels.

And obviously many assets belong to Nintendo.
