/*
 * app.h
 *
 *  Created on: 8 juil. 2022
 *      Author: Rokhaya Soumare
 */

#ifndef APP_H_
#define APP_H_


#include "knx_app.h"


struct appParams_s * appInit(void);

void ButtonLeftCallback(Button_Handle buttonLeft, Button_EventMask buttonEvents);

void ButtonRightCallback(Button_Handle buttonRight, Button_EventMask buttonEvents);


#endif /* APP_H_ */
