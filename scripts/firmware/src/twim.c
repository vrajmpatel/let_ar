/**
 * @file twim.c
 * @brief nRF52840 TWIM (I2C Master) driver implementation
 * 
 * Low-level I2C master driver for the nRF52840 using the TWIM peripheral
 * with EasyDMA support.
 * 
 * Citations:
 * - nRF52840_PS_v1.11.pdf Section 6.31: TWIM peripheral documentation
 *   - TWIM0 Base: 0x40003000
 *   - TWIM1 Base: 0x40004000
 *   - Supported baud rates: 100, 250, 400 kbps
 *   - FREQUENCY values: K100=0x01980000, K250=0x04000000, K400=0x06400000
 * 
 * CRITICAL EasyDMA Requirements (nRF52840_PS_v1.11.pdf Section 4.6):
 *   "If TXD.PTR or RXD.PTR is not pointing to the Data RAM region, an EasyDMA 
 *    transfer may result in a HardFault and/or memory corruption."
 *   
 *   Data RAM region: 0x20000000 - 0x2003FFFF (256 KB)
 *   
 *   All buffers passed to TWIM must be in RAM, NOT Flash or stack variables
 *   that could potentially be optimized to Flash by the compiler.
 */

#include "twim.h"
#include "board.h"
#include <string.h>

/*******************************************************************************
 * Private Definitions
 ******************************************************************************/

/* Default timeout for I2C transactions (in simple loop iterations) */
#define TWIM_TIMEOUT_LOOPS      100000

/* Pin selection helper macro
 * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7.19/20:
 *   Bits[4:0] = PIN, Bits[5] = PORT, Bit[31] = CONNECT (0=connected)
 */
#define TWIM_PIN_SEL(port, pin) \
    (((pin) & 0x1F) | (((port) & 0x01) << 5) | TWIM_PSEL_CONNECT)

/* Memory barriers for Cortex-M4
 * Citation: ARM Cortex-M4 TRM - Required after peripheral configuration
 */
#define __DSB() __asm volatile ("dsb 0xF" ::: "memory")
#define __ISB() __asm volatile ("isb 0xF" ::: "memory")
#define __NOP() __asm volatile ("nop")

/* EasyDMA buffer for single-byte operations - MUST be in RAM, not stack
 * Citation: nRF52840_PS_v1.11.pdf Section 4.6 EasyDMA:
 *   "PTR register must point to a valid memory region"
 *   "If PTR is not pointing to Data RAM region, transfer may result in HardFault"
 */
static uint8_t s_easydma_buffer[4] __attribute__((aligned(4)));

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/**
 * @brief Wait for event with timeout
 * @param base TWIM base address
 * @param event_offset Event register offset
 * @return true if event occurred, false if timeout
 */
static bool twim_wait_event(uint32_t base, uint32_t event_offset)
{
    uint32_t timeout = TWIM_TIMEOUT_LOOPS;
    
    while (TWIM_REG_GET(base, event_offset) == 0) {
        if (--timeout == 0) {
            return false;
        }
    }
    
    /* Clear the event */
    TWIM_REG_SET(base, event_offset, 0);
    return true;
}

/**
 * @brief Check and handle TWIM errors
 * @param twim Pointer to TWIM handle
 * @return Error code or TWIM_OK
 */
static int twim_check_error(twim_t *twim)
{
    uint32_t errorsrc = TWIM_REG_GET(twim->base, TWIM_ERRORSRC);
    
    if (errorsrc == 0) {
        return TWIM_OK;
    }
    
    /* Clear error flags (write 1 to clear)
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7.17:
     *   "RW W1C" - Read-Write, Write-1-to-Clear
     */
    TWIM_REG_SET(twim->base, TWIM_ERRORSRC, errorsrc);
    
    if (errorsrc & TWIM_ERRORSRC_ANACK) {
        return TWIM_ERR_ANACK;
    }
    if (errorsrc & TWIM_ERRORSRC_DNACK) {
        return TWIM_ERR_DNACK;
    }
    if (errorsrc & TWIM_ERRORSRC_OVERRUN) {
        return TWIM_ERR_OVERRUN;
    }
    
    return TWIM_OK;
}

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

