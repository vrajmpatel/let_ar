/**
 * @file twim.h
 * @brief nRF52840 TWIM (I2C Master) driver header
 * 
 * Low-level I2C master driver for the nRF52840 using the TWIM peripheral
 * with EasyDMA support.
 * 
 * Citations:
 * - nRF52840_PS_v1.11.pdf Section 6.31: TWIM peripheral documentation
 *   - TWIM0 Base: 0x40003000
 *   - TWIM1 Base: 0x40004000
 *   - Supported baud rates: 100, 250, 400 kbps
 */

#ifndef TWIM_H
#define TWIM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * TWIM Register Definitions
 * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7 "Registers"
 ******************************************************************************/

/* TWIM Instance Base Addresses
 * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7:
 *   "TWIM0: 0x40003000"
 *   "TWIM1: 0x40004000"
 */
#define TWIM0_BASE                  0x40003000UL
#define TWIM1_BASE                  0x40004000UL

/* Task Register Offsets
 * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7 Register overview
 */
#define TWIM_TASKS_STARTRX          0x000   /* Start TWI receive sequence */
#define TWIM_TASKS_STARTTX          0x008   /* Start TWI transmit sequence */
#define TWIM_TASKS_STOP             0x014   /* Stop TWI transaction */
#define TWIM_TASKS_SUSPEND          0x01C   /* Suspend TWI transaction */
#define TWIM_TASKS_RESUME           0x020   /* Resume TWI transaction */

/* Event Register Offsets */
#define TWIM_EVENTS_STOPPED         0x104   /* TWI stopped */
#define TWIM_EVENTS_ERROR           0x124   /* TWI error */
#define TWIM_EVENTS_SUSPENDED       0x148   /* SUSPEND task issued */
#define TWIM_EVENTS_RXSTARTED       0x14C   /* Receive sequence started */
#define TWIM_EVENTS_TXSTARTED       0x150   /* Transmit sequence started */
#define TWIM_EVENTS_LASTRX          0x15C   /* Starting to receive last byte */
#define TWIM_EVENTS_LASTTX          0x160   /* Starting to transmit last byte */

/* Register Offsets */
#define TWIM_SHORTS                 0x200   /* Shortcuts */
#define TWIM_INTEN                  0x300   /* Enable/disable interrupt */
#define TWIM_INTENSET               0x304   /* Enable interrupt */
#define TWIM_INTENCLR               0x308   /* Disable interrupt */
#define TWIM_ERRORSRC               0x4C4   /* Error source */
#define TWIM_ENABLE                 0x500   /* Enable TWIM */
#define TWIM_PSEL_SCL               0x508   /* Pin select for SCL */
#define TWIM_PSEL_SDA               0x50C   /* Pin select for SDA */
#define TWIM_FREQUENCY              0x524   /* TWI frequency */
#define TWIM_RXD_PTR                0x534   /* RXD EasyDMA pointer */
#define TWIM_RXD_MAXCNT             0x538   /* RXD maximum byte count */
#define TWIM_RXD_AMOUNT             0x53C   /* RXD bytes transferred */
#define TWIM_RXD_LIST               0x540   /* RXD EasyDMA list type */
#define TWIM_TXD_PTR                0x544   /* TXD EasyDMA pointer */
#define TWIM_TXD_MAXCNT             0x548   /* TXD maximum byte count */
#define TWIM_TXD_AMOUNT             0x54C   /* TXD bytes transferred */
#define TWIM_TXD_LIST               0x550   /* TXD EasyDMA list type */
#define TWIM_ADDRESS                0x588   /* Address used in TWI transfer */

/*******************************************************************************
 * TWIM Register Values
 * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7
 ******************************************************************************/

/* ENABLE register values
 * Citation: Section 6.31.7.18 - "Enabled: 6 - Enable TWIM"
 */
#define TWIM_ENABLE_DISABLED        0       /* Disable TWIM */
#define TWIM_ENABLE_ENABLED         6       /* Enable TWIM */

