/*
 * Copyright (C) 2018 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   drivers_cc110x
 * @{
 *
 * @file
 * @brief     Implementation of RIOT's netdev_driver API for the CC1100/CC1101
 *            transceiver
 *
 * @author    Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 * @}
 */

#include <errno.h>
#include <string.h>

#include "assert.h"
#include "iolist.h"
#include "luid.h"
#include "net/eui64.h"
#include "net/netdev.h"
#include "xtimer.h"

#include "cc110x.h"
#include "cc110x_internal.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

static int cc110x_init(netdev_t *netdev);
static int cc110x_recv(netdev_t *netdev, void *buf, size_t len, void *info);
static int cc110x_send(netdev_t *netdev, const iolist_t *iolist);
static int cc110x_get(netdev_t *netdev, netopt_t opt,
                      void *val, size_t max_len);
static int cc110x_set(netdev_t *netdev, netopt_t opt,
                      const void *val, size_t len);

const netdev_driver_t cc110x_driver = {
    .init = cc110x_init,
    .recv = cc110x_recv,
    .send = cc110x_send,
    .isr  = cc110x_isr,
    .get  = cc110x_get,
    .set  = cc110x_set,
};

void cc110x_on_gdo(void *_dev)
{
    cc110x_t *dev = _dev;

    dev->netdev.event_callback(&dev->netdev, NETDEV_EVENT_ISR);
}

static int check_device(cc110x_t *dev)
{
    uint8_t partnum, version, status;
    int is_ready;
    cc110x_state_t state;

    cc110x_read(dev, CC110X_REG_VERSION, &version);
    /* Retrieving the status is reliable for non-transient status */
    status = cc110x_read(dev, CC110X_REG_PARTNUM, &partnum);
    state = (status >> 4) & 0x5;
    /* Most significant bit should be zero, otherwise chip is not ready */
    is_ready = !(status & 0x80);

    DEBUG("[cc110x] PARTNUM = %i, VERSION = %i, STATUS = 0x%02x, READY = %i\n",
          (int)partnum, (int)version, (int)status, is_ready);

    if ((state != CC110X_STATE_IDLE) || (!is_ready)) {
        DEBUG("[cc110x] IC not ready or in invalid state\n");
        return -1;
    }

    /* Source: https://e2e.ti.com/support/wireless-connectivity/other-wireless/f/667/t/370643 */
    if (partnum != 0) {
        DEBUG("[cc110x] Device not a CC110x transceiver\n");
        return -1;
    }

    switch (version) {
        case 3:
            DEBUG("[cc110x] Detected CC1100 transceiver\n");
            /* RSSI offset is 78dBm @ 868MHz & 250kBaud.
             * Depends on the symbol rate and base band and ranges from
             * 74dBm to 79dBm.
             */
            dev->rssi_offset = 78;
            return 0;
        case 5:
            DEBUG("[cc110x] Detected CC1100E transceiver\n");
            /* RSSI offset is 79 dBm @ 250kbps & 250 kbps.
             * Depends on base band and symbol rate and ranges from
             * 75dBm to 79dBm
             */
            dev->rssi_offset = 79;
            return 0;
        case 4:
        /* falls through */
        case 14:
            /* RSSI offset for the CCC1101 is independent of symbol rate and
             * base 74 dBm
             */
            dev->rssi_offset = 74;
            DEBUG("[cc110x] Detected CC1101 transceiver\n");
            return 0;
        default:
            DEBUG("[cc110x] Device not a CC110x transceiver\n");
            return -1;
    }
}

static int check_config(cc110x_t *dev)
{
    char buf[CC110X_CONF_SIZE];

    /* Verify content of main config registers */
    cc110x_burst_read(dev, CC110X_CONF_START, buf, sizeof(buf));
    if (memcmp(buf, cc110x_conf, sizeof(buf))) {
        DEBUG("[cc110x] ERROR: Verification of main config registers failed\n");
        return -1;
    }

    /* Verify content of "magic number" config registers */
    cc110x_burst_read(dev, CC110X_REG_TEST2, buf,
                      sizeof(cc110x_magic_registers));
    if (memcmp(buf, cc110x_magic_registers, sizeof(cc110x_magic_registers))) {
        DEBUG("[cc110x] ERROR: Verification of \"magic\" registers failed\n");
        return -1;
    }

    /* Verify content of PA_TABLE */
    cc110x_burst_read(dev, CC110X_MULTIREG_PATABLE, buf, CC110X_PATABLE_LEN);
    if (memcmp(buf, dev->params.patable->data, CC110X_PATABLE_LEN)) {
        DEBUG("[cc110x] ERROR: Verification of PA_TABLE failed\n");
        return -1;
    }

    DEBUG("[cc110x] Content of configuration registers verified\n");
    return 0;
}

