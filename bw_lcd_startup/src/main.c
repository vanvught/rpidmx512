#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <bw_lcd.h>
#include <bcm2835.h>

int main(int argc, char **argv) {
	int flags = 0;
	int addr = 0x00;

	while (1 + flags < argc && argv[1 + flags][0] == '-') {
		switch (argv[1 + flags][1]) {
		case 'a':
			if (2 + flags < argc) {
				sscanf (argv[2 + flags], "%x", &addr);
			} else {
				fprintf(stderr, "Warning: No slave address given. The default is used.\n");
			}
			break;
		default:
			fprintf(stderr, "Warning: Unsupported flag \"-%c\"!\n", argv[1 + flags][1]);
			exit(1);
		}
		flags++;
	}

	struct ifreq ifr;
	char iface[] = "eth0";
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);

	char buf[BW_LCD_MAX_CHARACTERS];
	int l = snprintf(buf, BW_LCD_MAX_CHARACTERS, "%s", inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));
	printf("%d:[%s]\n", l, buf);

	if (lcd_start(addr) != BW_LCD_OK) {
		fprintf(stderr, "Cannot start LCD\n");
		exit(EXIT_FAILURE);
	}

	lcd_cls();
	lcd_text_line_1("Raspberry Pi", 12);
	lcd_text_line_2(buf, l);
	lcd_end();

	return 0;
}