int twim_init(twim_t *twim, uint8_t instance, const twim_config_t *config)
{
    if (twim == NULL || config == NULL) {
        return TWIM_ERR_INVALID_PARAM;
    }
    
    if (instance > 1) {
        return TWIM_ERR_INVALID_PARAM;
    }
    
    /* Clear handle */
    memset(twim, 0, sizeof(twim_t));
    
    /* Set base address
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7:
     *   "TWIM0: 0x40003000, TWIM1: 0x40004000"
     */
    twim->base = (instance == 0) ? TWIM0_BASE : TWIM1_BASE;
    twim->instance = instance;
    twim->config = *config;
    
    /* Disable TWIM before configuration
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.6:
     *   "PSEL.SCL, PSEL.SDA must only be configured when the TWI master is disabled"
     */
    TWIM_REG_SET(twim->base, TWIM_ENABLE, TWIM_ENABLE_DISABLED);
    
    /* Configure pins
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7.19/20:
     *   PSEL.SCL and PSEL.SDA register format
     */
    TWIM_REG_SET(twim->base, TWIM_PSEL_SCL, 
                 TWIM_PIN_SEL(config->scl_port, config->scl_pin));
    TWIM_REG_SET(twim->base, TWIM_PSEL_SDA, 
                 TWIM_PIN_SEL(config->sda_port, config->sda_pin));
    
    /* Set frequency
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7.21:
     *   "K100: 0x01980000, K250: 0x04000000, K400: 0x06400000"
     */
    TWIM_REG_SET(twim->base, TWIM_FREQUENCY, config->frequency);
    
    /* Clear any pending events */
    TWIM_REG_SET(twim->base, TWIM_EVENTS_STOPPED, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_ERROR, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_TXSTARTED, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_RXSTARTED, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_LASTTX, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_LASTRX, 0);
    
    /* Clear error source */
    TWIM_REG_SET(twim->base, TWIM_ERRORSRC, 
                 TWIM_ERRORSRC_OVERRUN | TWIM_ERRORSRC_ANACK | TWIM_ERRORSRC_DNACK);
    
    /* Memory barrier before enabling
     * Citation: ARM Cortex-M4 TRM - Ensure all configuration writes complete
     */
    __DSB();
    __ISB();
    
    /* Enable TWIM
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7.18:
     *   "Enabled: 6 - Enable TWIM"
     */
    TWIM_REG_SET(twim->base, TWIM_ENABLE, TWIM_ENABLE_ENABLED);
    
    /* Memory barrier after enabling to ensure peripheral is ready
     * Citation: ARM Cortex-M4 TRM - Required for peripheral access ordering
     */
    __DSB();
    __ISB();
    
    /* Small delay to allow peripheral to stabilize
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.8.1:
     *   "tTWIM,START: Time from STARTRX/STARTTX task to transmission started = 1.5 µs"
     */
    for (volatile int i = 0; i < 100; i++) {
        __NOP();
    }
    
    twim->initialized = true;
    
    return TWIM_OK;
}

void twim_deinit(twim_t *twim)
{
    if (twim == NULL || !twim->initialized) {
        return;
    }
    
    /* Stop any ongoing transaction */
    TWIM_REG_SET(twim->base, TWIM_TASKS_STOP, 1);
    
    /* Wait for stop (with timeout) */
    twim_wait_event(twim->base, TWIM_EVENTS_STOPPED);
    
    /* Disable TWIM */
    TWIM_REG_SET(twim->base, TWIM_ENABLE, TWIM_ENABLE_DISABLED);
    
    /* Disconnect pins */
    TWIM_REG_SET(twim->base, TWIM_PSEL_SCL, TWIM_PSEL_DISCONNECT);
    TWIM_REG_SET(twim->base, TWIM_PSEL_SDA, TWIM_PSEL_DISCONNECT);
    
    twim->initialized = false;
}

int twim_set_frequency(twim_t *twim, twim_frequency_t frequency)
{
    if (twim == NULL || !twim->initialized) {
        return TWIM_ERR_INVALID_PARAM;
    }
    
    /* Must disable TWIM to change frequency */
    TWIM_REG_SET(twim->base, TWIM_ENABLE, TWIM_ENABLE_DISABLED);
    TWIM_REG_SET(twim->base, TWIM_FREQUENCY, frequency);
    TWIM_REG_SET(twim->base, TWIM_ENABLE, TWIM_ENABLE_ENABLED);
    
    twim->config.frequency = frequency;
    
    return TWIM_OK;
}

