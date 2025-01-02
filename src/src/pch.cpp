// Portions copyright (c) 2005 Intel Corporation, all rights reserved

#include "cube.h"

// Begin Intel Corporation code
#ifdef _WIN32_WCE

#include <winioctl.h>

// in case a soft reset is needed in software
#define IOCTL_HAL_REBOOT CTL_CODE(FILE_DEVICE_HAL, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)

extern "C" __declspec(dllimport) BOOL KernelIoControl(
	DWORD dwIoControlCode, 
	LPVOID lpInBuf, 
	DWORD nInBufSize, 
	LPVOID lpOutBuf, 
	DWORD nOutBufSize, 
	LPDWORD lpBytesReturned);

__inline BOOL ResetPocketPC()
{
	// soft reset
	return KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);
}
#endif /* _WIN32_WCE */
// End Intel Corporation code