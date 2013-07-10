// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the HIDCLI_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// HIDCLI_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifndef __HID_CLI_H__
#define __HID_CLI_H__

#ifdef _WINDOWS_
#include <windows.h>
#else
#include <stdlib.h>
#endif

#ifdef _WINDOWS_

#ifdef HIDCLI_EXPORTS
#define HIDCLI_API __declspec(dllexport)
#else
#define HIDCLI_API __declspec(dllimport)
#endif

#else
#define HIDCLI_API
#endif

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef long int hid_dev_t;

HIDCLI_API hid_dev_t hid_device_open(short vid, short pid);
HIDCLI_API int hid_device_close(hid_dev_t handle);
HIDCLI_API int hid_device_write(hid_dev_t handle, void *b, size_t count);

#if defined(__cplusplus) || defined(c_plusplus)
} /* extern "C" */
#endif

#endif /* __HID_CLI_H__ */
