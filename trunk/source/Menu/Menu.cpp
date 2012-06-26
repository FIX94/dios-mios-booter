/****************************************************************************
 * DIOS-MIOS Booter - A small and easy DIOS-MIOS (Lite) Game Booter
 * Copyright (C) 2012  FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <unistd.h>
#include <dirent.h>
#include <fstream>

#include "Menu.hpp"
#include "svnrev.h"
#include "text.hpp"
#include "fs.h"
#include "Memory/mem2.hpp"

#define IOCTL_DI_STOPMOTOR	0xE3
#define IOCTL_DI_RESET		0x8A
#define	HW_PPCSPEED			((vu32*)0xCD800018)

static u32 inbuf[8]  ATTRIBUTE_ALIGN(32);
static u32 outbuf[8] ATTRIBUTE_ALIGN(32);
static s32 di_fd = -1;

void Menu::MainLoop()
{
	VIDEO_WaitVSync();
	usleep(100);
	printf("\x1b[2J");
	printf("\x1b[37m");
	printf("DIOS-MIOS Booter SVN r%s by FIX94\n", SVN_REV);
	printf(MIOS_Info);
	WPAD_ScanPads();
	PAD_ScanPads();
}

void Menu::Start()
{
	InitMain();
	while(!Sys_Exit())
	{
		MainLoop();
		if(currentMenu == MENU_MAIN)
			MainMenu();
		else if(currentMenu == MENU_OPTIONS)
			OptionsMenu();
		else if(currentMenu == MENU_GAME_OPTIONS)
			GameOptionsMenu();
		else
			break;
	}
	BooterINI.unload();
}

bool Menu::AbortOperation()
{
	bool stop = true;
	while(!Sys_Exit())
	{
		usleep(100);
		WPAD_ScanPads();
		PAD_ScanPads();
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_B) || (PAD_ButtonsDown(0) == PAD_BUTTON_B))
			break;
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_A) || (PAD_ButtonsDown(0) == PAD_BUTTON_A))
		{
			stop = false;
			break;
		}
	}
	return stop;
}

void Menu::ReadGameDir()
{
	List.ClearEntries();
	List.AddEntry(string("dvd:/"), string("GCDISC"), string("Boot Disc in DVD Drive"));

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
			/* Checking Magic Word */
			u32 magicword;
			infile.seekg(0x1c, ios::beg);
			infile.read((char*)&magicword, sizeof(magicword));
			if(magicword != 0xc2339f3d)
			{
				infile.close();
				continue;
			}

			/* Game ID */
			infile.seekg(0, ios::beg);
			memset(infileBuffer, 0, sizeof(infileBuffer));
			infile.read(infileBuffer, 6);
			string GameID(infileBuffer);

			/* Game Title */
			infile.seekg(0x20, ios::beg);
			memset(infileBuffer, 0, sizeof(infileBuffer));
			infile.read(infileBuffer, 64);
			string title(infileBuffer);

			/* Game Format */
			if(strcasestr(gamePath, "boot.bin") != NULL)
			{
				memset(gamePath, 0, sizeof(gamePath));
				snprintf(gamePath, sizeof(gamePath), "%s/boot.bin", pent->d_name);
				title.append(" (FST)");
			}
			else
			{
				memset(gamePath, 0, sizeof(gamePath));
				strncpy(gamePath, pent->d_name, sizeof(gamePath));
				title.append(" (ISO)");
			}

			List.AddEntry(string(gamePath), GameID, string(title));
			infile.close();
		}
	}
}

int Menu::BootGame()
{
	position += (listposition - 1);
	printf(" \nSelected: %s\nBooting game...\n", List.GetEntryName(position - 1));

	if(position > 1)
	{
		string gamePath(List.GetEntryPath(position - 1));
		if(strcasestr(gamePath.c_str(), "boot.bin") != NULL)
		{
			gamePath.erase(gamePath.size() - 8, 8);
			snprintf(BooterCFG->GamePath, sizeof(BooterCFG->GamePath), "/games/%s", gamePath.c_str());
		}
		else
			snprintf(BooterCFG->GamePath, sizeof(BooterCFG->GamePath), "/games/%s/game.iso", gamePath.c_str());
		BooterCFG->Config |= DML_CFG_GAME_PATH;
	}
	else
		BooterCFG->Config |= DML_CFG_BOOT_DISC;

	if(GC_Video_Mode > 5)
		BooterCFG->VideoMode |= DML_VID_PROG_PATCH;

	/* Set DML Options */
	if(OldDML && position > 1)
		DML_Old_SetOptions(List.GetEntryID(position - 1));

	UnMountDevices();
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
		if(List.GetEntryID(position - 1)[3] == 'P')
			GC_SetVideoMode(1, BooterCFG);
		else
			GC_SetVideoMode(2, BooterCFG);
	}
	else
		GC_SetVideoMode(GC_Video_Mode, BooterCFG);

	/* Set GC Lanugage */
	GC_SetLanguage(GC_Language);

	/* Write our options into memory */
	if(!OldDML)
		DML_New_WriteOptions(BooterCFG);
	MEM2_free(BooterCFG);

	/* NTSC-J Patch */
	if(NTSCJ_Patch)
		*HW_PPCSPEED = 0x0002A9E0;

	/* Boot BC */
	WII_Initialize();
	return WII_LaunchTitle(0x100000100LL);
}

int Menu::CheckMIOS()
{
	u32 size = 0;
	u8 *appfile = ISFS_GetFile((u8*)"/title/00000001/00000101/content/0000000c.app", &size, 0);
	if(appfile)
	{
		for(u32 i = 0; i < size; ++i) 
		{
			if((*(vu32*)(appfile+i)) == 0x44494F53 && (*(vu32*)(appfile+i+5)) == 0x4D494F53) //DIOS MIOS
			{
				if(*(vu32*)(appfile+i+10) == 0x4C697465) //Lite
				{
					strncpy(MIOS_Info, fmt("DIOS-MIOS Lite %s\n", (char*)(appfile+i+20)), sizeof(MIOS_Info));
					MEM2_free(appfile);
					return 2;
				}
				strncpy(MIOS_Info, fmt("DIOS-MIOS %s\n", (char*)(appfile+i+20)), sizeof(MIOS_Info));
				MEM2_free(appfile);
				return 1;
			}
		}
		MEM2_free(appfile);
	}
	strncpy(MIOS_Info, "DIOS-MIOS (Lite) not found!\n \n", sizeof(MIOS_Info));
	return 0;
}

void Menu::UnMountDevices()
{
	DevHandler.UnMountSD();
	DevHandler.UnMountUSB();
}
