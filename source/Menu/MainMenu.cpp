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
#include "Menu.hpp"
#include "Memory/mem2.hpp"
#include "defines.h"

static u32 listsize = 12;

void Menu::InitMain()
{
	InitOptions();
	DevHandler.Init();
	CheckMIOS();

	BooterCFG = (DML_CFG*)MEM2_alloc(sizeof(DML_CFG));
	memset(BooterCFG, 0, sizeof(DML_CFG));

	BooterCFG->Magicbytes = 0xD1050CF6;
	BooterCFG->CfgVersion = 0x00000001;
	BooterCFG->VideoMode |= DML_VID_FORCE;

	currentDev = SD;
	currentMenu = MENU_MAIN;

	DriveReset = true;
	OldDML = false;
	NTSCJ_Patch = false;

	for(u8 i = 0; i < 76; i++)
		listlimits[i] = '#';
	listlimits[76] = '\0';

#ifdef AUTOBOOT
	Autoboot = true;
#else
	Autoboot = false;
#endif

	MainMenuAutoboot = Autoboot;

	position = 0;
	listposition = 1;
	AutobootTimeout = 0;

	if(DevHandler.MountSD())
		BooterINI.load("sd:/dios-mios-booter.ini");
	else if(DevHandler.MountUSB())
		BooterINI.load("usb:/dios-mios-booter.ini");

	if(!Autoboot)
		ReadConfig("GENERAL");
	else
		ReadConfig(AUTOBOOT_GAME_ID);
	if(currentDev && !DevHandler.USB_Mounted())
		DevHandler.MountUSB();

	ReadGameDir();
	if(Autoboot)
	{
		for(u8 i = 0; i < List.GetEntrySize(); i++)
		{
			if(strcmp(List.GetEntryID(i), AUTOBOOT_GAME_ID) == 0)
			{
				position = i + 1;
				break;
			}
		}
		if(position == 0)
		{
			string AutoID(AUTOBOOT_GAME_ID);
			AutoID.erase(3, 1);
			for(u8 i = 0; i < List.GetEntrySize(); i++)
			{
				string CurrentID(List.GetEntryID(i));
				CurrentID.erase(3, 1);
				if(strcmp(CurrentID.c_str(), AutoID.c_str()) == 0)
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
}

void Menu::MainMenu()
{
	while(!Sys_Exit())
	{
		MainLoop();
		if(MainMenuAutoboot)
		{
			if(!AutobootTimeout)
				AutobootTimeout = time(NULL) + 3; //3 Seconds
			printf("Autoboot requested!\nPress any button to abort... %i\n", int(AutobootTimeout - time(NULL)));
			if(WPAD_ButtonsDown(0) || PAD_ButtonsDown(0))
				MainMenuAutoboot = false;
			if(time(NULL) >= AutobootTimeout)
				break;
			VIDEO_WaitVSync();
			continue;
		}
		else
		{
			printf("Press the HOME(Start) Button to exit, \nthe A Button to boot the selected Game \nor the B Button to enter the Options. \n \n");
			printf("Press the -(L) Button to to switch the Device \nor the 1(X) Button to enter the Game Operations.\nCurrent Device: %s\n \n", DeviceName[currentDev]);
		}
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
		{
			currentMenu = MENU_NONE;
			break;
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_B) || (PAD_ButtonsDown(0) == PAD_BUTTON_B))
		{
			currentMenu = MENU_OPTIONS;
			break;
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_1) || (PAD_ButtonsDown(0) == PAD_BUTTON_X))
		{
			currentMenu = MENU_GAME_OPTIONS;
			break;
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_MINUS) || (PAD_ButtonsDown(0) == PAD_TRIGGER_L))
		{
			currentDev = (currentDev == 0) ? 1 : 0;
			WriteConfig("GENERAL");
			if(currentDev && !DevHandler.USB_Mounted())
				DevHandler.MountUSB();
			ReadGameDir();
			position = 1;
			listposition = 1;
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) == PAD_BUTTON_START))
			Input_Reset();
		if(position == 0)
		{
			if(List.GetEntrySize() > listsize)
			{
				listposition = listsize;
				position = List.GetEntrySize() - (listposition - 1);
			}
			else
			{
				listposition = List.GetEntrySize();
				position = 1;
			}
		}
		if(position + (listposition - 1) > (u8)List.GetEntrySize())
		{
			listposition = 1;
			position = 1;
		}
		printf(listlimits);
		for(u8 i = 0; i < listsize; i++)
		{
			if(listposition - 1 == i)
				printf("\x1b[33m");
			else
				printf("\x1b[37m");
			if(position - 1 + i < (u8)List.GetEntrySize())
				printf("%s\n", List.GetEntryName(position - 1 + i));
			else
				printf("\n");
		}
		printf("\x1b[37m");
		printf(listlimits);
	}
}
