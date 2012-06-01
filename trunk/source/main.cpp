
//============================================================================
// Name        : main.cpp
// Version     : WIP SVN
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

#include "config.hpp"
#include "text.hpp"
#include "gc.h"
#include "defines.h"
#include "devicemounter.h"
#include "svnrev.h"

using namespace std;

#define IOCTL_DI_STOPMOTOR	0xE3
#define IOCTL_DI_RESET		0x8A

/* Variables */
static u32 inbuf[8]  ATTRIBUTE_ALIGN(32);
static u32 outbuf[8] ATTRIBUTE_ALIGN(32);
static s32 di_fd = -1;

static u32 listsize = 12;

DML_CFG *BooterCFG;
Config BooterINI;

bool Autoboot;
bool OldDML;
bool DriveReset;

string GC_Language_string;
vector<string> GC_Language_strings;
u8 GC_Language;

string GC_Video_string;
vector<string> GC_Video_strings;
u8 GC_Video_Mode;

vector<string> DirEntries;
vector<string> DirEntryNames;
vector<string> DirEntryIDs;

u8 currentDev;

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
	BooterINI.setBool(Domain, "Drive_Reset", DriveReset);
	BooterINI.setBool(Domain, "OldDML", OldDML);
	BooterINI.setBool("GENERAL", "usb", currentDev);
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
		DriveReset = BooterINI.getBool(Domain, "Drive_Reset", true);
		OldDML = BooterINI.getBool(Domain, "OldDML", false);
		currentDev = BooterINI.getBool("GENERAL", "usb", false);
	}
	else
		WriteConfig(Domain);
}

void SetDefaultConfig()
{
	BooterCFG->Magicbytes = 0xD1050CF6;
	BooterCFG->CfgVersion = 0x00000001;
	BooterCFG->VideoMode |= DML_VID_DML_AUTO;
	currentDev = SD;

	GC_Language_strings.push_back("Wii"); GC_Language_strings.push_back("English");
	GC_Language_strings.push_back("German"); GC_Language_strings.push_back("French");
	GC_Language_strings.push_back("Spanish"); GC_Language_strings.push_back("Italian");
	GC_Language_strings.push_back("Dutch");

	GC_Video_strings.push_back("Auto"); GC_Video_strings.push_back("PAL50");
	GC_Video_strings.push_back("NTSC480i"); GC_Video_strings.push_back("PAL60");
	GC_Video_strings.push_back("NTSC480p");  GC_Video_strings.push_back("PAL480p");
	GC_Video_strings.push_back("NTSC480p (patched)");  GC_Video_strings.push_back("PAL480p (patched)");
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
	else if(Option == 0x52455345) //RESE
		return (DriveReset == true) ? "On" : "Off";
	else if(Option == 0x424F4F54) //BOOT
		return (OldDML == true) ? "Old" : "New";
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
	else if(Option == 0x52455345) //RESE
	{
		DriveReset = (DriveReset == true) ? false : true;
		return;
	}
	else if(Option == 0x424F4F54) //BOOT
	{
		OldDML = (OldDML == true) ? false : true;
		return;
	}
	else if(BooterCFG->Config & Option)
		BooterCFG->Config &= ~Option;
	else
		BooterCFG->Config |= Option;
}

