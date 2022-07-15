/*
 * knx_app.h
 *
 *  Created on: 22 juin 2022
 *      Author: Rokhaya Soumare
 */

#ifndef KNX_APP_H_
#define KNX_APP_H_

#include "FreeRTOS.h"
#include <task.h>
#include <stdint.h>
#include "knx_link_gadd_pool.h"
#include "knx_commissioning_data.h"

#include <ti/drivers/apps/Button.h>


#define US_STACK_DEPTH              200         //Number of words to allocate for use as the task's stack.

#define KNX_APP_QUEUE_LENGTH        10          //Number of units that a queue can handle


struct knxAppParams_s;
struct knxLinkHandle_s;

/**
 * Initialize KNX app
 * @return NULL if error, pointer to allocated application handle otherwise
 */
struct knxAppParams_s * knxAppInit(uint16_t ia, commissioning_data_t *comm_data, struct knxLinkHandle_s *uplink, struct knxLinkHandle_s *downlink);

/**
 * @brief   Callback function of the button SW2
 * Al pulsar el botón SW2, inyecta datos en la cola buttonsQueue
 */
void ButtonLeftCallback(Button_Handle buttonLeft, Button_EventMask buttonEvents);


/**
 * @brief   Callback function of the button SW3
 * Al pulsar el botón SW3, inyecta datos en la cola buttonsQueue
 */
void ButtonRightCallback(Button_Handle buttonHandle, Button_EventMask buttonEvents);


#endif /* KNX_APP_H_ */
