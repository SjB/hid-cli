// hid-cli.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "hid-cli.h"

#ifdef  __cplusplus
extern "C" {
#endif

	#include <setupapi.h>
	#include <hidsdi.h>

	inline GUID get_guid()
	{
		GUID hid_guid;
		HidD_GetHidGuid(&hid_guid);
		return hid_guid;
	}

	HIDCLI_API void* hid_device_open(short vid, short pid)
	{
		GUID hid_guid = get_guid();

		HANDLE info = SetupDiGetClassDevs(&hid_guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (INVALID_HANDLE_VALUE == info)
			return INVALID_HANDLE_VALUE;

		unsigned i = 0;
		while(1) {
			SP_DEVICE_INTERFACE_DATA dev_iface = { cbSize : sizeof(SP_DEVICE_INTERFACE_DATA)};
			if (!SetupDiEnumDeviceInterfaces(info, NULL, &hid_guid, i, &dev_iface))
				break;
			++i;

			DWORD size;
			if (!SetupDiGetDeviceInterfaceDetail(info, &dev_iface, NULL, 0, &size, NULL) && (122 != GetLastError()))
				break;

			SP_DEVICE_INTERFACE_DETAIL_DATA detail;
			detail.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			if (SetupDiGetDeviceInterfaceDetail(info, &dev_iface, &detail, size, NULL, NULL)) {
				HANDLE hid_dev = CreateFile(detail.DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
				if (INVALID_HANDLE_VALUE != hid_dev) {
					HIDD_ATTRIBUTES attributes;
					HidD_GetAttributes(hid_dev, &attributes);

					if ((attributes.ProductID == pid) && (attributes.VendorID == vid))
						return hid_dev;

					CloseHandle(hid_dev);
					hid_dev = INVALID_HANDLE_VALUE;
				}
			}
		}
		return INVALID_HANDLE_VALUE;
	}

	HIDCLI_API int hid_device_close(void* fd)
	{
		CloseHandle((void*)fd);
	}

	HIDCLI_API int hid_device_write(void* fd, void *b, size_t count)
	{
		DWORD bytesWritten = 0;
		WriteFile(fd, b, (DWORD)count, &bytesWritten, NULL);
		return (int)bytesWritten;
	}

#ifdef  __cplusplus
}
#endif
