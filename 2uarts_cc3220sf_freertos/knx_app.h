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
 * @return NULL if error, pointer to allocated app handle otherwise
 */
struct knxAppParams_s * knxAppInit(void);


void ButtonLeftCallback(Button_Handle buttonHandle, Button_EventMask buttonEvents);

void ButtonRightCallback(Button_Handle buttonHandle, Button_EventMask buttonEvents);

/**
 * @brief   Function that manage button notifications for sending KNX telegrams
 */
void knxAppThread(void);


/**
 * @brief   Function that manage incoming KNX telegrams to change the status of LEDs
 */
void knxAppRecvThread(void);

//
void ledVerdeAppThread(void);

//
void ledAmarilloAppThread(void);


#endif /* KNX_APP_H_ */
