#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include "gc.h"
#include "sdhc.h"
#include "defines.h"

#ifdef OLD_DML
#include <fat.h>
#include <sdcard/wiisd_io.h>
#endif

#define IOCTL_DI_STOPMOTOR	0xE3
#define IOCTL_DI_RESET		0x8A

/* Variables */
static u32 inbuf[8]  ATTRIBUTE_ALIGN(32);
static u32 outbuf[8] ATTRIBUTE_ALIGN(32);
static s32 di_fd = -1;

int main() 
{
	#ifdef OLD_DML
		/* Mount SD Card */
		DISC_INTERFACE storage = __io_wiisd;
		if (fatMountSimple("sd", &storage) < 0)
			return -1;
	#endif

	/* Set Video Mode */
	#ifdef PAL_VIDEO_MODE
		GC_SetVideoMode(1);
	#else
		GC_SetVideoMode(2);
	#endif

	/* Set GC Lanugage */
	GC_SetLanguage(0);

	#ifdef OLD_DML
		DML_Old_SetOptions(Game);
		/* Unmount SD Card */
		fatUnmount("sd");
		storage.shutdown();
	#else
		DML_New_SetOptions(Game);
	#endif

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

	/* Boot BC */
	return WII_LaunchTitle(0x100000100LL);
}
