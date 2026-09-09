#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdlib.h>

void shift_leds(int32_t fd, uint8_t val, int32_t pos) {
	uint8_t *buff = malloc(pos);
	for(int i=0;i<pos;i++) {
		buff[i] = val;
	}
	struct spi_ioc_transfer tr = {0};

	tr.tx_buf = (unsigned long)buff;
	tr.rx_buf = 0;
	tr.len = pos;
	tr.speed_hz = 1000000;
	tr.bits_per_word = 8;

	int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if(ret < 1) {
		perror("Transfer error");
	}
}

int main(void) {
	int rotd = open("/dev/input/event0", O_RDONLY);
	int spid = open("/dev/spidev0.0", O_RDWR);
	if (spid < 0) {
		perror("open");
		return 1;
	}

	if (rotd < 0) {
		perror("open");
		return 1;
	}

	int pos = 0;
	struct input_event ev;

	shift_leds(spid, 0, 5);
	shift_leds(spid, 0, 5);
	shift_leds(spid, 0, 5);
	shift_leds(spid, 0, 5);

	int prev_pos = 0;

	while(read(rotd, &ev, sizeof(ev)) == sizeof(ev)) {
		if(ev.type == EV_REL && ev.code == REL_X) {
			pos += ev.value;
			if(pos < 1)
				pos = 1;
			if(pos > 18)
				pos = 18;
			if(prev_pos < pos) {
				shift_leds(spid, 0xFF, 1);
			}
			else {
				shift_leds(spid, 0, 5);
				shift_leds(spid, 0, 5);
				shift_leds(spid, 0, 5);
				shift_leds(spid, 0, 5);
				shift_leds(spid, 0, 5);
				shift_leds(spid, 0, 5);
				shift_leds(spid, 0, 5);
				shift_leds(spid, 0, 5);
				shift_leds(spid, 0xFF, pos);
				shift_leds(spid, 0xFF, pos);
			}

			printf("position: %d\n", pos);
			prev_pos = pos;
		}
	}

	close(spid);
	close(rotd);
	return 0;
}
