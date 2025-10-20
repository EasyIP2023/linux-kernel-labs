#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../uapi/serial-uart.h"

int
main(int argc, char *argv[])
{
	int err = -1, fd = 0;

	if (argc < 2) {
		fprintf(stdout, "Example:\n");
		fprintf(stdout, "\t%s /dev/serial-52AB99\n", argv[0]);
		return 1;
	}

	fd = open(argv[1], O_RDWR, 0666);
	if (fd == -1) {
		fprintf(stdout, "open: %s\n", strerror(errno));
		return 1;
	}

	err = ioctl(fd, SERIAL_RESET_COUNTER, &(size_t){0});
	if (err == -1) {
		close(fd);
		return 1;
	}

	fprintf(stdout, "serial-uart driver char counter has been reset.\n");

	close(fd);

	return 0;
}
