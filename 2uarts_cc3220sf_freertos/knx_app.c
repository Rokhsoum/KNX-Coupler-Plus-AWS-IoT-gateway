/*
 * knx_app.c
 */
#include <knx_link_conf.h>
#include "FreeRTOS.h"
#include <task.h>
#include <stdint.h>
#include "knx_app.h"
#include "knx_link.h"
#include "knx_link_internal.h"
#include "knx_commissioning_data.h"
#include "knx_tpuart.h"
#include "knx_link_gadd_pool.h"
#include "knx_link_frame.h"
#include "knx_link_frame_pool.h"
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>

/* Driver configuration */
#include "ti_drivers_config.h"


/**
 * Tipo estructurado con todos los parámetros del nivel de application
 */
typedef struct knxAppParams_s {
    uint16_t device_ia;
    ga_set_type gas_boton_0, gas_boton_1;
    ga_set_type gas_led_verde, gas_led_amarillo;
    QueueHandle_t buttonsKNXQueue;
    QueueHandle_t ledVerdeKNXQueue;
    QueueHandle_t ledAmarilloKNXQueue;
} knxAppParams_t;


/**
 * Tipo estructurado con todos los parámetros del button
 */
typedef struct {
    uint8_t buttonID;
    uint8_t buttonValue;
} buttonMessageItem_t;


/**
 * Variable privada con todos los parámetros del nivel de application
 */
static knxAppParams_t knxAppParams;


/**
 * Variable privada con todos los parámetros del nivel de application
 */

static buttonMessageItem_t buttonInf;


// ............................................................................
// ============================================================================
//                      DEFINICIONES/IMPLEMENTACIÓN
// ============================================================================
// ............................................................................


struct knxAppParams_s * knxAppInit(void) {
    int i;
    knxAppParams.device_ia = 0;

    ga_set_init(&knxAppParams.gas_boton_0, ARRAY_SIZE(GA_SET_BOTON_0));
    for (i = 0; i < ARRAY_SIZE(GA_SET_BOTON_0)-1; i++) {
        ga_set_add(&knxAppParams.gas_boton_0, GA_SET_BOTON_0[i]);
    }

    ga_set_init(&knxAppParams.gas_boton_1, ARRAY_SIZE(GA_SET_BOTON_1));
    for (i = 0 ; i < ARRAY_SIZE(GA_SET_BOTON_0)-1; i++) {
        ga_set_add(&knxAppParams.gas_boton_1, GA_SET_BOTON_1[i]);
    }

    ga_set_init(&knxAppParams.gas_led_verde, ARRAY_SIZE(GA_SET_LED_0));
    for ( i = 0; i < ARRAY_SIZE(GA_SET_BOTON_0)-1; i++) {
        ga_set_add(&knxAppParams.gas_led_verde, GA_SET_LED_0[i]);
    }

    ga_set_init(&knxAppParams.gas_led_amarillo, ARRAY_SIZE(GA_SET_LED_1));
    for (i = 0; i < ARRAY_SIZE(GA_SET_BOTON_0)-1; i++) {
        ga_set_add(&knxAppParams.gas_led_amarillo, GA_SET_LED_1[i]);
    }

    knxLinkHandle_t *link = NULL;
    knxLinkResetReq(link);
    if (knxLinkResetReq(link) != 1) {
        knxLinkResetReq(link);
        knxLinkResetReq(link);
        knxLinkResetReq(link);
    }

    knxLinkSetAddressReq(link, knxAppParams.device_ia);

    knxAppParams.buttonsKNXQueue = xQueueCreate(KNX_APP_QUEUE_LENGTH, 1);

