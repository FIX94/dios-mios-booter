
//============================================================================
// Name        : main.cpp
// Version     : v0.9
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
#include "config.hpp"

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
Config BooterINI;
bool Autoboot;
bool OldDML;
bool DMLv1_0;
string GC_Language_string;
vector<string> GC_Language_strings;
u8 GC_Language;
string GC_Video_string;
vector<string> GC_Video_strings;
u8 GC_Video_Mode;

string GetLanguage()
{
	bool found = false;
	u8 i;
	for(i = 0; i < GC_Language_strings.size(); i++)
	{
		if(strcasecmp(GC_Language_strings.at(i).c_str(), GC_Language_string.c_str()) == 0)
		{
			found = true;
			GC_Language = i;
			break;
		}
	}
	if(!found)
	{
		GC_Language_string = GC_Language_strings.at(0);
		GC_Language = 0;
		i = 0;
	}

	return GC_Language_strings.at(i);
}

string GetVideoMode()
{
	bool found = false;
	u8 i;
	for(i = 0; i < GC_Video_strings.size(); i++)
	{
		if(strcasecmp(GC_Video_strings.at(i).c_str(), GC_Video_string.c_str()) == 0)
		{
			found = true;
			GC_Video_Mode = i;
			break;
		}
	}
	if(!found)
	{
		GC_Video_string = GC_Video_strings.at(0);
		GC_Video_Mode = 0;
		i = 0;
	}

	return GC_Video_strings.at(i);
}

void WriteConfig(const char *Domain)
{
	GetLanguage();
	GetVideoMode();
	BooterINI.setBool(Domain, "ActivityLED", (BooterCFG->Config & DML_CFG_ACTIVITY_LED));
	BooterINI.setBool(Domain, "PadReset", (BooterCFG->Config & DML_CFG_PADHOOK));
	BooterINI.setBool(Domain, "Cheats", (BooterCFG->Config & DML_CFG_CHEATS));
	BooterINI.setBool(Domain, "NMM", (BooterCFG->Config & DML_CFG_NMM));
	BooterINI.setBool(Domain, "NMM_Debug", (BooterCFG->Config & DML_CFG_NMM_DEBUG));
	BooterINI.setBool(Domain, "No_Disc_Patch", (BooterCFG->Config & DML_CFG_NODISC));
	BooterINI.setBool(Domain, "Debugger", (BooterCFG->Config & DML_CFG_DEBUGGER));
	BooterINI.setBool(Domain, "Wait_for_Debugger", (BooterCFG->Config & DML_CFG_DEBUGWAIT));
	BooterINI.setString(Domain, "Language", GC_Language_string);
	BooterINI.setString(Domain, "VideoMode", GC_Video_string);
	BooterINI.save(false);
}

void NextLanguage()
{
	string current_GC_Language_string = GetLanguage();
	for(u8 i = 0; i < GC_Language_strings.size(); i++)
	{
		if(strcasecmp(current_GC_Language_string.c_str(), GC_Language_strings.at(i).c_str()) == 0)
		{
			if(i+1 == (u8)GC_Language_strings.size())
				GC_Language_string = GC_Language_strings.front();
			else
				GC_Language_string = GC_Language_strings.at(i+1).c_str();
		}
	}
}

void PrevLanguage()
{
	string current_GC_Language_string = GetLanguage();
	for(u8 i = 0; i < GC_Language_strings.size(); i++)
	{
		if(strcasecmp(current_GC_Language_string.c_str(), GC_Language_strings.at(i).c_str()) == 0)
		{
			if(i == 0)
				GC_Language_string = GC_Language_strings.back();
			else
				GC_Language_string = GC_Language_strings.at(i-1).c_str();
		}
	}
}

void NextVideoMode()
{
	string current_GC_Video_string = GetVideoMode();
	for(u8 i = 0; i < GC_Video_strings.size(); i++)
	{
		if(strcasecmp(current_GC_Video_string.c_str(), GC_Video_strings.at(i).c_str()) == 0)
		{
			if(i+1 == (u8)GC_Video_strings.size())
				GC_Video_string = GC_Video_strings.front();
			else
				GC_Video_string = GC_Video_strings.at(i+1).c_str();
		}
	}
}

