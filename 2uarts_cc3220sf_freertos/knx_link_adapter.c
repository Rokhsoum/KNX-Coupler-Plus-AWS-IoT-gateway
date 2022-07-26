/*
 * knx_link_adapter.c
 */

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "knx_link_internal.h"
#include "knx_link_adapter.h"

/* Driver configuration */
#include "ti_drivers_config.h"

/* Driver Header files */
#include <ti/drivers/UART.h>

knxLink_uart_t debugUart = NULL;
pthread_mutex_t debugUartMutex;
int debugerror = 0;

knxLink_uart_t knxLinkAdapterOpen(int channel, int baudRate, int parityType) {
    UART_Params         params;

    UART_Params_init(&params);

    params.readTimeout = UART_WAIT_FOREVER;
    params.writeTimeout = UART_WAIT_FOREVER;
    params.readEcho = UART_ECHO_OFF;
    params.readDataMode = UART_DATA_BINARY;
    params.writeDataMode = UART_DATA_BINARY;
    params.writeMode = UART_MODE_BLOCKING;
    params.readMode = UART_MODE_BLOCKING;
    //params.baudRate = (baudRate == KNX_LINK_ADAPTER_BPS_9600? 9600 : 19200);
    params.baudRate = (baudRate == KNX_LINK_ADAPTER_BPS_9600? KNX_LINK_ADAPTER_BPS_9600 : KNX_LINK_ADAPTER_BPS_19200);
    params.parityType = (parityType == KNX_LINK_ADAPTER_PARITY_NONE? UART_PAR_NONE : (parityType == KNX_LINK_ADAPTER_PARITY_EVEN? UART_PAR_EVEN : UART_PAR_ODD));

    knxLink_uart_t res = UART_open(channel, &params);

    if (res != NULL) {
        char rxBuffer[1] = {1};
        UART_write(res, rxBuffer, 1);
    }
    return res;
}


char knxLinkAdapterReadChar(knxLink_uart_t channel) {

    if (channel == NULL) {
        return 0;
    }

    char rxBuffer[1];
    if (UART_read(channel, rxBuffer, 1) != 1) {
        return 0;
    }

    return rxBuffer[0];
}


void knxLinkAdapterWriteBuffer(knxLink_uart_t channel, uint8_t *txBuffer, int len) {

    if ((channel == NULL) || (txBuffer == NULL) || (len <= 0)) {
        return;
    }

    UART_write(channel, txBuffer, len);
}

void debugInit(knxLink_uart_t handle) {

    if(pthread_mutex_init(&debugUartMutex, NULL)!= 0) {
        while(1);
    }
    debugUart = handle;
}

void debug(char *msg) {

    if (debugUart != NULL) {
        pthread_mutex_lock(&debugUartMutex);

        UART_write(debugUart, msg, strlen(msg));

        pthread_mutex_unlock(&debugUartMutex);
    }
    else {
        debugerror++;
    }

}

void debugPointer(char *msg, void *p) {
    static char buffer[128];

    if (debugUart != NULL) {
        if (pthread_mutex_lock(&debugUartMutex) == 0) {

            snprintf(buffer, sizeof(buffer), msg, p);
            UART_write(debugUart, buffer, strlen(buffer));
            pthread_mutex_unlock(&debugUartMutex);
        }
        else {
            debugerror++;
        }
    }
    else {
        debugerror++;
    }

}
