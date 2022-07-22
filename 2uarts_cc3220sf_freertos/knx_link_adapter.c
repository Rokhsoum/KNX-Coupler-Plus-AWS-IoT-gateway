/*
 * knx_link_adapter.c
 */

#include <pthread.h>
#include "FreeRTOS.h"
#include "knx_link_internal.h"
#include "knx_link_adapter.h"

/* Driver configuration */
#include "ti_drivers_config.h"

/* Driver Header files */
#include <ti/drivers/UART.h>


knxLink_uart_t knxLinkAdapterOpen(int channel, int baudRate, int parityType) {
    UART_Params         params;

    UART_init();

    UART_Params_init(&params);

    params.readTimeout = UART_WAIT_FOREVER;
    params.writeTimeout = UART_WAIT_FOREVER;
    params.readEcho = UART_ECHO_OFF;
    params.readDataMode = UART_DATA_BINARY;
    params.writeDataMode = UART_DATA_BINARY;

    params.writeMode = UART_MODE_BLOCKING;
    params.readMode = UART_MODE_BLOCKING;
    params.baudRate = (baudRate == KNX_LINK_ADAPTER_BPS_9600? 9600 : 19200);
    params.parityType = (parityType == KNX_LINK_ADAPTER_PARITY_NONE? UART_PAR_NONE : (parityType == KNX_LINK_ADAPTER_PARITY_EVEN? UART_PAR_EVEN : UART_PAR_ODD));

    UART_Handle res = UART_open(channel, &params);

    return res;
}


char knxLinkAdapterReadChar(knxLink_uart_t channel) {

    if (channel == NULL) {
        return 0;
    }

    char rxBuffer[1];
    UART_read(channel, rxBuffer, 1);

    return rxBuffer[0];
}


void knxLinkAdapterWriteBuffer(knxLink_uart_t channel, uint8_t *txBuffer, int len) {

    if ((channel == NULL) || (txBuffer == NULL) || (len <= 0)) {
        return;
    }

    UART_write(channel, txBuffer, len);
}
