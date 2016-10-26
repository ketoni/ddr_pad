
/* WIP
 * TODO: Map keys, Simulate keystrokes, maybe a signal handler for exit ? */


#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

//#define DEBUG_COM
#define DEBUG_BTN

int main(int argc, char **argv)
{
    char *device = "/dev/hidraw0";
	
	int fd, res;
    uint8_t buf[12], byte = 0x00;
	unsigned sleep_us = 0, run = 0, cal_runs = 1000;
	
	memset(buf, 0x0, sizeof(buf));

	if (argc > 1)
    	device = argv[1];

    fd = open(device, O_RDONLY|O_NONBLOCK);
	if (fd < 0) {
		printf("Unable to open device '%s':\n", device);
    	perror("");
    	return 1;
    }
	printf("Connected to device '%s'\n", device);
	sleep(2);

	puts("Calibrating ...");
	while (run++ < cal_runs) {
		printf("\r%0.0f%% ", ((double) run/ (double) cal_runs) * 100);
		fflush(stdout);
		res = read(fd, buf, 11);
		if (res < 0) {
			sleep_us += 100;
		}
		usleep(sleep_us);
	}
	printf("\rDone! Read interval is %d us\n", sleep_us);

	while (1) {

		res = read(fd, buf, 11);
		if (res < 0) {
			perror("read");
		} else {
			
			#ifdef DEBUG_COM
			unsigned i, j;
			printf("read() read %d bytes, %d us interval:\n\t", res, sleep_us);
			for (i = 0; i < res; i++)
				printf("%hhx ", buf[i]);
			printf("\n\t");
			for (i = 0; i < 2; i++) {
				printf("   ");
				for (j = 0; j < 8; j++) {
					if (buf[i] & 1 << j) {
						printf("1");
					} else {
						printf("0");
					}
			 	}
			}
			puts("\n");
			#endif

			#ifdef DEBUG_BTN
			byte = buf[0];		
			if (byte & 0x01) puts("SQUARE");
			if (byte & 0x02) puts("CROSS");
			if (byte & 0x04) puts("CIRCLE");
			if (byte & 0x08) puts("TRIANGLE");

			byte = buf[1];
			if (byte & 0x01) puts("SELECT");
			if (byte & 0x02) puts("START");

			if (buf[7]) puts("RIGHT");
			if (buf[8]) puts("LEFT");
			if (buf[9]) puts("UP");
			if (buf[10]) puts("DOWN");


			#endif
		}
		usleep(sleep_us);
	}
	
	close(fd);
	return 0;
}