static int cc110x_init(netdev_t *netdev)
{
    cc110x_t *dev = (cc110x_t *)netdev;

    /* Make sure the crystal is stable and the chip ready */
    if (cc110x_power_on(dev)) {
        DEBUG("[cc110x] netdev_driver_t::init(): Failed to pull CS pin low\n");
        return -EIO;
    }

    if (cc110x_acquire(dev) != SPI_OK) {
        DEBUG("[cc110x] netdev_driver_t::init(): Failed to setup/acquire SPI "
              "interface\n");
        return -EIO;
    }

    /* Performing a reset of the transceiver to get it in a known state */
    cc110x_cmd(dev, CC110X_STROBE_RESET);
    cc110x_release(dev);

    /* Again, make sure the crystal is stable and the chip ready */
    if (cc110x_power_on(dev)) {
        DEBUG("[cc110x] netdev_driver_t::init(): Failed to pull CS pin low "
              "after reset\n");
        return -EIO;
    }

    if (cc110x_acquire(dev) != SPI_OK) {
        DEBUG("[cc110x] netdev_driver_t::init(): Failed to setup/acquire SPI "
              "interface after reset\n");
        return -EIO;
    }

    if (check_device(dev)) {
        DEBUG("[cc110x] netdev_driver_t::init(): Device check failed\n");
        cc110x_release(dev);
        return -ENOTSUP;
    }

    /* Upload the main configuration */
    cc110x_burst_write(dev, CC110X_CONF_START, cc110x_conf, CC110X_CONF_SIZE);

    /* Upload the poorly documented magic numbers obtained via SmartRF Studio */
    cc110x_burst_write(dev, CC110X_REG_TEST2, cc110x_magic_registers,
                       sizeof(cc110x_magic_registers));

    /* Setup the selected PA_TABLE */
    cc110x_burst_write(dev, CC110X_MULTIREG_PATABLE,
                       dev->params.patable->data, CC110X_PATABLE_LEN);

    /* Verify main config, magic numbers and PA_TABLE correctly uploaded */
    if (check_config(dev)) {
        return -EIO;
    }

    /* Setup the layer 2 address, but do not accept 0x00 (used for broadcast) */
    dev->addr = dev->params.l2addr;
    while (dev->addr == 0x00) {
        luid_get(&dev->addr, 1);
    }
    cc110x_write(dev, CC110X_REG_ADDR, dev->addr);

    /* Setup interrupt on GDO0  */
    if (gpio_init_int(dev->params.gdo0, GPIO_IN, GPIO_BOTH,
                      cc110x_on_gdo, dev)) {
        cc110x_release(dev);
        DEBUG("[cc110x] netdev_driver_t::init(): Failed to setup interrupt on "
              "GDO0 pin\n");
        return -EIO;
    }

    /* Setup interrupt on GDO2 */
    if (gpio_init_int(dev->params.gdo2, GPIO_IN, GPIO_BOTH,
                      cc110x_on_gdo, dev)) {
        gpio_irq_disable(dev->params.gdo0);
        cc110x_release(dev);
        DEBUG("[cc110x] netdev_driver_t::init(): Failed to setup interrupt on "
              "GDO2 pin\n");
        return -EIO;
    }

    /* Update the state of the driver/transceiver */
    dev->state = CC110X_STATE_IDLE;
    cc110x_release(dev);


    int retval; /*< Store return value to be able to pass through error code */
    /* Apply configuration (if non-NULL) and channel map, which also calls
     * cc110x_full_calibration
     */
    retval = cc110x_apply_config(dev, dev->params.config, dev->params.channels);
    if (retval) {
        gpio_irq_disable(dev->params.gdo0);
        gpio_irq_disable(dev->params.gdo2);
        DEBUG("[cc110x] netdev_driver_t::init(): cc110x_apply_config() "
              "failed\n");
        /* Pass through received error code  */
        return retval;
    }
    else {
        DEBUG("[cc110x] netdev_driver_t::init(): Success\n");
    }

    return 0;
}

