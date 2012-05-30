
//============================================================================
// Name        : main.cpp
// Version     : v0.5
// Copyright   : 2012 FIX94
// Description : A small and easy DML Game Booter
//============================================================================

#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <dirent.h>
#include <unistd.h>
#include "gc.h"
#include "sdhc.h"
#include "defines.h"

#include <fat.h>
#include <sdcard/wiisd_io.h>

using namespace std;

#define IOCTL_DI_STOPMOTOR	0xE3
#define IOCTL_DI_RESET		0x8A

/* Variables */
static u32 inbuf[8]  ATTRIBUTE_ALIGN(32);
static u32 outbuf[8] ATTRIBUTE_ALIGN(32);
static s32 di_fd = -1;

void wait(u32 s)
{
	time_t t;
	t = time(NULL) + s; 
	while (time(NULL) < t);
}

int main(int argc, char *argv[]) 
{
	/* Mount SD Card */
	DISC_INTERFACE storage = __io_wiisd;
	if (fatMountSimple("sd", &storage) < 0)
		return -1;

	/* Init Video */
	VIDEO_Init();
	GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
	void *xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	CON_InitEx(rmode, 24, 32, rmode->fbWidth - (32), rmode->xfbHeight - (48));
	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);

	WPAD_Init();

	bool done = false;
	bool exit = false;
	bool autoboot = false;

	#ifdef AUTOBOOT
		autoboot = true;
	#endif

	const char *DMLgameDir = "sd:/games/";

	DIR *DMLdir = opendir(DMLgameDir);
	struct dirent *pent;
	ifstream infile;
	char infileBuffer[64];
	char gamePath[1024];
	vector<string> DirEntries;
	vector<string> DirEntryNames;
	vector<string> DirEntryIDs;
	u8 position = 1;

	while(1)
	{
		pent = readdir(DMLdir);
		if(pent == NULL)
			break;
		if (strcmp(pent->d_name, ".") == 0 || strcmp(pent->d_name, "..") == 0)
			continue;
		memset(gamePath, 0, sizeof(gamePath));
		snprintf(gamePath, sizeof(gamePath), "%s%s/game.iso", DMLgameDir, autoboot ? Game : pent->d_name);
		infile.open(gamePath, ios::binary);
		if(infile.is_open())
		{
			DirEntries.push_back(string(autoboot ? Game : pent->d_name));
			infile.seekg(0, ios::beg);
			memset(infileBuffer, 0, sizeof(infileBuffer));
			infile.read(infileBuffer, 6);
			DirEntryIDs.push_back(string(infileBuffer));
			infile.seekg(0x20, ios::beg);
			memset(infileBuffer, 0, sizeof(infileBuffer));
			infile.read(infileBuffer, 64);
			DirEntryNames.push_back(string(infileBuffer));
			infile.close();
		}
		if(autoboot)
			break;
	}

	printf("\x1b[2J");
	while(!done)
	{
		/* Clear console */
		VIDEO_WaitVSync();
		printf("\x1b[2J");
		printf("DML Game Booter v0.5 by FIX94 \n \n");
		if(!autoboot)
			printf("Please select a game.\n");
		else
		{
			printf("Autoboot requested!\n");
			break;
		}
		printf("<<<  %s  >>>\n \n", DirEntryNames.at(position - 1).c_str());
		printf("Press the HOME button to exit, the A button to continue, select with the DPAD.\n");
		/* Waiting until File selected */
		WPAD_ScanPads();
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_RIGHT)
			position++;
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_LEFT)
			position--;
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_A)
			done = true;
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_HOME)
		{
			done = true;
			exit = true;
		}
		if(position == 0)
			position = DirEntries.size();
		if(position > DirEntries.size())
			position = 1;
	}

	if(exit)
	{
		/* Unmount SD Card */
		fatUnmount("sd");
		storage.shutdown();
		printf("Home Button pressed, exiting...\n");
		WPAD_Shutdown();
		wait(3);
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
		return 0;
	}
	else
		printf(" \nSelected: %s\nBooting game...\n", DirEntryNames.at(position - 1).c_str());

	WPAD_Shutdown();

	#ifdef OLD_DML
		DML_Old_SetOptions(DirEntries.at(position - 1).c_str());
	#else
		DML_New_SetOptions(DirEntries.at(position - 1).c_str());
	#endif

	/* Unmount SD Card */
	fatUnmount("sd");
	storage.shutdown();

	/* Open "/dev/di" */
	di_fd = IOS_Open("/dev/di", 0);

	/* Reset drive */
	memset(inbuf, 0, sizeof(inbuf));
	inbuf[0] = IOCTL_DI_RESET << 24;
	inbuf[1] = 1;
	IOS_Ioctl(di_fd, IOCTL_DI_RESET, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	
	/* Stop motor */
	memset(inbuf, 0, sizeof(inbuf));
	inbuf[0] = IOCTL_DI_STOPMOTOR << 24;
	IOS_Ioctl(di_fd, IOCTL_DI_STOPMOTOR, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));
	
	/* Close "/dev/di" */
	IOS_Close(di_fd);

	/* Set Video Mode */
	#ifdef FORCE_VIDEO_MODE
		#ifdef PAL_VIDEO_MODE
			GC_SetVideoMode(1);
		#else
			GC_SetVideoMode(2);
		#endif
	#else
		if(DirEntryIDs.at(position - 1).c_str()[3] == 'P')
			GC_SetVideoMode(1);
		else
			GC_SetVideoMode(2);
	#endif

	/* Set GC Lanugage */
	GC_SetLanguage(0);

	/* Boot BC */
	return WII_LaunchTitle(0x100000100LL);
}