void ReadGameDir()
{
	DirEntries.clear();
	DirEntryIDs.clear();
	DirEntryNames.clear();

	const char *DMLgameDir = fmt("%s:/games/", DeviceName[currentDev]);

	DIR *DMLdir = opendir(DMLgameDir);
	struct dirent *pent;
	ifstream infile;
	char infileBuffer[64];
	char gamePath[1024];

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
		if(!infile.is_open())
		{
			memset(gamePath, 0, sizeof(gamePath));
			snprintf(gamePath, sizeof(gamePath), "%s%s/sys/boot.bin", DMLgameDir, pent->d_name);
			infile.open(gamePath, ios::binary);
		}
		if(infile.is_open())
		{
			u32 magicword;
			infile.seekg(0x1c, ios::beg);
			infile.read((char*)&magicword, sizeof(magicword));
			if(magicword != 0xc2339f3d)
			{
				infile.close();
				continue;
			}
			if(strcasestr(gamePath, "boot.bin") != NULL)
			{
				memset(gamePath, 0, sizeof(gamePath));
				snprintf(gamePath, sizeof(gamePath), "%s/boot.bin", pent->d_name);
				DirEntries.push_back(string(gamePath));
			}
			else
			{
				DirEntries.push_back(string(pent->d_name));
			}
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
}

void OptionsMenu()
{
	if(!Autoboot)
		ReadConfig("GENERAL");
	else
		ReadConfig(AUTOBOOT_GAME_ID);

	bool done = false;
	u8 verticalselect = 1;
	u8 Page = 1;
	u8 Pages = 2;
	u8 PageSize = 6;

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
	OptionNameList.push_back("Language          "); OptionList.push_back(0x4C414E47); //LANG 
	OptionNameList.push_back("Video Mode        "); OptionList.push_back(0x56494445); //VIDE
	OptionNameList.push_back("Booter Drive Reset"); OptionList.push_back(0x52455345); //RESE
	OptionNameList.push_back("Boot Method       "); OptionList.push_back(0x424F4F54); //BOOT

	while(!done)
	{
		/* Clear console */
		VIDEO_WaitVSync();
		printf("\x1b[2J");
		printf("\x1b[37m");
		printf("DML Game Booter SVN r%s by FIX94 \n \n", SVN_REV);
		printf("Options\nPress the B Button to return to game selection \nor +/- (X/Y) to switch page.\n");
		printf(" \nPage %i/%i\n", Page, Pages);
		for(u8 i = PageSize * (Page - 1); i < OptionList.size(); i++)
		{
			if(i >= PageSize * (Page))
				break;
			if(verticalselect - 1 == i)
				printf("\x1b[33m");
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
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_PLUS) || (PAD_ButtonsDown(0) == PAD_BUTTON_X))
		{
			Page--;
			verticalselect = 1 + PageSize * (Page - 1);
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_MINUS) || (PAD_ButtonsDown(0) == PAD_BUTTON_Y))
		{
			Page++;
			verticalselect = 1 + PageSize * (Page - 1);
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_B) || (PAD_ButtonsDown(0) == PAD_BUTTON_B))
			done = true;
		if(Page == 0)
		{
			Page = Pages;
			verticalselect = 1 + PageSize * (Page - 1);
		}
		if(Page > Pages)
		{
			Page = 1;
			verticalselect = 1 + PageSize * (Page - 1);
		}
		if(verticalselect == PageSize * (Page - 1))
			verticalselect = PageSize * (Page);
		if(verticalselect > PageSize * (Page))
			verticalselect = 1 + PageSize * (Page - 1);
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
	SDCard_Init();

	/* Init Video */
	VIDEO_Init();
	GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
	void *xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	CON_InitEx(rmode, 24, 32, rmode->fbWidth - (32), rmode->xfbHeight - (48));
	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);

	WPAD_Init();
	PAD_Init();

	bool done = false;
	bool exit = false;
	DriveReset = true;
	OldDML = false;

	char listlimits[77];
	for(u8 i = 0; i < 76; i++)
		listlimits[i] = '#';
	listlimits[76] = '\0';

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

	u8 position = 0;
	u8 listposition = 1;
	u8 timeout = 3;

	if(currentDev)
		USBDevice_Init();
	ReadGameDir();

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
		printf("DML Game Booter SVN r%s by FIX94 \n \n", SVN_REV);
		if(!MainMenuAutoboot)
			printf("Please select a game with the Wiimote(GameCube Controller) Digital Pad.\n \n");
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

		printf("Press the HOME(Start) Button to exit, \nthe A Button to continue \nor the B Button to enter the options. \n \n");
		printf("Press the +(X) Button to switch the Device. \nCurrent Device: %s\n \n", DeviceName[currentDev]);

		printf(listlimits);
		for(u8 i = 0; i < listsize; i++)
		{
			if(listposition - 1 == i)
				printf("\x1b[33m");
			else
				printf("\x1b[37m");
			if(position - 1 + i < (u8)DirEntryNames.size())
				printf("%s\n", DirEntryNames.at(position - 1 + i).c_str());
			else
				printf("\n");
		}
		printf("\x1b[37m");
		printf(listlimits);

		/* Waiting until File selected */
		WPAD_ScanPads();
		PAD_ScanPads();
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_UP) || (PAD_ButtonsDown(0) == PAD_BUTTON_UP))
		{
			listposition--;
			if(listposition < 1)
			{
				listposition = 1;
				position--;
			}
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_DOWN) || (PAD_ButtonsDown(0) == PAD_BUTTON_DOWN))
		{
			listposition++;
			if(listposition > listsize)
			{
				listposition = listsize;
				position++;
			}
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_A) || (PAD_ButtonsDown(0) == PAD_BUTTON_A))
			done = true;
		if(((WPAD_ButtonsDown(0) == WPAD_BUTTON_B) || (PAD_ButtonsDown(0) == PAD_BUTTON_B)))
			OptionsMenu();
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_PLUS) || (PAD_ButtonsDown(0) == PAD_BUTTON_X))
		{
			currentDev = (currentDev == 0) ? 1 : 0;
			WriteConfig("GENERAL");
			if(currentDev)
				USBDevice_Init();
			else
				USBDevice_deInit();
			ReadGameDir();
			position = 1;
			listposition = 1;
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) == PAD_BUTTON_START))
		{
			done = true;
			exit = true;
		}
		if(position == 0)
		{
			if(DirEntries.size() > listsize)
			{
				listposition = listsize;
				position = DirEntries.size() - (listposition - 1);
			}
			else
			{
				listposition = DirEntries.size();
				position = 1;
			}
		}
		if(position + (listposition - 1) > (u8)DirEntries.size())
		{
			listposition = 1;
			position = 1;
		}
	}

	if(exit)
	{
		SDCard_deInit();
		USBDevice_deInit();
		printf("HOME/Start Button pressed, exiting...\n");
		WPAD_Shutdown();
		wait(3);
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
		return 0;
	}
	else
	{
		position += (listposition - 1);
		printf(" \nSelected: %s\nBooting game...\n", DirEntryNames.at(position - 1).c_str());
	}

	WPAD_Shutdown();
	BooterINI.unload();

	if(strcasestr(DirEntries.at(position - 1).c_str(), "boot.bin") != NULL)
	{
		DirEntries.at(position - 1).erase(DirEntries.at(position - 1).size() - 8, 8);
		snprintf(BooterCFG->GamePath, sizeof(BooterCFG->GamePath), "/games/%s", DirEntries.at(position - 1).c_str());
	}
	else
	{
		snprintf(BooterCFG->GamePath, sizeof(BooterCFG->GamePath), "/games/%s/game.iso", DirEntries.at(position - 1).c_str());
	}
	BooterCFG->Config |= DML_CFG_GAME_PATH;

	if(GC_Video_Mode > 5)
		BooterCFG->VideoMode |= DML_VID_PROG_PATCH;

	/* Set DML Options */
	if(OldDML)
		DML_Old_SetOptions(DirEntries.at(position - 1).c_str());
	else
		DML_New_SetOptions(BooterCFG);

	SDCard_deInit();
	USBDevice_deInit();

	if(DriveReset)
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
			GC_SetVideoMode(1);
		else
			GC_SetVideoMode(2);
	}
	else
		GC_SetVideoMode(GC_Video_Mode);

	/* Set GC Lanugage */
	GC_SetLanguage(GC_Language);

	/* Boot BC */
	return WII_LaunchTitle(0x100000100LL);
}