static int cc110x_recv(netdev_t *netdev, void *buf, size_t len, void *info)
{
    cc110x_t *dev = (cc110x_t *)netdev;

    /* Call to cc110x_rx() will clear dev->buf.len, so back up it first */
    int size = dev->buf.len;

    if (cc110x_acquire(dev) != SPI_OK) {
        DEBUG("[cc110x] netdev_driver_t::recv(): cc110x_acquire() "
              "failed\n");
        return -EIO;
    }

    if (!buf) {
        /* Get the size of the frame; if len > 0 then also drop the frame */
        if (len > 0) {
            /* Drop frame requested */
            cc110x_rx(dev);
        }
        cc110x_release(dev);
        return size;
    }

    if (len < (size_t)size) {
        cc110x_release(dev);
        return -ENOBUFS;
    }

    memcpy(buf, dev->buf.data, (size_t)size);

    /* Copy RX info (if requested) on last frame before entering RX again */
    if (info != NULL) {
        *((cc110x_rx_info_t *)info) = dev->rx_info;
    }

    cc110x_rx(dev);
    cc110x_release(dev);
    return size;
}

static int cc110x_send(netdev_t *netdev, const iolist_t *iolist)
{
    cc110x_t *dev = (cc110x_t *)netdev;

    /* assert that cc110x_send was called with valid parameters */
    assert(netdev && iolist && (iolist->iol_len == CC110X_HEADER_SIZE));

    if (cc110x_acquire(dev) != SPI_OK) {
        DEBUG("[cc110x] netdev_driver_t::send(): cc110x_acquire() failed\n");
        return -1;
    }

    switch (dev->state) {
        case CC110X_STATE_FSTXON:
        /* falls through */
        case CC110X_STATE_RX:
            break;
        case CC110X_STATE_RECEIVING:
            /* While receiving medium is obviously busy */
            dev->netdev.event_callback(&dev->netdev,
                                       NETDEV_EVENT_TX_MEDIUM_BUSY);
        /* falls through */
        default:
            DEBUG("[cc110x] netdev_driver_t::send(): Driver state %i prevents "
                  "sending\n", (int)dev->state);
            cc110x_release(dev);
            return -1;
    }

    /* Copy data to send into frame buffer */
    size_t size = CC110X_HEADER_SIZE;
    memcpy(dev->buf.data, iolist->iol_base, CC110X_HEADER_SIZE);

    for (const iolist_t *iol = iolist->iol_next; iol; iol = iol->iol_next) {
        if (size + iol->iol_len > CC110X_MAX_FRAME_SIZE) {
            DEBUG("[cc110x] netdev_driver_t::send(): Frame size exceeds "
                  "maximum supported size\n");
            cc110x_release(dev);
            return -1;
        }
        memcpy(dev->buf.data + size, iol->iol_base, iol->iol_len);
        size += iol->iol_len;
    }

    dev->buf.len = (uint8_t)size;

    /* Disable IRQs, as GDO configuration will be changed now */
    gpio_irq_disable(dev->params.gdo0);
    gpio_irq_disable(dev->params.gdo2);

    /* Fill the TX FIFO: First write the length, then the frame */
    dev->buf.pos = (size > CC110X_FIFO_SIZE - 1) ? CC110X_FIFO_SIZE - 1 : size;
    /* cc110x_framebuf_t has the same memory layout as the device expects */
    cc110x_burst_write(dev, CC110X_MULTIREG_FIFO,
                       &dev->buf, dev->buf.pos + 1);

    /* Configure GDO2 and update state */
    if (dev->buf.pos > dev->buf.len) {
        /* We need to keep feeding TX FIFO */
        cc110x_write(dev, CC110X_REG_IOCFG2, CC110X_GDO_ON_TX_DATA);
        dev->state = CC110X_STATE_TX;
    }
    else {
        /* All data in TX FIFO, just waiting for transceiver to finish */
        cc110x_write(dev, CC110X_REG_IOCFG2, CC110X_GDO_CONSTANT_LOW);
        dev->state = CC110X_STATE_TX_COMPLETING;
    }

    /* Finally: Go to TX */
    cc110x_cmd(dev, CC110X_STROBE_TX);

    /* Restore IRQs */
    gpio_irq_enable(dev->params.gdo0);
    gpio_irq_enable(dev->params.gdo2);

    cc110x_release(dev);
    return 0;
}

