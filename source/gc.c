#include <gccore.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include "gc.h"
#include "defines.h"

syssram* __SYS_LockSram();
u32 __SYS_UnlockSram(u32 write);
u32 __SYS_SyncSram(void);

void GC_SetVideoMode(u8 videomode, DML_CFG *BooterCFG)
{
	syssram *sram;
	sram = __SYS_LockSram();
	static GXRModeObj *rmode;
	int memflag = 0;

	if((VIDEO_HaveComponentCable() && (CONF_GetProgressiveScan() > 0)) || videomode > 3)
		sram->flags |= 0x80; //set progressive flag
	else
		sram->flags &= 0x7F; //clear progressive flag

	if(videomode == 1 || videomode == 3 || videomode == 5)
	{
		memflag = 1;
		sram->flags |= 0x01; // Set bit 0 to set the video mode to PAL
		sram->ntd |= 0x40; //set pal60 flag
	}
	else
	{
		sram->flags &= 0xFE; // Clear bit 0 to set the video mode to NTSC
		sram->ntd &= 0xBF; //clear pal60 flag
	}

	if(videomode == 1)
	{
		BooterCFG->VideoMode |= DML_VID_FORCE_PAL50;
		rmode = &TVPal528IntDf;
	}
	else if(videomode == 2)
	{
		BooterCFG->VideoMode |= DML_VID_FORCE_NTSC;
		rmode = &TVNtsc480IntDf;
	}
	else if(videomode == 3)
	{
		BooterCFG->VideoMode |= DML_VID_FORCE_PAL60;
		rmode = &TVEurgb60Hz480IntDf;
		memflag = 5;
	}
	else if(videomode == 4 ||videomode == 6)
	{
		BooterCFG->VideoMode |= DML_VID_FORCE_PROG;
		rmode = &TVNtsc480Prog;
	}
	else if(videomode == 5 || videomode == 7)
	{
		BooterCFG->VideoMode |= DML_VID_FORCE_PROG;
		rmode = &TVEurgb60Hz480Prog;
		memflag = 5;
	}

	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram());

	/* Set video mode register */
	*(vu32 *)0x800000CC = memflag;
	DCFlushRange((void *)(0x800000CC), 4);

	/* Set video mode */
	if (rmode != 0)
		VIDEO_Configure(rmode);

	/* Setup video  */
	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
}

u8 get_wii_language()
{
	switch (CONF_GetLanguage())
	{
		case CONF_LANG_GERMAN:
			return SRAM_GERMAN;
		case CONF_LANG_FRENCH:
			return SRAM_FRENCH;
		case CONF_LANG_SPANISH:
			return SRAM_SPANISH;
		case CONF_LANG_ITALIAN:
			return SRAM_ITALIAN;
		case CONF_LANG_DUTCH:
			return SRAM_DUTCH;
		default:
			return SRAM_ENGLISH;
	}
}

void GC_SetLanguage(u8 lang)
{
	if (lang == 0)
		lang = get_wii_language();
	else
		lang--;

	syssram *sram;
	sram = __SYS_LockSram();
	sram->lang = lang;

	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram());
}

void DML_New_WriteOptions(DML_CFG *DMLCfg)
{
	if(DMLCfg == NULL)
		return;

	//Write options into memory
	memcpy((void *)0x80001700, DMLCfg, sizeof(DML_CFG));
	DCFlushRange((void *)(0x80001700), sizeof(DML_CFG));

	//DML v1.2+
	memcpy((void *)0x81200000, DMLCfg, sizeof(DML_CFG));
	DCFlushRange((void *)(0x81200000), sizeof(DML_CFG));
}

void DML_Old_SetOptions(const char *GameID)
{
	FILE *f;
	f = fopen("sd:/games/boot.bin", "wb");
	fwrite(GameID, 1, 6, f);
	fclose(f);

	//Tell DML to boot the game from sd card
	*(vu32*)0x80001800 = 0xB002D105;
	DCFlushRange((void *)(0x80001800), 4);

	*(vu32*)0xCC003024 |= 7;
}