    TaskFunction_t knxAppThread = NULL;
    TaskHandle_t knxAppThreadHandle = NULL;
    xTaskCreate(knxAppThread, "knxAppThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &knxAppThreadHandle);

    TaskFunction_t knxAppRecvThread = NULL;
    TaskHandle_t knxAppRecvThreadHandle = NULL;
    xTaskCreate(knxAppRecvThread, "knxAppRecvThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &knxAppRecvThreadHandle);


    TaskFunction_t ledVerdeAppThread = NULL;
    TaskHandle_t ledVerdeAppThreadHandle = NULL;
    xTaskCreate(ledVerdeAppThread, "ledVerdeAppThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &ledVerdeAppThreadHandle);

    TaskFunction_t ledAmarilloAppThread = NULL;
    TaskHandle_t ledAmarilloAppThreadHandle = NULL;
    xTaskCreate(ledAmarilloAppThread, "ledAmarilloAppThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &ledAmarilloAppThreadHandle);

    return UART_STATUS_SUCCESS;;
}


void ButtonLeftCallback(Button_Handle buttonLeft, Button_EventMask buttonEvents) {

    buttonInf.buttonID = CONFIG_BUTTON_0;
    buttonInf.buttonValue = 1;
    if (buttonEvents & Button_EV_CLICKED) {
    xQueueSend(knxAppParams.buttonsKNXQueue, &buttonInf.buttonID, portMAX_DELAY);
    xQueueSend(knxAppParams.buttonsKNXQueue, &buttonInf.buttonValue, portMAX_DELAY);
    }
}

void ButtonRightCallback(Button_Handle buttonRight, Button_EventMask buttonEvents) {

    buttonInf.buttonID = CONFIG_BUTTON_1;
    buttonInf.buttonValue = 1;
    if (buttonEvents & Button_EV_CLICKED) {
    xQueueSend(knxAppParams.buttonsKNXQueue, &buttonInf.buttonID, portMAX_DELAY);
    xQueueSend(knxAppParams.buttonsKNXQueue, &buttonInf.buttonValue, portMAX_DELAY);
    }
}


void knxAppThread(void) {
    uint8_t             slots[KNX_LINK_FRAME_POOL_SIZE];
    knxLinkFrame_t      *frames[KNX_LINK_FRAME_POOL_SIZE];
    knxLinkFrame_t      *frame;
    knxLinkHandle_t     *link;
    int i;

    xQueueReceive(knxAppParams.buttonsKNXQueue, &buttonInf.buttonID, portMAX_DELAY);
    xQueueReceive(knxAppParams.buttonsKNXQueue, &buttonInf.buttonValue, portMAX_DELAY);

    for (i = 0; i < knxAppParams.gas_boton_0.used-1; i++) {
        slots[i] = knxLinkPoolAppLock();
    }

    for (i = 0; i < knxAppParams.gas_boton_0.used-1; i++) {
        frames[i] = knxLinkFramePoolAppGet(slots[i]);
        if (frames[i] != NULL) {
            frame->sa = knxAppParams.device_ia;
            frame->da = knxAppParams.gas_boton_0.ga_set[i];
            frame->prio = 0x10;
            frame->rep = 0;
            frame->at = 1;
            frame->hop_count = 7;
            frame->ext_ff = 0;
            //frame->length = knxAppParams.device_ia;
            //frame->lsdu = knxAppParams.device_ia;
        }
    }

    for (i = 0; i < knxAppParams.gas_boton_0.used-1; i++) {
        if (frames[i] != NULL) {
            knxLinkDataReq(link, slots[i]);
        }
    }

    for (i = 0; i < knxAppParams.gas_boton_0.used-1; i++) {
        if (frames[i] != NULL) {
            recvDataCon();
        }
    }

    for (i = 0; i < knxAppParams.gas_boton_0.used-1; i++) {
        knxLinkPoolAppUnLock(slots[i]);
    }


    for (i = 0; i < knxAppParams.gas_boton_1.used-1; i++) {
        slots[i] = knxLinkPoolAppLock();
    }


    for (i = 0; i < knxAppParams.gas_boton_1.used-1; i++) {
        frames[i] = knxLinkFramePoolAppGet(slots[i]);
        if (frames[i] != NULL) {
            frame->sa = knxAppParams.device_ia;
            frame->da = knxAppParams.gas_boton_1.ga_set[i];
            frame->prio = 0x10;
            frame->rep = 0;
            frame->at = 1;
            frame->hop_count = 7;
            frame->ext_ff = 0;
            //frames->length = knxAppParams.device_ia;
            //frames->lsdu = knxAppParams.device_ia;
        }
    }

    for (i = 0; i < knxAppParams.gas_boton_1.used-1; i++) {
        if (frames[i] != NULL) {
            knxLinkDataReq(link, slots[i]);
        }
    }

    for (i = 0; i < knxAppParams.gas_boton_1.used-1; i++) {
        if (frames[i] != NULL) {
            recvDataCon();
        }
    }

    for (i = 0; i < knxAppParams.gas_boton_1.used-1; i++) {
        knxLinkPoolAppUnLock(slots[i]);
    }
}


void knxAppRecvThread(void) {
    int i =0 ;
    knxLinkFrame_t      *frames[KNX_LINK_FRAME_POOL_SIZE];
    knxLinkFrame_t      *frame;

    recvDataInd(i);

    for (i = 0; i < knxAppParams.gas_led_verde.used-1; i++) {
        if (frames[i] != NULL) {
            frame->da = knxAppParams.gas_led_verde.ga_set[i];

            if(ga_set_in(&knxAppParams.gas_led_verde, frame->da) == 1) {
                xQueueReceive(knxAppParams.ledVerdeKNXQueue, &buttonInf.buttonValue, portMAX_DELAY);
            }
        }
    }

    for (i = 0; i < knxAppParams.gas_led_amarillo.used-1; i++) {
        if (frames[i] != NULL) {
            frame->da = knxAppParams.gas_led_amarillo.ga_set[i];

            if(ga_set_in(&knxAppParams.gas_led_amarillo, frame->da) == 1) {
                xQueueReceive(knxAppParams.ledAmarilloKNXQueue, &buttonInf.buttonValue, portMAX_DELAY);
            }
        }
    }
}


void ledVerdeAppThread(void) {
    xQueueReceive(knxAppParams.ledVerdeKNXQueue, &buttonInf.buttonValue, portMAX_DELAY);

    LED_Params ledParams;
    LED_Handle ledVerde;

    LED_Params_init(&ledParams);

    ledVerde = LED_open(CONFIG_LED_1, &ledParams);

    LED_toggle(ledVerde);
}

void ledAmarilloAppThread(void) {
    xQueueReceive(knxAppParams.ledVerdeKNXQueue, &buttonInf.buttonValue, portMAX_DELAY);


    LED_Params ledParams;
    LED_Handle ledAmarillo;

    LED_Params_init(&ledParams);

    ledAmarillo = LED_open(CONFIG_LED_2, &ledParams);

    LED_toggle(ledAmarillo);

}



