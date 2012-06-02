
#include <wiiuse/wpad.h>

bool reset = false;
bool shutdown = false;

void __Wpad_PowerCallback()
{
	/* Poweroff console */
	shutdown = 1;
}

void __Sys_ResetCallback(void)
{
	reset = true;
}

void __Sys_PowerCallback(void)
{
	shutdown = true;
}

void Sys_Init(void)
{
	/* Set RESET/POWER button callback */
	SYS_SetResetCallback(__Sys_ResetCallback);
	SYS_SetPowerCallback(__Sys_PowerCallback);
}

void Open_Inputs(void)
{
	/* Initialize Wiimote subsystem */
	PAD_Init();
	WPAD_Init();

	/* Set POWER button callback */
	WPAD_SetPowerButtonCallback(__Wpad_PowerCallback);
	
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC);
}

void Close_Inputs(void)
{
	/* Disconnect Wiimote */
	WPAD_Disconnect(0);

	/* Shutdown Wiimote subsystem */
	WPAD_Shutdown();
}
