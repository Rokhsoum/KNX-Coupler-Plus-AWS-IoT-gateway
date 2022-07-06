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

#include <ti/drivers/apps/Button.h>


#define US_STACK_DEPTH              200         //Number of words to allocate for use as the task's stack.

#define KNX_APP_QUEUE_LENGTH        10          //Number of units that a queue can handle


struct knxAppParams_s;


/**
 * Initialize KNX app
 * @return NULL if error, pointer to allocated application handle otherwise
 */
struct knxAppParams_s * knxAppInit(void);

/**
 * @brief   Callback function of the button SW2
 */
void ButtonLeftCallback(Button_Handle buttonLeft, Button_EventMask buttonEvents);


/**
 * @brief   Callback function of the button SW3
 */
void ButtonRightCallback(Button_Handle buttonHandle, Button_EventMask buttonEvents);


#endif /* KNX_APP_H_ */