/**
 * @brief Generate an IPv6 interface identifier for a CC110X transceiver
 *
 * @param dev   Transceiver to create the IPv6 interface identifier (IID)
 * @param iid   Store the generated IID here
 *
 * @return      Returns the size of @ref eui64_t to confirm with the API
 *              in @ref netdev_driver_t::get
 */
static int cc110x_get_iid(cc110x_t *dev, eui64_t *iid)
{
    static const eui64_t empty_iid = {
        .uint8 = { 0x00, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x00 }
    };

    *iid = empty_iid;
    iid->uint8[7] = dev->addr;
    return sizeof(eui64_t);
}

static int cc110x_get(netdev_t *netdev, netopt_t opt,
                      void *val, size_t max_len)
{
    cc110x_t *dev = (cc110x_t *)netdev;

    switch (opt) {
        case NETOPT_DEVICE_TYPE:
            assert(max_len == sizeof(uint16_t));
            *((uint16_t *)val) = NETDEV_TYPE_CC110X;
            return sizeof(uint16_t);
        case NETOPT_PROTO:
            assert(max_len == sizeof(gnrc_nettype_t));
            *((gnrc_nettype_t *)val) = CC110X_DEFAULT_PROTOCOL;
            return sizeof(gnrc_nettype_t);
        case NETOPT_MAX_PACKET_SIZE:
            assert(max_len == sizeof(uint16_t));
            *((uint16_t *)val) = CC110X_MAX_FRAME_SIZE;
            return sizeof(uint16_t);
        case NETOPT_ADDR_LEN:
        /* falls through */
        case NETOPT_SRC_LEN:
            assert(max_len == sizeof(uint16_t));
            *((uint16_t *)val) = CC110X_ADDR_SIZE;
            return sizeof(uint16_t);
        case NETOPT_ADDRESS:
            assert(max_len >= CC110X_ADDR_SIZE);
            *((uint8_t *)val) = dev->addr;
            return CC110X_ADDR_SIZE;
        case NETOPT_IPV6_IID:
            if (max_len < sizeof(eui64_t)) {
                return -EOVERFLOW;
            }
            return cc110x_get_iid(dev, val);
        case NETOPT_CHANNEL:
            assert(max_len == sizeof(uint16_t));
            *((uint16_t *)val) = dev->channel;
            return sizeof(uint16_t);
        default:
            return -ENOTSUP;
    }
}

/**
 * @brief Set the given address as the device's layer 2 address
 *
 * @param dev    Device descripter of the transceiver
 * @param addr   Address to set
 */
static int cc110x_set_addr(cc110x_t *dev, uint8_t addr)
{
    if (cc110x_acquire(dev) != SPI_OK) {
        return -EIO;
    }

    dev->addr = addr;
    cc110x_write(dev, CC110X_REG_ADDR, addr);
    cc110x_release(dev);
    return 1;
}

static int cc110x_set(netdev_t *netdev, netopt_t opt,
                      const void *val, size_t len)
{
    (void)len;
    cc110x_t *dev = (cc110x_t *)netdev;

    switch (opt) {
        case NETOPT_ADDRESS:
            assert(len == CC110X_ADDR_SIZE);
            return cc110x_set_addr(dev, *((uint8_t *)val));
        case NETOPT_CHANNEL:
        {
            assert(len == sizeof(uint16_t));
            int retval;
            uint16_t channel = *((uint16_t *)val);
            if (channel >= CC110X_MAX_CHANNELS) {
                return -EINVAL;
            }
            if ((retval = cc110x_hop(dev, (uint8_t)channel))) {
                return retval;
            }
        }
            return sizeof(uint16_t);
        default:
            return -ENOTSUP;
    }
}
