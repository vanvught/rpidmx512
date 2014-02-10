#ifndef BW_SPI_DIO_H_
#define BW_SPI_DIO_H_

#include <bw_dio.h>
#include <device_info.h>

#include <stdint.h>

#define BW_DIO_SPI_BYTE_WAIT_US				0

extern int bw_spi_dio_start(device_info_t *);
extern void bw_spi_dio_end(void);

extern void bw_spi_dio_fsel_mask(device_info_t *, uint8_t);
extern void bw_spi_dio_output(device_info_t *, uint8_t);
extern void bw_spi_dio_read_id(device_info_t *);

#endif /* BW_SPI_DIO_H_ */
