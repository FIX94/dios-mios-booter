#ifndef _FATMOUNTER_H_
#define _FATMOUNTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

enum
{
	SD = 0,
	USB,
	MAXDEVICES
};

static const char DeviceName[MAXDEVICES][6] =
{
	"sd",
	"usb"
};

int USBDevice_Init();
void USBDevice_deInit();
int SDCard_Init();
void SDCard_deInit();

#ifdef __cplusplus
}
#endif

#endif
