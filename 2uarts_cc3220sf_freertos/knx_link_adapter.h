/*
 * knx_link_adapter.h
 *
 *  Created on: 9 juin 2022
 *      Author: Rokhaya Soumare
 */

#ifndef KNX_LINK_ADAPTER_H_
#define KNX_LINK_ADAPTER_H_

//#include "FreeRTOS.h"
//#include <semphr.h>
//#include <pthread.h>
#include <stdint.h>
#include <ti/drivers/UART.h>


/**
 *  Macro to define link uart channel
 */
#define KNX_LINK_ADAPTER_UPLINK            0
#define KNX_LINK_ADAPTER_DOWNLINK          1

/**
 *  Macro to define the baudRate
 */
#define KNX_LINK_ADAPTER_BPS_9600          0
#define KNX_LINK_ADAPTER_BPS_19200         1

/**
 *  Macro to define the parity
 */
#define KNX_LINK_ADAPTER_PARITY_NONE        0
#define KNX_LINK_ADAPTER_PARITY_ODD         1
#define KNX_LINK_ADAPTER_PARITY_EVEN        2


/**
 *  @brief      A handle that is returned from a knxLinkAdapterOpen() call.
 */
typedef UART_Handle knxLink_uart_t;


/**
 *  @brief  Function to open a given UART peripheral
 *
 *  @param  link          Link type
 *
 *  @param  bps           Baud rate for UART
 *
 *  @param  parity        Parity bit type for UART
 *
 *  @return A UART_Handle upon success. NULL if an error occurs, or if the
 *          indexed UART peripheral is already opened.
 */
knxLink_uart_t knxLinkAdapterOpen(int link, int bps, int parity);


/**
 *  @brief  Function that reads data from a UART with blocking mode.
 *
 *  @param  channel     The pointer to knxLink_uart_t.
 *
 *  @return Returns the byte read from the UART,
 *          UART_STATUS_ERROR on an error.
 */
char knxLinkAdapterReadChar(knxLink_uart_t channel);

/**
 *  @brief  Function that writes data to a UART in blocking mode.
 *
 *  @param  channel     The pointer to knxLink_uart_t
 *
 *  @param  txBuffer    A read-only pointer to buffer containing data to
 *                      be written to the UART
 *
 *  @param  len        The number of bytes in the buffer that should be written
 *                     to the UART
 */
void knxLinkAdapterWriteBuffer(knxLink_uart_t channel, uint8_t *txBuffer, int len);



#endif /* KNX_LINK_ADAPTER_H_ */
