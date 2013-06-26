// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the HIDCLI_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// HIDCLI_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef HIDCLI_EXPORTS
#define HIDCLI_API __declspec(dllexport)
#else
#define HIDCLI_API __declspec(dllimport)
#endif

#ifdef  __cplusplus
extern "C" {
#endif

HIDCLI_API void* hid_device_open(short vid, short pid);
HIDCLI_API int hid_device_close(void* fd);
HIDCLI_API int hid_device_write(void* fd, void *b, size_t count);

#ifdef  __cplusplus
}
#endif
