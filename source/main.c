#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <fat.h>
#include <sdcard/wiisd_io.h>
#include "gc.h"
#include "sdhc.h"

#define IOCTL_DI_STOPMOTOR	0xE3
#define IOCTL_DI_RESET		0x8A

/* Variables */
static u32 inbuf[8]  ATTRIBUTE_ALIGN(32);
static u32 outbuf[8] ATTRIBUTE_ALIGN(32);
static s32 di_fd = -1;

static const char ID[6] = "GZLP01";

int main() 
{	
	/* Mount SD Card */
	DISC_INTERFACE storage = __io_wiisd;
	if (fatMountSimple("sd", &storage) < 0)
		return -1;

	/* Init Video */
	VIDEO_Init();
	if(ID[3] == 'P')
		set_video_mode(1);
	else
		set_video_mode(0);
	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	/* Set GC Lanugage */
	set_language(0);

	/* Create boot.bin */
    FILE * f = fopen("sd:/games/boot.bin" ,"wb");
	fwrite(ID, 1, 6, f);
	fclose(f);

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

	/* Tell DML to boot the game from sd card */
	*(vu32*)0x80001800 = 0xB002D105;
	DCFlushRange((void *)(0x80001800), 4);
	ICInvalidateRange((void *)(0x80001800), 4);
	*(vu32*)0xCC003024 |= 7;

	/* Boot BC */
	return WII_LaunchTitle(0x100000100LL);
}
