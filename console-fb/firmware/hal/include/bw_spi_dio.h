#ifndef BW_SPI_DIO_H_
#define BW_SPI_DIO_H_

extern int bw_spi_dio_start (char);
extern void bw_spi_dio_end (void);

extern void bw_spi_dio_fsel_mask(unsigned char);
extern void bw_spi_dio_output(unsigned char);
extern void bw_spi_dio_read_id(void);

#endif /* BW_SPI_DIO_H_ */
