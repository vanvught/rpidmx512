/**
 * @file mmi.h
 *
 */
/* Copyright (C) 2023-2025 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef EMAC_MMI_H_
#define EMAC_MMI_H_

#include <cstdint>

namespace net::mmi
{
/* Generic MII registers. */
inline constexpr uint32_t REG_BMCR = 0x00;      /* Basic mode control register    */
inline constexpr uint32_t REG_BMSR = 0x01;      /* Basic mode status register     */
inline constexpr uint32_t REG_PHYSID1 = 0x02;   /* PHYS ID 1                      */
inline constexpr uint32_t REG_PHYSID2 = 0x03;   /* PHYS ID 2                      */
inline constexpr uint32_t REG_ADVERTISE = 0x04; /* Advertisement control register */
inline constexpr uint32_t REG_LPA = 0x05;       /* Link partner ability register  */

/* Basic mode control register. */
inline constexpr uint16_t BMCR_HALFDUPLEX_10M = 0x0000;          /* configure speed to 10 Mbit/s and the half-duplex mode */
inline constexpr uint16_t BMCR_FULLDUPLEX_10M = 0x0100;          /* configure speed to 10 Mbit/s and the full-duplex mode */
inline constexpr uint16_t BMCR_RESTART_AUTONEGOTIATION = 0x0200; /* restart auto-negotiation function */
inline constexpr uint16_t BMCR_ISOLATE = 0x0400;                 /* isolate PHY from MII */
inline constexpr uint16_t BMCR_POWERDOWN = 0x0800;               /* enable the power down mode */
inline constexpr uint16_t BMCR_AUTONEGOTIATION = 0x1000;         /* enable auto-negotiation function */
inline constexpr uint16_t BMCR_HALFDUPLEX_100M = 0x2000;         /* configure speed to 100 Mbit/s and the half-duplex mode */
inline constexpr uint16_t BMCR_FULLDUPLEX_100M = 0x2100;         /* configure speed to 100 Mbit/s and the full-duplex mode */
inline constexpr uint16_t BMCR_LOOPBACK = 0x4000;                /* enable phy loop-back mode */
inline constexpr uint16_t BMCR_RESET = 0x8000;                   /* PHY reset */

/* PHY basic mode status register */
inline constexpr uint16_t BMSR_JABBER_DETECTION = 0x0002;  /* jabber condition detected */
inline constexpr uint16_t BMSR_LINKED_STATUS = 0x0004;     /* valid link established */
inline constexpr uint16_t BMSR_AUTONEGO_COMPLETE = 0x0020; /* auto-negotioation process completed */

/* Advertisement control register. */
inline constexpr uint16_t ADVERTISE_SLCT = 0x001f;          /* Selector bits               */
inline constexpr uint16_t ADVERTISE_CSMA = 0x0001;          /* Only selector supported     */
inline constexpr uint16_t ADVERTISE_10HALF = 0x0020;        /* Try for 10mbps half-duplex  */
inline constexpr uint16_t ADVERTISE_1000XFULL = 0x0020;     /* Try for 1000BASE-X full-duplex */
inline constexpr uint16_t ADVERTISE_10FULL = 0x0040;        /* Try for 10mbps full-duplex  */
inline constexpr uint16_t ADVERTISE_1000XHALF = 0x0040;     /* Try for 1000BASE-X half-duplex */
inline constexpr uint16_t ADVERTISE_100HALF = 0x0080;       /* Try for 100mbps half-duplex */
inline constexpr uint16_t ADVERTISE_1000XPAUSE = 0x0080;    /* Try for 1000BASE-X pause    */
inline constexpr uint16_t ADVERTISE_100FULL = 0x0100;       /* Try for 100mbps full-duplex */
inline constexpr uint16_t ADVERTISE_1000XPSE_ASYM = 0x0100; /* Try for 1000BASE-X asym pause */
inline constexpr uint16_t ADVERTISE_100BASE4 = 0x0200;      /* Try for 100mbps 4k packets  */
inline constexpr uint16_t ADVERTISE_PAUSE_CAP = 0x0400;     /* Try for pause               */
inline constexpr uint16_t ADVERTISE_PAUSE_ASYM = 0x0800;    /* Try for asymetric pause     */
inline constexpr uint16_t ADVERTISE_RESV = 0x1000;          /* Unused...                   */
inline constexpr uint16_t ADVERTISE_RFAULT = 0x2000;        /* Say we can detect faults    */
inline constexpr uint16_t ADVERTISE_LPACK = 0x4000;         /* Ack link partners response  */
inline constexpr uint16_t ADVERTISE_NPAGE = 0x8000;         /* Next page bit               */

inline constexpr uint16_t ADVERTISE_FULL = (ADVERTISE_100FULL | ADVERTISE_10FULL | ADVERTISE_CSMA);
inline constexpr uint16_t ADVERTISE_ALL = (ADVERTISE_10HALF | ADVERTISE_10FULL | ADVERTISE_100HALF | ADVERTISE_100FULL);

/* Link partner ability register. */
inline constexpr uint16_t LPA_SLCT = 0x001f;            /* Same as advertise selector  */
inline constexpr uint16_t LPA_10HALF = 0x0020;          /* Can do 10mbps half-duplex   */
inline constexpr uint16_t LPA_1000XFULL = 0x0020;       /* Can do 1000BASE-X full-duplex */
inline constexpr uint16_t LPA_10FULL = 0x0040;          /* Can do 10mbps full-duplex   */
inline constexpr uint16_t LPA_1000XHALF = 0x0040;       /* Can do 1000BASE-X half-duplex */
inline constexpr uint16_t LPA_100HALF = 0x0080;         /* Can do 100mbps half-duplex  */
inline constexpr uint16_t LPA_1000XPAUSE = 0x0080;      /* Can do 1000BASE-X pause     */
inline constexpr uint16_t LPA_100FULL = 0x0100;         /* Can do 100mbps full-duplex  */
inline constexpr uint16_t LPA_1000XPAUSE_ASYM = 0x0100; /* Can do 1000BASE-X pause asym*/
inline constexpr uint16_t LPA_100BASE4 = 0x0200;        /* Can do 100mbps 4k packets   */
inline constexpr uint16_t LPA_PAUSE_CAP = 0x0400;       /* Can pause                   */
inline constexpr uint16_t LPA_PAUSE_ASYM = 0x0800;      /* Can pause asymetrically     */
inline constexpr uint16_t LPA_RFAULT = 0x2000;          /* Link partner faulted        */
inline constexpr uint16_t LPA_LPACK = 0x4000;           /* Link partner acked us       */
inline constexpr uint16_t LPA_NPAGE = 0x8000;           /* Next page bit               */

} // namespace net::mmi

#endif // EMAC_MMI_H_
