/*
 * app.c
 *
 *  Created on: 8 juil. 2022
 *      Author: Rokhaya Soumare
 */

#include <knx_link_conf.h>
#include "FreeRTOS.h"
#include <stdint.h>
#include "knx_app.h"
#include "app.h"
#include "knx_link.h"
#include "knx_link_internal.h"

/* Driver configuration */
#include "ti_drivers_config.h"

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>


/**
 * Tipo estructurado con todos los parámetros del application
 */
typedef struct appParams_s {
    QueueHandle_t buttonsQueue;
    QueueHandle_t ledGreenQueue;
    QueueHandle_t ledYellowQueue;
} appParams_t;


/**
 * Tipo estructurado con todos los parámetros del button
 */
typedef struct {
    uint8_t buttonID;               /**< Indica del  botón */
    uint8_t buttonValue;            /**< El valor del botón (siempre igual a 1) */
} buttonMessageItem_t;


/**
 * @brief   Manage button notifications for sending KNX telegrams
 */
static void _appThread(void *arg0);

/**
 * @brief   Extracts a value from the ledGreenQUEUE queue and switches the output value of the green LED
 */
static void _ledGreenThread(void *arg0);

/**
 * @brief   Extracts a value from the ledYellowQUEUE queue and switches the output value of the yellow LED
 */
static void _ledYellowThread(void *arg0);

/**
 * Variable privada con todos los parámetros del boton
 */
static buttonMessageItem_t buttonInf;


/**
 * Variable privada con todos los parámetros del application
 */
static appParams_t appParams;




// ............................................................................
// ============================================================================
//                      DEFINICIONES/IMPLEMENTACIÓN
// ============================================================================
// ............................................................................

struct appParams_s * appInit(void) {

    appParams.buttonsQueue = xQueueCreate(KNX_APP_QUEUE_LENGTH, sizeof(buttonMessageItem_t));

    appParams.ledGreenQueue = xQueueCreate(KNX_APP_QUEUE_LENGTH, 1);

    appParams.ledYellowQueue = xQueueCreate(KNX_APP_QUEUE_LENGTH, 1);


    TaskHandle_t appThreadHandle = NULL;
    xTaskCreate(_appThread, "appThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &appThreadHandle);

    TaskHandle_t ledGreenThreadHandle = NULL;
    xTaskCreate(_ledGreenThread, "ledGreenAppThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &ledGreenThreadHandle);


    TaskHandle_t ledYellowThreadHandle = NULL;
    xTaskCreate(_ledYellowThread, "ledYellowAppThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &ledYellowThreadHandle);

    return &appParams;
}


void ButtonLeftCallback(Button_Handle buttonLeft, Button_EventMask buttonEvents) {

    buttonInf.buttonID = CONFIG_BUTTON_0;
    buttonInf.buttonValue = 1;

    if (buttonEvents & Button_EV_CLICKED) {
        xQueueSend(appParams.buttonsQueue, &buttonInf, portMAX_DELAY);
    }
}

void ButtonRightCallback(Button_Handle buttonRight, Button_EventMask buttonEvents) {

    buttonInf.buttonID = CONFIG_BUTTON_1;
    buttonInf.buttonValue = 1;

    if (buttonEvents & Button_EV_CLICKED) {
        xQueueSend(appParams.buttonsQueue, &buttonInf, portMAX_DELAY);
    }
}


static void _appThread(void *arg0) {

    xQueueReceive(appParams.buttonsQueue, &buttonInf.buttonValue , portMAX_DELAY);

    if (buttonInf.buttonID == CONFIG_BUTTON_0) {
        xQueueSend(appParams.ledGreenQueue, &buttonInf.buttonValue, portMAX_DELAY);
    }
    else {
        xQueueSend(appParams.ledYellowQueue, &buttonInf.buttonValue, portMAX_DELAY);
    }
}


static void _ledGreenThread(void *arg0) {
    xQueueReceive(appParams.ledGreenQueue, &buttonInf.buttonValue, portMAX_DELAY);

    LED_Params ledParams;
    LED_Handle ledGreen;

    LED_Params_init(&ledParams);

    ledGreen = LED_open(CONFIG_LED_1, &ledParams);

    LED_toggle(ledGreen);
}


static void _ledYellowThread(void *arg0) {
    xQueueReceive(appParams.ledYellowQueue, &buttonInf.buttonValue, portMAX_DELAY);

    LED_Params ledParams;
    LED_Handle ledYellow;

    LED_Params_init(&ledParams);

    ledYellow = LED_open(CONFIG_LED_2, &ledParams);

    LED_toggle(ledYellow);
}



