#include <stdint.h>
#include <stdbool.h>

#include <esp_common.h>

#include <esp8266/eagle_soc.h>
#include <esp8266/uart_register.h>

#define UART0 0

void __attribute__((section(".irom0.text"))) uart0_putc(char c) {
	while (true) {
		uint32 fifo_cnt = READ_PERI_REG(UART_STATUS(UART0)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);
		if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126) {
			break;
		}
	}

	WRITE_PERI_REG(UART_FIFO(UART0), c);
	return;
}

int __attribute__((section(".irom0.text"))) uart0_puts(const char *s) {
	char c;
	int i = 0;;

	while ((c = *s++) != (char) 0) {
		i++;
		uart0_putc(c);
	}

	return i;
}


