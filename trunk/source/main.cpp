
//============================================================================
// Name        : main.cpp
// Version     : v0.6
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
#include <malloc.h>
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

DML_CFG *BooterCFG;

void WriteConfig()
{
	ofstream outfile;
	outfile.open("sd:/games/DML_BOOTER.bin", ios::binary);
	outfile.write((char*)BooterCFG, sizeof(DML_CFG));
	outfile.close();
}

void ReadConfig()
{
	ifstream infile;
	infile.open("sd:/games/DML_BOOTER.bin", ios::binary);
	if(infile.is_open())
	{
		infile.seekg(0, ios::end);
		if(infile.tellg() != sizeof(DML_CFG))
		{
			infile.close();
			WriteConfig();
			return;
		}
		infile.seekg(0, ios::beg);
		infile.read((char*)BooterCFG, sizeof(DML_CFG));
		infile.close();
	}
	else
		WriteConfig();
}

void SetDefaultConfig()
{
	BooterCFG->Magicbytes = 0xD1050CF6;
	BooterCFG->CfgVersion = 0x00000001;
	BooterCFG->VideoMode |= DML_VID_NONE;

	#ifdef ACTIVITYLED
		BooterCFG->Config |= DML_CFG_ACTIVITY_LED;
	#endif

	#ifdef PADRESET
		BooterCFG->Config |= DML_CFG_PADHOOK;
	#endif

	#ifdef CHEATS
		BooterCFG->Config |= DML_CFG_CHEATS;
	#endif

	#ifdef NMM
		BooterCFG->Config |= DML_CFG_NMM;
	#endif

	#ifdef NODISC
		BooterCFG->Config |= DML_CFG_NODISC;
	#endif
}
void wait(u32 s)
{
	time_t t;
	t = time(NULL) + s;
	while (time(NULL) < t);
}

const char *GetOption(u32 Option)
{
	if(BooterCFG->Config & Option)
		return "On";
	else
		return "Off";
}

void SetOption(u32 Option)
{
	if(BooterCFG->Config & Option)
		BooterCFG->Config &= ~Option;
	else
		BooterCFG->Config |= Option;
}

void OptionsMenu()
{
	bool done = false;
	u8 verticalselect = 1;
	vector<u32> OptionList;
	vector<string> OptionNameList;
	OptionNameList.push_back("Cheats            ");	OptionList.push_back(DML_CFG_CHEATS);
	OptionNameList.push_back("Debugger          ");	OptionList.push_back(DML_CFG_DEBUGGER);
	OptionNameList.push_back("Wait for Debugger ");	OptionList.push_back(DML_CFG_DEBUGWAIT);
	OptionNameList.push_back("NMM               ");	OptionList.push_back(DML_CFG_NMM);
	OptionNameList.push_back("NMM Debug         ");	OptionList.push_back(DML_CFG_NMM_DEBUG);
	OptionNameList.push_back("Activity LED      ");	OptionList.push_back(DML_CFG_ACTIVITY_LED);
	OptionNameList.push_back("Pad Hook          ");	OptionList.push_back(DML_CFG_PADHOOK);
	OptionNameList.push_back("No Disc Patch     ");	OptionList.push_back(DML_CFG_NODISC);

	ReadConfig();
	while(!done)
	{
		/* Clear console */
		VIDEO_WaitVSync();
		printf("\x1b[2J");
		printf("\x1b[37m");
		printf("DML Game Booter v0.6 by FIX94 \n \n");
		printf("Options\nPress the B Button to return to game selection.");
		for(u8 i = 0; i < OptionList.size(); i++)
		{
			if(verticalselect - 1 == i)
				printf("\x1b[32m");
			else
				printf("\x1b[37m");
			printf(" \n%s           <<< %s >>>\n", OptionNameList.at(i).c_str(), GetOption(OptionList.at(i)));
		}
		WPAD_ScanPads();
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_RIGHT)
			SetOption(OptionList.at(verticalselect - 1));
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_LEFT)
			SetOption(OptionList.at(verticalselect - 1));
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_UP)
			verticalselect--;
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_DOWN)
			verticalselect++;
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_B)
			done = true;
		if(verticalselect == 0)
			verticalselect = OptionList.size();
		if(verticalselect > OptionList.size())
			verticalselect = 1;
	}
	/* Set new Options */
	WriteConfig();
}

int main(int argc, char *argv[]) 
{
	BooterCFG = (DML_CFG*)malloc(sizeof(DML_CFG));
	memset(BooterCFG, 0, sizeof(DML_CFG));
	SetDefaultConfig();

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

	if(!autoboot)
		ReadConfig();

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
	u8 timeout = 5;

	while(1)
	{
		pent = readdir(DMLdir);
		if(pent == NULL)
			break;
		if (strcmp(pent->d_name, ".") == 0 || strcmp(pent->d_name, "..") == 0)
			continue;
		memset(gamePath, 0, sizeof(gamePath));
		snprintf(gamePath, sizeof(gamePath), "%s%s/game.iso", DMLgameDir, pent->d_name);
		infile.open(gamePath, ios::binary);
		if(infile.is_open())
		{
			DirEntries.push_back(string(pent->d_name));
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
	}

	if(autoboot)
	{
		for(u8 i = 0; i < DirEntryNames.size(); i++)
		{
			if(strcmp(DirEntries.at(i).c_str(), AUTOBOOT_GAME) == 0)
			{
				position = i + 1;
				break;
			}
		}
	}

	time_t t;
	t = time(NULL) + timeout;

	while(!done)
	{
		/* Clear console */
		VIDEO_WaitVSync();
		printf("\x1b[2J");
		printf("\x1b[37m");
		printf("DML Game Booter v0.6 by FIX94 \n \n");
		if(!autoboot)
			printf("Please select a game with the Wiimote Digital Pad.\n");
		else
		{
			printf("Autoboot requested!\nPress any button to abort... %i\n", int(t-time(NULL)));
			WPAD_ScanPads();
			if(WPAD_ButtonsDown(0))
			{
				autoboot = false;
				ReadConfig();
			}
			if(time(NULL) >= t)
				break;
			if(autoboot)
				continue;
		}
		printf("<<<  %s  >>>\n \n", DirEntryNames.at(position - 1).c_str());
		printf("Press the HOME Button to exit, \nthe A Button to continue \nor the B Button to enter the options.\n");
		/* Waiting until File selected */
		WPAD_ScanPads();
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_RIGHT)
			position++;
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_LEFT)
			position--;
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_A)
			done = true;
		if(WPAD_ButtonsDown(0) == WPAD_BUTTON_B)
			OptionsMenu();
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
		printf("HOME Button pressed, exiting...\n");
		WPAD_Shutdown();
		wait(3);
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
		return 0;
	}
	else
		printf(" \nSelected: %s\nBooting game...\n", DirEntryNames.at(position - 1).c_str());

	WPAD_Shutdown();

	snprintf(BooterCFG->GamePath, sizeof(BooterCFG->GamePath), "/games/%s/game.iso", DirEntries.at(position - 1).c_str());
	BooterCFG->Config |= DML_CFG_GAME_PATH;

	/* Set DML Options */
	#ifdef OLD_DML
		DML_Old_SetOptions(DirEntries.at(position - 1).c_str());
	#else
		DML_New_SetOptions(BooterCFG);
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