void PrevVideoMode()
{
	string current_GC_Video_string = GetVideoMode();
	for(u8 i = 0; i < GC_Video_strings.size(); i++)
	{
		if(strcasecmp(current_GC_Video_string.c_str(), GC_Video_strings.at(i).c_str()) == 0)
		{
			if(i == 0)
				GC_Video_string = GC_Video_strings.back();
			else
				GC_Video_string = GC_Video_strings.at(i-1).c_str();
		}
	}
}

void ReadConfig(const char *Domain)
{
	if(BooterINI.loaded())
	{
		if(BooterINI.getBool(Domain, "ActivityLED"))
			BooterCFG->Config |= DML_CFG_ACTIVITY_LED;
		else
			BooterCFG->Config &= ~DML_CFG_ACTIVITY_LED;
		if(BooterINI.getBool(Domain, "PadReset"))
			BooterCFG->Config |= DML_CFG_PADHOOK;
		else
			BooterCFG->Config &= ~DML_CFG_PADHOOK;
		if(BooterINI.getBool(Domain, "Cheats"))
			BooterCFG->Config |= DML_CFG_CHEATS;
		else
			BooterCFG->Config &= ~DML_CFG_CHEATS;
		if(BooterINI.getBool(Domain, "NMM"))
			BooterCFG->Config |= DML_CFG_NMM;
		else
			BooterCFG->Config &= ~DML_CFG_NMM;
		if(BooterINI.getBool(Domain, "NMM_Debug"))
			BooterCFG->Config |= DML_CFG_NMM_DEBUG;
		else
			BooterCFG->Config &= ~DML_CFG_NMM_DEBUG;
		if(BooterINI.getBool(Domain, "No_Disc_Patch"))
			BooterCFG->Config |= DML_CFG_NODISC;
		else
			BooterCFG->Config &= ~DML_CFG_NODISC;
		if(BooterINI.getBool(Domain, "Debugger"))
			BooterCFG->Config |= DML_CFG_DEBUGGER;
		else
			BooterCFG->Config &= ~DML_CFG_DEBUGGER;
		if(BooterINI.getBool(Domain, "Wait_for_Debugger"))
			BooterCFG->Config |= DML_CFG_DEBUGWAIT;
		else
			BooterCFG->Config &= ~DML_CFG_DEBUGWAIT;
		GC_Language_string = BooterINI.getString(Domain, "Language", "Wii");
		GetLanguage();
		GC_Video_string = BooterINI.getString(Domain, "VideoMode", "Auto");
		GetVideoMode();
	}
	else
		WriteConfig(Domain);
}

void SetDefaultConfig()
{
	BooterCFG->Magicbytes = 0xD1050CF6;
	BooterCFG->CfgVersion = 0x00000001;
	BooterCFG->VideoMode |= DML_VID_DML_AUTO;

	GC_Language_strings.push_back("Wii"); GC_Language_strings.push_back("English");
	GC_Language_strings.push_back("German"); GC_Language_strings.push_back("French");
	GC_Language_strings.push_back("Spanish"); GC_Language_strings.push_back("Italian");
	GC_Language_strings.push_back("Dutch");

	GC_Video_strings.push_back("Auto"); GC_Video_strings.push_back("PAL50");
	GC_Video_strings.push_back("NTSC480i"); GC_Video_strings.push_back("PAL60");
	GC_Video_strings.push_back("NTSC480p");  GC_Video_strings.push_back("PAL480p");
}

void wait(u32 s)
{
	time_t t;
	t = time(NULL) + s;
	while (time(NULL) < t);
}

const char *GetOption(u32 Option)
{
	if(Option == 0x4C414E47) //LANG
		return GetLanguage().c_str();
	else if(Option == 0x56494445) //VIDE
		return GetVideoMode().c_str();
	else if(BooterCFG->Config & Option)
		return "On";
	else
		return "Off";
}

void SetOption(u32 Option, u8 direction)
{
	if(Option == 0x4C414E47) //LANG
	{
		if(direction == 9) //Right
			NextLanguage();
		else if(direction == 1) //Left
			PrevLanguage();
		return;
	}
	else if(Option == 0x56494445) //VIDE
	{
		if(direction == 9) //Right
			NextVideoMode();
		else if(direction == 1) //Left
			PrevVideoMode();
		return;
	}
	else if(BooterCFG->Config & Option)
		BooterCFG->Config &= ~Option;
	else
		BooterCFG->Config |= Option;
}