int twim_write(twim_t *twim, uint8_t addr, const uint8_t *data, 
               uint16_t len, bool stop)
{
    int result;
    
    if (twim == NULL || !twim->initialized) {
        return TWIM_ERR_INVALID_PARAM;
    }
    
    if (data == NULL || len == 0) {
        return TWIM_ERR_INVALID_PARAM;
    }
    
    /* Clear events */
    TWIM_REG_SET(twim->base, TWIM_EVENTS_STOPPED, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_ERROR, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_LASTTX, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_SUSPENDED, 0);
    
    /* Clear errors */
    TWIM_REG_SET(twim->base, TWIM_ERRORSRC, 
                 TWIM_ERRORSRC_OVERRUN | TWIM_ERRORSRC_ANACK | TWIM_ERRORSRC_DNACK);
    
    /* Set target address
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7.24:
     *   "Address used in the TWI transfer" (7-bit address)
     */
    TWIM_REG_SET(twim->base, TWIM_ADDRESS, addr);
    
    /* Configure TX buffer (EasyDMA)
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7.23:
     *   TXD.PTR and TXD.MAXCNT for EasyDMA
     */
    TWIM_REG_SET(twim->base, TWIM_TXD_PTR, (uint32_t)data);
    TWIM_REG_SET(twim->base, TWIM_TXD_MAXCNT, len);
    
    /* Configure shortcuts based on whether to generate stop */
    if (stop) {
        TWIM_REG_SET(twim->base, TWIM_SHORTS, TWIM_SHORTS_LASTTX_STOP);
    } else {
        TWIM_REG_SET(twim->base, TWIM_SHORTS, TWIM_SHORTS_LASTTX_SUSPEND);
    }
    
    /* Memory barrier before starting transaction
     * Citation: ARM Cortex-M4 TRM - Ensure buffer pointer is written before task
     */
    __DSB();
    
    /* Start TX transaction
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7:
     *   "TASKS_STARTTX: Start TWI transmit sequence"
     */
    TWIM_REG_SET(twim->base, TWIM_TASKS_STARTTX, 1);
    
    /* Wait for completion */
    if (stop) {
        if (!twim_wait_event(twim->base, TWIM_EVENTS_STOPPED)) {
            /* Timeout - force stop */
            TWIM_REG_SET(twim->base, TWIM_TASKS_STOP, 1);
            return TWIM_ERR_TIMEOUT;
        }
    } else {
        if (!twim_wait_event(twim->base, TWIM_EVENTS_SUSPENDED)) {
            TWIM_REG_SET(twim->base, TWIM_TASKS_STOP, 1);
            return TWIM_ERR_TIMEOUT;
        }
    }
    
    /* Check for errors */
    result = twim_check_error(twim);
    if (result != TWIM_OK) {
        return result;
    }
    
    /* Return number of bytes transferred */
    return (int)TWIM_REG_GET(twim->base, TWIM_TXD_AMOUNT);
}

int twim_read(twim_t *twim, uint8_t addr, uint8_t *data, uint16_t len)
{
    int result;
    
    if (twim == NULL || !twim->initialized) {
        return TWIM_ERR_INVALID_PARAM;
    }
    
    if (data == NULL || len == 0) {
        return TWIM_ERR_INVALID_PARAM;
    }
    
    /* Clear events */
    TWIM_REG_SET(twim->base, TWIM_EVENTS_STOPPED, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_ERROR, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_LASTRX, 0);
    
    /* Clear errors */
    TWIM_REG_SET(twim->base, TWIM_ERRORSRC, 
                 TWIM_ERRORSRC_OVERRUN | TWIM_ERRORSRC_ANACK | TWIM_ERRORSRC_DNACK);
    
    /* Set target address */
    TWIM_REG_SET(twim->base, TWIM_ADDRESS, addr);
    
    /* Configure RX buffer (EasyDMA)
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7.22:
     *   RXD.PTR and RXD.MAXCNT for EasyDMA
     */
    TWIM_REG_SET(twim->base, TWIM_RXD_PTR, (uint32_t)data);
    TWIM_REG_SET(twim->base, TWIM_RXD_MAXCNT, len);
    
    /* Configure shortcut: stop after last RX byte */
    TWIM_REG_SET(twim->base, TWIM_SHORTS, TWIM_SHORTS_LASTRX_STOP);
    
    /* Memory barrier before starting transaction
     * Citation: ARM Cortex-M4 TRM - Ensure buffer pointer is written before task
     */
    __DSB();
    
    /* Start RX transaction
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7:
     *   "TASKS_STARTRX: Start TWI receive sequence"
     */
    TWIM_REG_SET(twim->base, TWIM_TASKS_STARTRX, 1);
    
    /* Wait for completion */
    if (!twim_wait_event(twim->base, TWIM_EVENTS_STOPPED)) {
        TWIM_REG_SET(twim->base, TWIM_TASKS_STOP, 1);
        return TWIM_ERR_TIMEOUT;
    }
    
    /* Check for errors */
    result = twim_check_error(twim);
    if (result != TWIM_OK) {
        return result;
    }
    
    /* Return number of bytes received */
    return (int)TWIM_REG_GET(twim->base, TWIM_RXD_AMOUNT);
}

