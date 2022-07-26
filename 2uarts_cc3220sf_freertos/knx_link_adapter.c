/*
 * knx_link_adapter.c
 */

#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "knx_link_internal.h"
#include "knx_link_adapter.h"

/* Driver configuration */
#include "ti_drivers_config.h"

/* Driver Header files */
#include <ti/drivers/UART.h>


knxLink_uart_t debugUart = NULL;
SemaphoreHandle_t  debugUartMutex;
int debugerror = 0;

knxLink_uart_t knxLinkAdapterOpen(int channel, int baudRate, int parityType) {
    UART_Params params;
    UART_Handle res;

    UART_Params_init(&params);

    params.readTimeout = UART_WAIT_FOREVER;
    params.writeTimeout = UART_WAIT_FOREVER;
    params.readEcho = UART_ECHO_OFF;
    params.readDataMode = UART_DATA_BINARY;
    params.writeDataMode = UART_DATA_BINARY;
    params.writeMode = UART_MODE_BLOCKING;
    params.readMode = UART_MODE_BLOCKING;
    params.baudRate = baudRate; // (baudRate == KNX_LINK_ADAPTER_BPS_9600? KNX_LINK_ADAPTER_BPS_9600 : KNX_LINK_ADAPTER_BPS_19200);
    params.parityType = (parityType == KNX_LINK_ADAPTER_PARITY_NONE? UART_PAR_NONE : (parityType == KNX_LINK_ADAPTER_PARITY_EVEN? UART_PAR_EVEN : UART_PAR_ODD));

    res = UART_open(channel, &params);
    return (knxLink_uart_t)res;
}

char knxLinkAdapterReadChar(knxLink_uart_t channel) {
    char rxBuffer;

    //debugPointer("adapterRead, num = %p\r\n", channel);
    if (channel == NULL) {
        //debug("adapterRead, num = NULL\r\n");
        return 0;
    }
    debug("adapterRead gonna read\r\n");
    if (UART_read(channel, &rxBuffer, 1) != 1) {
        debug("adapterRead, read failed\r\n");
        return 0;
    }
    debugInt("adapterRead, read ok (%02x)\r\n", rxBuffer);
    return rxBuffer;
}

void knxLinkAdapterWriteBuffer(knxLink_uart_t channel, uint8_t *txBuffer, int len) {

    if ((channel == NULL) || (txBuffer == NULL) || (len <= 0)) {
        return;
    }
    UART_write(channel, txBuffer, len);
}



void debugInit(knxLink_uart_t handle) {

    debugUartMutex = xSemaphoreCreateMutex();
    if(debugUartMutex == NULL) {
        while(1);
    }
    debugUart = handle;
}

void debug(char *msg) {
    if (debugUart != NULL) {
        if (xSemaphoreTake(debugUartMutex, portMAX_DELAY) == pdTRUE) {
            UART_write(debugUart, msg, strlen(msg));
            xSemaphoreGive(debugUartMutex);
        }
        else {
            debugerror++;
        }
    }
    else {
        debugerror++;
    }

}

void debugPointer(char *msg, void *p) {
    static char buffer[128];

    if (debugUart != NULL) {
        if (xSemaphoreTake(debugUartMutex, portMAX_DELAY) == pdTRUE) {
            snprintf(buffer, sizeof(buffer), msg, p);
            UART_write(debugUart, buffer, strlen(buffer));
            xSemaphoreGive(debugUartMutex);
        }
        else {
            debugerror++;
        }
    }
    else {
        debugerror++;
    }

}

void debugInt(char *msg, int c) {
    static char buffer[128];

    if (debugUart != NULL) {
        if (xSemaphoreTake(debugUartMutex, portMAX_DELAY) == pdTRUE) {
            snprintf(buffer, sizeof(buffer), msg, c);
            UART_write(debugUart, buffer, strlen(buffer));
            xSemaphoreGive(debugUartMutex);
        }
        else {
            debugerror++;
        }
    }
    else {
        debugerror++;
    }

}
