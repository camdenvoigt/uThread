# uThread
A simple user thread library written in C

## CONTENTS:
  
  Makefile
  uThread.c
  uThread.h
  
  example main files:
   - main.c
   - memoryleaks.c
   - bufferoverflow.c
   - threadsafe.c
  
## INSTRUCTIONS:

run make _____ where the space is replaced with the name of the main file you wish to compile.
Afterwords run my typing ./______

run make clean to remove executables and object files.

refer to uthread.h comments for particular instructions for using the uThread methods.

## CHANGES:
- Created a .h file
- Documented uThread methods
- Freed uThread allocated memory
- Added an input check to uThread_init
- Changed to stack size to be variable
- Added flag in makefile to set canary values before function pointers. 
- Implemented thread specific variable feature
- Created example main files to showcase changes.

## ROOM FOR IMPROVEMENT:
- Further pthread functionality mirroring such as:
  - mutexs
  - conditional variables
  - joining threads
  - passing arguements to threads
- Further refine solutions to buffer overflow and thread safety