int twim_write_read(twim_t *twim, uint8_t addr, 
                    const uint8_t *tx_data, uint16_t tx_len,
                    uint8_t *rx_data, uint16_t rx_len)
{
    int result;
    
    if (twim == NULL || !twim->initialized) {
        return TWIM_ERR_INVALID_PARAM;
    }
    
    if (tx_data == NULL || tx_len == 0 || rx_data == NULL || rx_len == 0) {
        return TWIM_ERR_INVALID_PARAM;
    }
    
    /* Clear events */
    TWIM_REG_SET(twim->base, TWIM_EVENTS_STOPPED, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_ERROR, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_LASTTX, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_LASTRX, 0);
    
    /* Clear errors */
    TWIM_REG_SET(twim->base, TWIM_ERRORSRC, 
                 TWIM_ERRORSRC_OVERRUN | TWIM_ERRORSRC_ANACK | TWIM_ERRORSRC_DNACK);
    
    /* Set target address */
    TWIM_REG_SET(twim->base, TWIM_ADDRESS, addr);
    
    /* Configure TX buffer */
    TWIM_REG_SET(twim->base, TWIM_TXD_PTR, (uint32_t)tx_data);
    TWIM_REG_SET(twim->base, TWIM_TXD_MAXCNT, tx_len);
    
    /* Configure RX buffer */
    TWIM_REG_SET(twim->base, TWIM_RXD_PTR, (uint32_t)rx_data);
    TWIM_REG_SET(twim->base, TWIM_RXD_MAXCNT, rx_len);
    
    /* Configure shortcut for repeated start sequence:
     * LASTTX -> STARTRX, LASTRX -> STOP
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.3:
     *   "Repeated start" using LASTTX_STARTRX shortcut
     */
    TWIM_REG_SET(twim->base, TWIM_SHORTS, 
                 TWIM_SHORTS_LASTTX_STARTRX | TWIM_SHORTS_LASTRX_STOP);
    
    /* Memory barrier before starting transaction
     * Citation: ARM Cortex-M4 TRM - Ensure buffer pointers are written before task
     */
    __DSB();
    
    /* Start TX (will automatically continue to RX via shortcut) */
    TWIM_REG_SET(twim->base, TWIM_TASKS_STARTTX, 1);
    
    /* Wait for completion */
    if (!twim_wait_event(twim->base, TWIM_EVENTS_STOPPED)) {
        TWIM_REG_SET(twim->base, TWIM_TASKS_STOP, 1);
        return TWIM_ERR_TIMEOUT;
    }
    
    /* Check for errors */
    result = twim_check_error(twim);
    if (result != TWIM_OK) {
        return result;
    }
    
    return TWIM_OK;
}

int twim_write_reg(twim_t *twim, uint8_t addr, uint8_t reg, uint8_t value)
{
    uint8_t data[2] = {reg, value};
    int result = twim_write(twim, addr, data, 2, true);
    
    if (result < 0) {
        return result;
    }
    
    return (result == 2) ? TWIM_OK : TWIM_ERR_DNACK;
}

int twim_read_reg(twim_t *twim, uint8_t addr, uint8_t reg, uint8_t *value)
{
    return twim_write_read(twim, addr, &reg, 1, value, 1);
}

