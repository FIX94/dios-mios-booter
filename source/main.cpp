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
#include "Menu/Menu.hpp"
#include "Memory/mem2.hpp"

using namespace std;

extern "C" 
{
	extern bool shutdown;
	extern void __exception_setreload(int t);
}

void InitVideo()
{
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
}

int main() 
{
	__exception_setreload(10);

	InitVideo();

	MagicPatches(1); //It's a kind of magic :P

	MEM2_init(48);
	ISFS_Initialize();

	Open_Inputs();
	Sys_Init();

	Menu menu;
	menu.Start();

	Close_Inputs();
	ISFS_Deinitialize();
	MagicPatches(0);

	if(Sys_Exit())
	{
		menu.UnMountDevices();
		if(shutdown)
			SYS_ResetSystem(SYS_POWEROFF, 0, 0);
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
		return 0;
	}
	return menu.BootGame();
}
