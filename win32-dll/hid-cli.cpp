// hid-cli.cpp : Defines the exported functions for the DLL application.
//

#include <windows.h>
#include <hid-cli.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

	#include <stdio.h>
	#include <setupapi.h>
	#include <hidsdi.h>

	inline GUID get_guid()
	{
		GUID hid_guid;
		HidD_GetHidGuid(&hid_guid);
		return hid_guid;
	}

#if OVERLAPPED_ENABLE
	static OVERLAPPED *hid_overlapped = NULL;
#endif

	HIDCLI_API hid_dev_t hid_device_open(short vid, short pid)
	{
		SP_DEVICE_INTERFACE_DATA dev_iface = { cbSize : sizeof(SP_DEVICE_INTERFACE_DATA)};
		SP_DEVINFO_DATA dev_info_data = { cbSize : sizeof(SP_DEVINFO_DATA) };
		PSP_DEVICE_INTERFACE_DETAIL_DATA dev_detail_data;

		BOOL result;

		GUID hid_guid = get_guid();

		HDEVINFO hdl_dev_info = SetupDiGetClassDevs(&hid_guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (INVALID_HANDLE_VALUE == hdl_dev_info)
			return (hid_dev_t)INVALID_HANDLE_VALUE;

		unsigned i = 0;
		TCHAR *buffer;
		do {
			result = SetupDiEnumDeviceInfo(hdl_dev_info, i, &dev_info_data);
			if (result)
				result = SetupDiEnumDeviceInterfaces(hdl_dev_info, NULL, &hid_guid, i, &dev_iface);

			if (!result) {
				SetupDiDestroyDeviceInfoList(hdl_dev_info);
				return (hid_dev_t)INVALID_HANDLE_VALUE;
			}

			i++;

			DWORD required = 0;
			BOOL b = SetupDiGetDeviceInterfaceDetail(hdl_dev_info, &dev_iface, NULL, 0, &required, NULL);

			buffer = new TCHAR[required];
			dev_detail_data = (PSP_DEVICE_INTERFACE_DETAIL_DATA)buffer;
			dev_detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (SetupDiGetDeviceInterfaceDetail(hdl_dev_info, &dev_iface, dev_detail_data, required, NULL, NULL)) {
#if OVERLAPPED_ENABLE
				HANDLE hid_dev = CreateFile(dev_detail_data->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
#else
				HANDLE hid_dev = CreateFile(dev_detail_data->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif

#if DEBUG
				printf("device path %s\n", dev_detail_data->DevicePath);
#endif
				if (INVALID_HANDLE_VALUE != hid_dev) {
					HIDD_ATTRIBUTES attributes;
					HidD_GetAttributes(hid_dev, &attributes);
#if DEBUG
					printf("%d - %d\n", attributes.ProductID, attributes.VendorID);
#endif
					if ((attributes.ProductID == pid) && (attributes.VendorID == vid)) {
						delete(buffer);
#if OVERLAPPED_ENABLE
						hid_overlapped = new OVERLAPPED;
						hid_overlapped->Internal = 0;
						hid_overlapped->InternalHigh = 0;
						hid_overlapped->Offset = 0;
						hid_overlapped->OffsetHigh = 0;
						hid_overlapped->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
						ResetEvent(hid_overlapped->hEvent);
#endif
						return (hid_dev_t)hid_dev;
					}
					hid_dev = INVALID_HANDLE_VALUE;
				}
			}
			delete(buffer);
		} while(1);

		return (hid_dev_t)INVALID_HANDLE_VALUE;
	}

	HIDCLI_API int hid_device_close(hid_dev_t handle)
	{
		CloseHandle((HANDLE)handle);
#if OVERLAPPED_ENABLE
		ResetEvent(hid_overlapped->hEvent);
		CloseHandle(hid_overlapped->hEvent);
		delete(hid_overlapped);
#endif
	}

	HIDCLI_API int hid_device_write(hid_dev_t handle, void *b, size_t count)
	{
		DWORD bytes_written = 0;
		unsigned char *hid_report;
		size_t hid_report_size = count + 1;

		hid_report = (unsigned char*)malloc(hid_report_size * sizeof(unsigned char));
		hid_report[0] = 0x0;
		memcpy(hid_report + 1, b, count);

#if OVERLAPPED_ENABLE
		BOOL res = WriteFile((HANDLE)handle, hid_report, hid_report_size, &bytes_written, hid_overlapped);
#else
		BOOL res = WriteFile((HANDLE)handle, (void*)hid_report, hid_report_size, &bytes_written, NULL);
#endif
		if (res) {
			Sleep(32);
		}

		free(hid_report);
#if DEBUG
		DWORD err_code;
		LPVOID msg_buffer;

		err_code = GetLastError();
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			err_code,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &msg_buffer,
			0, NULL);
		printf((const char*)msg_buffer);
		LocalFree(msg_buffer);
		printf("Write File ret %s\n", res ? "True" : "False");
#endif
		return (int)(bytes_written - 1);
	}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