int twim_scan(twim_t *twim, uint8_t *found_addrs, uint8_t max_count)
{
    uint8_t count = 0;
    
    if (twim == NULL || !twim->initialized || found_addrs == NULL) {
        return TWIM_ERR_INVALID_PARAM;
    }
    
    /* Scan all valid 7-bit I2C addresses (0x08 to 0x77) */
    for (uint8_t addr = 0x08; addr <= 0x77 && count < max_count; addr++) {
        if (twim_device_present(twim, addr)) {
            found_addrs[count++] = addr;
        }
    }
    
    return count;
}

bool twim_device_present(twim_t *twim, uint8_t addr)
{
    if (twim == NULL || !twim->initialized) {
        return false;
    }
    
    /* Clear events and errors */
    TWIM_REG_SET(twim->base, TWIM_EVENTS_STOPPED, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_ERROR, 0);
    TWIM_REG_SET(twim->base, TWIM_ERRORSRC, 
                 TWIM_ERRORSRC_OVERRUN | TWIM_ERRORSRC_ANACK | TWIM_ERRORSRC_DNACK);
    
    /* Try to read one byte - device will ACK address if present
     * 
     * CRITICAL: Use static buffer for EasyDMA - NOT stack variable!
     * Citation: nRF52840_PS_v1.11.pdf Section 4.6 EasyDMA:
     *   "If RXD.PTR is not pointing to the Data RAM region when reception 
     *    is enabled, an EasyDMA transfer may result in a HardFault and/or 
     *    memory corruption."
     */
    TWIM_REG_SET(twim->base, TWIM_ADDRESS, addr);
    TWIM_REG_SET(twim->base, TWIM_RXD_PTR, (uint32_t)&s_easydma_buffer[0]);
    TWIM_REG_SET(twim->base, TWIM_RXD_MAXCNT, 1);
    TWIM_REG_SET(twim->base, TWIM_SHORTS, TWIM_SHORTS_LASTRX_STOP);
    
    /* Memory barrier before starting transaction */
    __DSB();
    
    TWIM_REG_SET(twim->base, TWIM_TASKS_STARTRX, 1);
    
    /* Wait for stop or error */
    uint32_t timeout = TWIM_TIMEOUT_LOOPS / 10;  /* Shorter timeout for scan */
    while (TWIM_REG_GET(twim->base, TWIM_EVENTS_STOPPED) == 0 &&
           TWIM_REG_GET(twim->base, TWIM_EVENTS_ERROR) == 0) {
        if (--timeout == 0) {
            TWIM_REG_SET(twim->base, TWIM_TASKS_STOP, 1);
            return false;
        }
    }
    
    /* Clear events */
    TWIM_REG_SET(twim->base, TWIM_EVENTS_STOPPED, 0);
    TWIM_REG_SET(twim->base, TWIM_EVENTS_ERROR, 0);
    
    /* Check if we got an ANACK (no device at this address) */
    uint32_t errorsrc = TWIM_REG_GET(twim->base, TWIM_ERRORSRC);
    TWIM_REG_SET(twim->base, TWIM_ERRORSRC, errorsrc);  /* Clear errors */
    
    return (errorsrc & TWIM_ERRORSRC_ANACK) == 0;
}

uint32_t twim_get_error_source(twim_t *twim)
{
    if (twim == NULL || !twim->initialized) {
        return 0;
    }
    
    return TWIM_REG_GET(twim->base, TWIM_ERRORSRC);
}

void twim_clear_errors(twim_t *twim)
{
    if (twim == NULL || !twim->initialized) {
        return;
    }
    
    TWIM_REG_SET(twim->base, TWIM_ERRORSRC, 
                 TWIM_ERRORSRC_OVERRUN | TWIM_ERRORSRC_ANACK | TWIM_ERRORSRC_DNACK);
}

const char* twim_error_string(int error)
{
    switch (error) {
        case TWIM_OK:               return "OK";
        case TWIM_ERR_ANACK:        return "Address NACK";
        case TWIM_ERR_DNACK:        return "Data NACK";
        case TWIM_ERR_OVERRUN:      return "RX buffer overrun";
        case TWIM_ERR_TIMEOUT:      return "Timeout";
        case TWIM_ERR_BUSY:         return "Bus busy";
        case TWIM_ERR_INVALID_PARAM: return "Invalid parameter";
        default:                    return "Unknown error";
    }
}
