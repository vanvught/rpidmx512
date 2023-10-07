/*
 * emac.h
 */

#ifndef EMAC_EMAC_H_
#define EMAC_EMAC_H_

#include <cstdint>

/** \defgroup platform Platform implementation
  @{
*/
/**
 * Configure the PHY interface
 * - Call \ref net::phy_config
 */
void emac_config();

/**
 *
 * - Soft MAC reset
 * - Set the MAC address
 * - Initialize rx/tx descriptors
 * - PHY Start Up -> \ref net::phy_start
 * - Adjust the link with duplex and speed returned from \ref net::phy_start
 * - Start RX/TX DMA
 * - Enable RX/TX
 *
 * @param[out] macAddress
 *
 */
void emac_start(uint8_t macAddress[], net::Link& link);
/** @} */

#endif /* EMAC_EMAC_H_ */