/* FREQUENCY register values
 * Citation: Section 6.31.7.21:
 *   "K100: 0x01980000 - 100 kbps"
 *   "K250: 0x04000000 - 250 kbps"
 *   "K400: 0x06400000 - 400 kbps"
 */
#define TWIM_FREQUENCY_K100         0x01980000UL    /* 100 kbps */
#define TWIM_FREQUENCY_K250         0x04000000UL    /* 250 kbps */
#define TWIM_FREQUENCY_K400         0x06400000UL    /* 400 kbps */

/* ERRORSRC register bits
 * Citation: Section 6.31.7.17
 */
#define TWIM_ERRORSRC_OVERRUN       (1UL << 0)  /* RX buffer overrun */
#define TWIM_ERRORSRC_ANACK         (1UL << 1)  /* NACK after address */
#define TWIM_ERRORSRC_DNACK         (1UL << 2)  /* NACK after data byte */

/* SHORTS register bits */
#define TWIM_SHORTS_LASTTX_STARTRX  (1UL << 7)  /* LASTTX -> STARTRX */
#define TWIM_SHORTS_LASTTX_SUSPEND  (1UL << 8)  /* LASTTX -> SUSPEND */
#define TWIM_SHORTS_LASTTX_STOP     (1UL << 9)  /* LASTTX -> STOP */
#define TWIM_SHORTS_LASTRX_STARTTX  (1UL << 10) /* LASTRX -> STARTTX */
#define TWIM_SHORTS_LASTRX_SUSPEND  (1UL << 11) /* LASTRX -> SUSPEND */
#define TWIM_SHORTS_LASTRX_STOP     (1UL << 12) /* LASTRX -> STOP */

/* PSEL register bit for connect/disconnect
 * Citation: Section 6.31.7.19/20 - "Disconnected: 1, Connected: 0" (bit 31)
 */
#define TWIM_PSEL_CONNECT           (0UL << 31) /* Pin connected */
#define TWIM_PSEL_DISCONNECT        (1UL << 31) /* Pin disconnected */
#define TWIM_PSEL_PORT_SHIFT        5           /* Port number bit position */

/*******************************************************************************
 * TWIM Error Codes
 ******************************************************************************/
#define TWIM_OK                     0           /* Success */
#define TWIM_ERR_ANACK              -1          /* Address NACK */
#define TWIM_ERR_DNACK              -2          /* Data NACK */
#define TWIM_ERR_OVERRUN            -3          /* RX buffer overrun */
#define TWIM_ERR_TIMEOUT            -4          /* Transaction timeout */
#define TWIM_ERR_BUSY               -5          /* Bus busy */
#define TWIM_ERR_INVALID_PARAM      -6          /* Invalid parameter */

/*******************************************************************************
 * TWIM Frequency Enumeration
 ******************************************************************************/
typedef enum {
    TWIM_FREQ_100K = TWIM_FREQUENCY_K100,
    TWIM_FREQ_250K = TWIM_FREQUENCY_K250,
    TWIM_FREQ_400K = TWIM_FREQUENCY_K400,
} twim_frequency_t;

/*******************************************************************************
 * TWIM Configuration Structure
 ******************************************************************************/
typedef struct {
    uint8_t          scl_pin;       /* SCL pin number (0-31) */
    uint8_t          scl_port;      /* SCL port (0 or 1) */
    uint8_t          sda_pin;       /* SDA pin number (0-31) */
    uint8_t          sda_port;      /* SDA port (0 or 1) */
    twim_frequency_t frequency;     /* I2C frequency */
} twim_config_t;

/*******************************************************************************
 * TWIM Handle Structure
 ******************************************************************************/
typedef struct {
    uint32_t         base;          /* TWIM peripheral base address */
    uint8_t          instance;      /* TWIM instance (0 or 1) */
    bool             initialized;   /* Initialization flag */
    twim_config_t    config;        /* Current configuration */
} twim_t;

/*******************************************************************************
 * Low-Level Register Access Macros
 ******************************************************************************/
