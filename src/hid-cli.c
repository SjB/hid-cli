//
// hid-cli.c
//
// Author:
//       Steve Beaulac <steve@nca.uwo.ca>
//
// Copyright (c) 2009 The National Centre for Audiology
//
// Licensed under The National Centre for Audiology, Version 1.0
// (the "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License by contacting
//
// The National Centre for Audiology
// http://www.uwo.ca/nca
//
// Unless required by applicable law or agreed to in writing
// software distributed under the License is distributed on an "AS IS"
// BASIS WITHOUT WARRANTIEDS OR CONDITIONS OF ANY KIND. either express
// or implied.  See the License for the specific language governing
// permissions and limitations under the licenses.
//

#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <linux/hidraw.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#if defined(DEBUG)
#define dbg(format, arg...)	\
	fprintf(stderr, format, ## arg)
#else
#define dbg(format, arg...)	\
	({while (0) {}; })
#endif

static int match_hidraw(int fd, short vid, short pid)
{
	struct hidraw_devinfo dinfo;
	int ret;

	if (0 != ioctl(fd, HIDIOCGRAWINFO, &dinfo)) {
		dbg("Can't open RAWINFO");
		return -EINVAL;
	}
	dbg("VID: %2x, PID: %2x\n", 0xffff & dinfo.vendor, 0xffff & dinfo.product);
	if ((dinfo.vendor == vid) && (dinfo.product == pid)) {
		return 0;
	}
	return -ENODEV;
}

static int find_hidraw_device(short vid, short pid, char *hidraw_path)
{
	int i,j, found, fd;
	char dev_path[PATH_MAX];

	struct hidraw_devinfo dev_info;
	const char *hiddev[] = {"/dev/hidraw", NULL};

	for (i = 0; hiddev[i]; i++) {
		j = 0;
		do {
			sprintf(dev_path, "%s%d", hiddev[i], j++);

			// Break if device file doesn't exists
			if (-1 == access(dev_path, F_OK))
					break;

			if ((fd = open(dev_path, O_RDONLY | O_NOCTTY)) < 0) {
				dbg("Can't open hidraw device %s\n", dev_path);
				continue;
			}

			found = match_hidraw(fd, vid, pid);
			close(fd);

			if (0 == found) {
				strncpy(hidraw_path, dev_path, PATH_MAX);
				return 0;
			}

		} while (1);
	}
	return -1;
}

int hid_device_open(short vid, short pid)
{
	int fd;
	char dev_path[PATH_MAX];

	if (0 == find_hidraw_device(vid, pid, dev_path)) {
		dbg("found devide : %s\n", dev_path);
		fd = open(dev_path, O_RDWR | O_NOCTTY);
		if (-1 != fd)
			return fd;
	}
	return -ENODEV;
}

int hid_device_close(int fd)
{
	return close(fd);
}

int hid_device_write(int fd, void *b, size_t count)
{
	unsigned char *mesg;
	size_t mesg_cnt = 1 + count;
	struct timespec req = {0, 32000};
	int ret;

	mesg = (unsigned char *)malloc(mesg_cnt);
	mesg[0] = 0x0;
	memcpy(&(mesg[1]), b, count);

	if ((count = write(fd, mesg, mesg_cnt)) == mesg_cnt) {
		ret = nanosleep(&req, NULL);
		return count;
	}
	return 0;
}
