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
    uint16_t device_ia;                             /**< Dirección individual del dispositivo */
    ga_set_type gas_boton_0, gas_boton_1;           /**< Dirección del grupo de butons 0 y 1 */
    ga_set_type gas_led_verde, gas_led_amarillo;    /**< Dirección del grupo de LEDs verde y amarillo */
    QueueHandle_t buttonsKNXQueue;                  /**< Cola buttons (datos buttonMessageItem_t) */
    QueueHandle_t ledGreenKNXQueue;                 /**< Cola ledVerde (uint8_t buttonValue) */
    QueueHandle_t ledYellowKNXQueue;                /**< Cola ledAmarillo (uint8_t buttonValue) */
} knxAppParams_t;


/**
 * Tipo estructurado con todos los parámetros del button
 */
typedef struct {
    uint8_t buttonID;               /**< Indica  el  botón */
    uint8_t buttonValue;            /**< El valor del botón (siempre igual a 1) */
} buttonMessageItem_t;


/**
 * Variable privada con todos los parámetros del nivel de application
 */
static knxAppParams_t knxAppParams;

/**
 * @brief   Manage button notifications for sending KNX telegrams
 */
static void _knxAppThread(void *arg0);

/**
 * @brief   Manage incoming KNX telegrams to change the status of LEDs
 */
static void _knxAppRecvThread(void *arg0);

/**
 * @brief   Extracts a value from the ledGreenQUEUE queue and switches the output value of the green LED
 */
static void _ledGreenAppThread(void *arg0);

/**
 * @brief   Extracts a value from the ledYellowQUEUE queue and switches the output value of the yellow LED
 */
static void _ledYellowAppThread(void *arg0);

/**
 * Variable privada con todos los parámetros del boton
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

    knxAppParams.buttonsKNXQueue = xQueueCreate(KNX_APP_QUEUE_LENGTH, sizeof(buttonMessageItem_t));

    knxAppParams.ledGreenKNXQueue = xQueueCreate(KNX_APP_QUEUE_LENGTH, 1);

    knxAppParams.ledYellowKNXQueue = xQueueCreate(KNX_APP_QUEUE_LENGTH, 1);


    TaskHandle_t knxAppThreadHandle = NULL;
    xTaskCreate(_knxAppThread, "knxAppThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &knxAppThreadHandle);


    TaskHandle_t knxAppRecvThreadHandle = NULL;
    xTaskCreate(_knxAppRecvThread, "knxAppRecvThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &knxAppRecvThreadHandle);


    TaskHandle_t ledGreenAppThreadHandle = NULL;
    xTaskCreate(_ledGreenAppThread, "ledGreenAppThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &ledGreenAppThreadHandle);


    TaskHandle_t ledYellowAppThreadHandle = NULL;
    xTaskCreate(_ledYellowAppThread, "ledYellowAppThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &ledYellowAppThreadHandle);

    return UART_STATUS_SUCCESS;;
}


void ButtonLeftCallback(Button_Handle buttonLeft, Button_EventMask buttonEvents) {

    buttonInf.buttonID = CONFIG_BUTTON_0;
    buttonInf.buttonValue = 1;
    if (buttonEvents & Button_EV_CLICKED) {
    xQueueSend(knxAppParams.buttonsKNXQueue, &buttonInf, portMAX_DELAY);
    }
}

void ButtonRightCallback(Button_Handle buttonRight, Button_EventMask buttonEvents) {

    buttonInf.buttonID = CONFIG_BUTTON_1;
    buttonInf.buttonValue = 1;
    if (buttonEvents & Button_EV_CLICKED) {
    xQueueSend(knxAppParams.buttonsKNXQueue, &buttonInf, portMAX_DELAY);
    }
}


void _knxAppThread(void *arg0) {
    uint8_t             slots[KNX_LINK_FRAME_POOL_SIZE];
    knxLinkFrame_t      *frames[KNX_LINK_FRAME_POOL_SIZE];
    knxLinkFrame_t      *frame;
    knxLinkHandle_t     *link;
    int i;

    xQueueReceive(knxAppParams.buttonsKNXQueue, &buttonInf, portMAX_DELAY);

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
            frame->length = sizeof(frame->lsdu);
            //frame->lsdu[KNX_LINK_EXT_FRAME_LSDU_MAX] = ;
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
            frame->length = sizeof(frame->lsdu);
            //frame->lsdu[KNX_LINK_EXT_FRAME_LSDU_MAX] = ;
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


void _knxAppRecvThread(void *arg0) {
    int i =0 ;
    knxLinkFrame_t      *frames[KNX_LINK_FRAME_POOL_SIZE];
    knxLinkFrame_t      *frame;

    recvDataInd(i);

    for (i = 0; i < knxAppParams.gas_led_verde.used-1; i++) {
        if (frames[i] != NULL) {
            frame->da = knxAppParams.gas_led_verde.ga_set[i];

            if(ga_set_in(&knxAppParams.gas_led_verde, frame->da) == 1) {
                xQueueReceive(knxAppParams.ledGreenKNXQueue, &buttonInf.buttonValue, portMAX_DELAY);
            }
        }
    }

    for (i = 0; i < knxAppParams.gas_led_amarillo.used-1; i++) {
        if (frames[i] != NULL) {
            frame->da = knxAppParams.gas_led_amarillo.ga_set[i];

            if(ga_set_in(&knxAppParams.gas_led_amarillo, frame->da) == 1) {
                xQueueReceive(knxAppParams.ledYellowKNXQueue, &buttonInf.buttonValue, portMAX_DELAY);
            }
        }
    }
}


void _ledGreenAppThread(void *arg0) {
    xQueueReceive(knxAppParams.ledGreenKNXQueue, &buttonInf.buttonValue, portMAX_DELAY);

    LED_Params ledParams;
    LED_Handle ledGreen;

    LED_Params_init(&ledParams);

    ledGreen = LED_open(CONFIG_LED_1, &ledParams);

    LED_toggle(ledGreen);
}

void _ledYellowAppThread(void *arg0) {
    xQueueReceive(knxAppParams.ledGreenKNXQueue, &buttonInf.buttonValue, portMAX_DELAY);


    LED_Params ledParams;
    LED_Handle ledYellow;

    LED_Params_init(&ledParams);

    ledYellow = LED_open(CONFIG_LED_2, &ledParams);

    LED_toggle(ledYellow);

}