void OptionsMenu()
{
	if(!Autoboot)
		ReadConfig("GENERAL");
	else
		ReadConfig(AUTOBOOT_GAME_ID);

	bool done = false;
	u8 verticalselect = 1;

	vector<u32> OptionList;
	vector<string> OptionNameList;
	if(!OldDML && !DMLv1_0)
	{
		OptionNameList.push_back("Cheats            ");	OptionList.push_back(DML_CFG_CHEATS);
		OptionNameList.push_back("Debugger          ");	OptionList.push_back(DML_CFG_DEBUGGER);
		OptionNameList.push_back("Wait for Debugger ");	OptionList.push_back(DML_CFG_DEBUGWAIT);
		OptionNameList.push_back("NMM               ");	OptionList.push_back(DML_CFG_NMM);
		OptionNameList.push_back("NMM Debug         ");	OptionList.push_back(DML_CFG_NMM_DEBUG);
		OptionNameList.push_back("Activity LED      ");	OptionList.push_back(DML_CFG_ACTIVITY_LED);
		OptionNameList.push_back("Pad Hook          ");	OptionList.push_back(DML_CFG_PADHOOK);
		OptionNameList.push_back("No Disc Patch     ");	OptionList.push_back(DML_CFG_NODISC);
	}
	OptionNameList.push_back("Language          "); OptionList.push_back(0x4C414E47); //LANG 
	OptionNameList.push_back("Video Mode        "); OptionList.push_back(0x56494445); //VIDE

	while(!done)
	{
		/* Clear console */
		VIDEO_WaitVSync();
		printf("\x1b[2J");
		printf("\x1b[37m");
		printf("DML Game Booter v0.9 by FIX94 \n \n");
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
		PAD_ScanPads();
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_RIGHT) || (PAD_ButtonsDown(0) == PAD_BUTTON_RIGHT))
			SetOption(OptionList.at(verticalselect - 1), 9);
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_LEFT) || (PAD_ButtonsDown(0) == PAD_BUTTON_LEFT))
			SetOption(OptionList.at(verticalselect - 1), 1);
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_UP) || (PAD_ButtonsDown(0) == PAD_BUTTON_UP))
			verticalselect--;
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_DOWN) || (PAD_ButtonsDown(0) == PAD_BUTTON_DOWN))
			verticalselect++;
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_B) || (PAD_ButtonsDown(0) == PAD_BUTTON_B))
			done = true;
		if(verticalselect == 0)
			verticalselect = OptionList.size();
		if(verticalselect > OptionList.size())
			verticalselect = 1;
	}
	/* Set new Options */
	if(!Autoboot)
		WriteConfig("GENERAL");
	else
		WriteConfig(AUTOBOOT_GAME_ID);
	OptionList.clear();
	OptionNameList.clear();
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
	PAD_Init();

	bool done = false;
	bool exit = false;

	OldDML = false;
	#ifdef OLD_DML
		OldDML = true;
	#endif

	DMLv1_0 = false;
	#ifdef DML_1_0
		DMLv1_0 = true;
	#endif

	Autoboot = false;
	#ifdef AUTOBOOT
		Autoboot = true;
	#endif

	bool MainMenuAutoboot = Autoboot;

	BooterINI.load("sd:/games/dml_booter.ini");
	if(!Autoboot)
		ReadConfig("GENERAL");
	else
		ReadConfig(AUTOBOOT_GAME_ID);

	const char *DMLgameDir = "sd:/games/";

	DIR *DMLdir = opendir(DMLgameDir);
	struct dirent *pent;
	ifstream infile;
	char infileBuffer[64];
	char gamePath[1024];
	vector<string> DirEntries;
	vector<string> DirEntryNames;
	vector<string> DirEntryIDs;
	u8 position = 0;
	u8 timeout = 3;

	while(1)
	{
		pent = readdir(DMLdir);
		if(pent == NULL)
			break;
		if (strcmp(pent->d_name, ".") == 0 || strcmp(pent->d_name, "..") == 0)
			continue;
		memset(gamePath, 0, sizeof(gamePath));
		snprintf(gamePath, sizeof(gamePath), "%s%s/sys/boot.bin", DMLgameDir, pent->d_name);
		infile.open(gamePath, ios::binary);
		if(!infile.is_open())
		{
			memset(gamePath, 0, sizeof(gamePath));
			snprintf(gamePath, sizeof(gamePath), "%s%s/game.iso", DMLgameDir, pent->d_name);
			infile.open(gamePath, ios::binary);
		}
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

	if(Autoboot)
	{
		for(u8 i = 0; i < DirEntryIDs.size(); i++)
		{
			if(strcmp(DirEntryIDs.at(i).c_str(), AUTOBOOT_GAME_ID) == 0)
			{
				position = i + 1;
				break;
			}
		}
		if(position == 0)
		{
			vector<string> tmpIDs = DirEntryIDs;
			string AutoID(AUTOBOOT_GAME_ID);
			AutoID.erase(3, 1);
			for(u8 i = 0; i < tmpIDs.size(); i++)
			{
				tmpIDs.at(i).erase(3, 1);
				if(strcmp(tmpIDs.at(i).c_str(), AutoID.c_str()) == 0)
				{
					position = i + 1;
					break;
				}
			}
		}
		if(position == 0)
		{
			Autoboot = false;
			MainMenuAutoboot = false;
		}
	}
	else
		position = 1;

	time_t t;
	t = time(NULL) + timeout;

	while(!done)
	{
		/* Clear console */
		VIDEO_WaitVSync();
		printf("\x1b[2J");
		printf("\x1b[37m");
		printf("DML Game Booter v0.9 by FIX94 \n \n");
		if(!MainMenuAutoboot)
			printf("Please select a game with the Wiimote Digital Pad.\n");
		else
		{
			printf("Autoboot requested!\nPress any button to abort... %i\n", int(t-time(NULL)));
			WPAD_ScanPads();
			PAD_ScanPads();
			if(WPAD_ButtonsDown(0) || PAD_ButtonsDown(0))
				MainMenuAutoboot = false;
			if(time(NULL) >= t)
				break;
			if(MainMenuAutoboot)
				continue;
		}
		printf("<<<  %s  >>>\n \n", DirEntryNames.at(position - 1).c_str());
		printf("Press the HOME/Start Button to exit, \nthe A Button to continue \nor the B Button to enter the options.\n");
		/* Waiting until File selected */
		WPAD_ScanPads();
		PAD_ScanPads();
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_RIGHT) || (PAD_ButtonsDown(0) == PAD_BUTTON_RIGHT))
			position++;
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_LEFT) || (PAD_ButtonsDown(0) == PAD_BUTTON_LEFT))
			position--;
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_A) || (PAD_ButtonsDown(0) == PAD_BUTTON_A))
			done = true;
		if(((WPAD_ButtonsDown(0) == WPAD_BUTTON_B) || (PAD_ButtonsDown(0) == PAD_BUTTON_B)))
			OptionsMenu();
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) == PAD_BUTTON_START))
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
		printf("HOME/Start Button pressed, exiting...\n");
		WPAD_Shutdown();
		wait(3);
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
		return 0;
	}
	else
		printf(" \nSelected: %s\nBooting game...\n", DirEntryNames.at(position - 1).c_str());

	WPAD_Shutdown();
	BooterINI.unload();

	snprintf(BooterCFG->GamePath, sizeof(BooterCFG->GamePath), "/games/%s/game.iso", DirEntries.at(position - 1).c_str());
	BooterCFG->Config |= DML_CFG_GAME_PATH;

	/* Set DML Options */
	if(OldDML || DMLv1_0)
		DML_Old_SetOptions(DirEntries.at(position - 1).c_str());
	else
		DML_New_SetOptions(BooterCFG);

	/* Unmount SD Card */
	fatUnmount("sd");
	storage.shutdown();

	if(!DMLv1_0)
	{
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
	}

	/* Set Video Mode */
	if(GC_Video_Mode == 0)
	{
		if(DirEntryIDs.at(position - 1).c_str()[3] == 'P')
		{
			if(!DMLv1_0)
				GC_SetVideoMode(1);
			else
				GC_SetVideoMode(5);
		}
		else
		{
			if(!DMLv1_0)
				GC_SetVideoMode(2);
			else
				GC_SetVideoMode(4);
		}
	}
	else
		GC_SetVideoMode(GC_Video_Mode);

	/* Set GC Lanugage */
	GC_SetLanguage(GC_Language);

	/* Boot BC */
	return WII_LaunchTitle(0x100000100LL);
}
