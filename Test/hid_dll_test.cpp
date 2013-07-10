/**
 * Windows Test
 */

#include <cstdio>
#include <hid-cli.h>

int main(int argc, char **argv)
{
	char d[] = {0x0, 0x0, 0xf, 0x0};
	int bytesWritten;
	hid_dev_t handler = hid_device_open(0x0d8c, 0x000c);
	for (int i = 0; i < 10; i++) {
		d[1] = (char)i;
		bytesWritten = hid_device_write(handler, (void*)&d, 4);
		printf("Byte Wrote %x.\n", bytesWritten);
	}
	hid_device_close(handler);
}