#define TWIM_REG(base, offset)      (*(volatile uint32_t *)((base) + (offset)))
#define TWIM_REG_SET(base, offset, val)  (TWIM_REG(base, offset) = (val))
#define TWIM_REG_GET(base, offset)       (TWIM_REG(base, offset))

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief Initialize TWIM peripheral
 * @param twim Pointer to TWIM handle
 * @param instance TWIM instance (0 or 1)
 * @param config Pointer to configuration structure
 * @return TWIM_OK on success, error code on failure
 */
int twim_init(twim_t *twim, uint8_t instance, const twim_config_t *config);

/**
 * @brief Deinitialize TWIM peripheral
 * @param twim Pointer to TWIM handle
 */
void twim_deinit(twim_t *twim);

/**
 * @brief Set TWIM frequency
 * @param twim Pointer to TWIM handle
 * @param frequency New frequency setting
 * @return TWIM_OK on success, error code on failure
 */
int twim_set_frequency(twim_t *twim, twim_frequency_t frequency);

/**
 * @brief Write data to I2C device
 * @param twim Pointer to TWIM handle
 * @param addr 7-bit I2C device address
 * @param data Pointer to data buffer
 * @param len Number of bytes to write
 * @param stop Generate stop condition after transfer
 * @return Number of bytes written, or negative error code
 */
int twim_write(twim_t *twim, uint8_t addr, const uint8_t *data, 
               uint16_t len, bool stop);

/**
 * @brief Read data from I2C device
 * @param twim Pointer to TWIM handle
 * @param addr 7-bit I2C device address
 * @param data Pointer to receive buffer
 * @param len Number of bytes to read
 * @return Number of bytes read, or negative error code
 */
int twim_read(twim_t *twim, uint8_t addr, uint8_t *data, uint16_t len);

/**
 * @brief Write then read from I2C device (combined transaction)
 * @param twim Pointer to TWIM handle
 * @param addr 7-bit I2C device address
 * @param tx_data Pointer to transmit data
 * @param tx_len Number of bytes to write
 * @param rx_data Pointer to receive buffer
 * @param rx_len Number of bytes to read
 * @return TWIM_OK on success, error code on failure
 */
int twim_write_read(twim_t *twim, uint8_t addr, 
                    const uint8_t *tx_data, uint16_t tx_len,
                    uint8_t *rx_data, uint16_t rx_len);

/**
 * @brief Write single byte to register
 * @param twim Pointer to TWIM handle
 * @param addr 7-bit I2C device address
 * @param reg Register address
 * @param value Value to write
 * @return TWIM_OK on success, error code on failure
 */
int twim_write_reg(twim_t *twim, uint8_t addr, uint8_t reg, uint8_t value);

/**
 * @brief Read single byte from register
 * @param twim Pointer to TWIM handle
 * @param addr 7-bit I2C device address
 * @param reg Register address
 * @param value Pointer to store read value
 * @return TWIM_OK on success, error code on failure
 */
int twim_read_reg(twim_t *twim, uint8_t addr, uint8_t reg, uint8_t *value);

/**
 * @brief Scan I2C bus for devices
 * @param twim Pointer to TWIM handle
 * @param found_addrs Array to store found addresses
 * @param max_count Maximum number of addresses to store
 * @return Number of devices found
 */
int twim_scan(twim_t *twim, uint8_t *found_addrs, uint8_t max_count);

/**
 * @brief Check if device is present at address
 * @param twim Pointer to TWIM handle
 * @param addr 7-bit I2C device address
 * @return true if device responds, false otherwise
 */
bool twim_device_present(twim_t *twim, uint8_t addr);

/**
 * @brief Get error source from last failed transaction
 * @param twim Pointer to TWIM handle
 * @return Error source bits
 */
uint32_t twim_get_error_source(twim_t *twim);

/**
 * @brief Clear error flags
 * @param twim Pointer to TWIM handle
 */
void twim_clear_errors(twim_t *twim);

/**
 * @brief Get string description of error code
 * @param error Error code
 * @return String description
 */
const char* twim_error_string(int error);

#ifdef __cplusplus
}
#endif

#endif /* TWIM_H */
