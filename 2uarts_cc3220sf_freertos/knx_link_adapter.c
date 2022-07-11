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
    params.writeMode = UART_MODE_BLOCKING;
    params.readMode = UART_MODE_BLOCKING;
    params.baudRate = (baudRate == KNX_LINK_ADAPTER_BPS_9600? 9600 : 19200);
    params.parityType = (parityType == KNX_LINK_ADAPTER_PARITY_NONE? UART_PAR_NONE : UART_PAR_EVEN);

    if (baudRate == KNX_LINK_ADAPTER_BPS_9600 || parityType == KNX_LINK_ADAPTER_PARITY_EVEN) {
        channel = KNX_LINK_ADAPTER_UPLINK;
        UART_open(channel, &params);
            if (UART_open(channel, &params) == NULL) {
                /* UART_open() failed */
                while (1);
            }
    }
    else if (baudRate == KNX_LINK_ADAPTER_BPS_19200 || parityType == KNX_LINK_ADAPTER_PARITY_ODD)
    {
        channel = KNX_LINK_ADAPTER_DOWNLINK;
        UART_open(channel, &params);
            if (UART_open(channel, &params) == NULL) {
                /* UART_open() failed */
                while (1);
            }
    }
    else {
        UART_open(channel, &params) == NULL;
    }

    return UART_STATUS_SUCCESS;
}


char knxLinkAdapterReadChar(knxLink_uart_t channel) {

    char rxBuffer[1];
    UART_read(channel, rxBuffer, 1);

    return rxBuffer[0];
}


void knxLinkAdapterWriteBuffer(knxLink_uart_t channel, char *txBuffer, int len) {

    UART_write(channel, &txBuffer, len);

}
