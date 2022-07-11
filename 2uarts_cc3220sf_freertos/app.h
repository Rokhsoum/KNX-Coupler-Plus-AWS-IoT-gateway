/*
 * app.h
 *
 *  Created on: 8 juil. 2022
 *      Author: Rokhaya Soumare
 */

#ifndef APP_H_
#define APP_H_

/**
 * Tipo estructurado con todos los parámetros del button
 */
typedef struct {
    uint8_t buttonID;               /**< Indica  el  botón */
    uint8_t buttonValue;            /**< El valor del botón (siempre igual a 1) */
} buttonMessageItem_t;



struct appParams_s * appInit(void);

void ButtonLeftCallback(Button_Handle buttonLeft, Button_EventMask buttonEvents);

void ButtonRightCallback(Button_Handle buttonRight, Button_EventMask buttonEvents);


#endif /* APP_H_ */
