/****************************************************************************
 * DIOS-MIOS Booter - A small and easy DIOS-MIOS (Lite) Game Booter
 * Copyright (C) 2012 FIX94
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
#include "../fileOps.h"

void Menu::GameOptionsMenu()
{
	u8 tmppos = position;
	position += (listposition - 1);
	string gameID = List.GetEntryID(position - 1);
	string gameName = List.GetEntryName(position - 1);
	string gamePath = List.GetEntryPath(position - 1);

	while(!Sys_Exit())
	{
		MainLoop();
		printf("Game Operations\nPress the B Button to return to game selection.\nCurrent Game: %s\n \n", gameName.c_str());
		printf("Press the -(L) Button to to delete the game%s\n", currentDev == 1 ? "\nor the +(R) Button to copy the game from USB to SD." : ".\n");
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_B) || (PAD_ButtonsDown(0) == PAD_BUTTON_B))
			break;
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_MINUS) || (PAD_ButtonsDown(0) == PAD_TRIGGER_L))
		{
			printf("WARNING: The game will be deleted forever and cant be restored. \nPress the A Button to continue or the B Button to abort.\n");
			if(!AbortOperation())
			{
				char source[1024];
				if(strcasestr(gamePath.c_str(), "boot.bin") != NULL)
					gamePath.erase(gamePath.size() - 9, 9);
				snprintf(source, sizeof(source), "%s:/games/%s", DeviceName[currentDev], gamePath.c_str());
				printf("Deleting folder: %s\n", source);
				VIDEO_WaitVSync();
				fsop_deleteFolder(source);
				listposition = 1;
				tmppos = 1;
				ReadGameDir();
			}
			continue;
		}
		if(((WPAD_ButtonsDown(0) == WPAD_BUTTON_PLUS) || (PAD_ButtonsDown(0) == PAD_TRIGGER_R)) && currentDev == 1)
		{
			printf("WARNING: Copying a game from USB to SD can take a few minutes and cannot be cancelled. \nPress the A Button to continue or the B Button to abort.\n");
			if(!AbortOperation())
			{
				currentDev = 0;
				ReadGameDir();
				bool gameOnSD = false;
				for(u8 i = 1; i < List.GetEntrySize(); i++)
				{
					if(strncasecmp(List.GetEntryID(i - 1), gameID.c_str(), 6) == 0)
						gameOnSD = true;
				}
				if(!gameOnSD)
				{
					char source[1024];
					char target[1024];
					if(strcasestr(gamePath.c_str(), "boot.bin") != NULL)
						gamePath.erase(gamePath.size() - 9, 9);
					snprintf(source, sizeof(source), "usb:/games/%s", gamePath.c_str());
					snprintf(target, sizeof(target), "sd:/games/%s", gamePath.c_str());
					if(fsop_GetFreeSpaceKb((char*)"sd:/") >= fsop_GetFolderKb(source))
						fsop_CopyFolder(source, target, gameName.c_str(), gameID.c_str(), MIOS_Info);
				}
				position = tmppos;
				currentDev = 1;
				ReadGameDir();
			}
			continue;
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) == PAD_BUTTON_START))
			Input_Reset();
	}
	position = tmppos;
	currentMenu = MENU_MAIN;
}
