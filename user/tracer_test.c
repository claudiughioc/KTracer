#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../src/tr_ioctl.h"

#define DEVICE_PATH	"/dev/tracer"

/*
 * prints error message and exits
 */

static void error(const char *message)
{
	perror(message);
	exit(EXIT_FAILURE);
}

/*
 * print use case
 */

static void usage(const char *argv0)
{
	printf("Usage: %s <options>\n options:\n"
			"\tp - print\n"
			"\ts string - set buffer\n"
			"\tg - get buffer\n"
			"\td - down\n"
			"\tu - up\n", argv0);
	exit(EXIT_FAILURE);
}


int main(int argc, char **argv)
{
	int fd;
	int pid = 777;

	if (argc < 3)
		usage(argv[0]);
	pid = atoi(argv[2]);

	if (strlen(argv[1]) != 1)
		usage(argv[0]);

	fd = open(DEVICE_PATH, O_RDONLY);

	switch (argv[1][0]) {
	case 'a':
		if(ioctl(fd, TRACER_ADD_PROCESS, pid) < 0)
			error("Error on ioctl\n");
		break;
	case 'r':
		if(ioctl(fd, TRACER_REMOVE_PROCESS, pid) < 0)
			error("Error on ioctl\n");
		break;
	default:
		error("Wrong parameter");
	}

	close(fd);

	return 0;
}
