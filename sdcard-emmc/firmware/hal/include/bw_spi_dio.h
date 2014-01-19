#ifndef BW_SPI_DIO_H_
#define BW_SPI_DIO_H_

#include <bw.h>

extern int bw_spi_dio_start (device_info_t *);
extern void bw_spi_dio_end (void);

extern void bw_spi_dio_fsel_mask(device_info_t, unsigned char);
extern void bw_spi_dio_output(device_info_t, unsigned char);
extern void bw_spi_dio_read_id(device_info_t);

#endif /* BW_SPI_DIO_H_ */
