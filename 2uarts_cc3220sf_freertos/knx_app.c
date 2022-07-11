/*
 * knx_app.c
 */
#include <knx_link_conf.h>
#include "FreeRTOS.h"
#include <task.h>
#include <stdint.h>
#include "knx_app.h"
#include "app.h"
#include "knx_link.h"
#include "knx_link_internal.h"
#include "knx_commissioning_data.h"
#include "knx_tpuart.h"
#include "knx_link_gadd_pool.h"
#include "knx_link_frame.h"
#include "knx_link_frame_pool.h"

/* Driver configuration */
#include "ti_drivers_config.h"

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>


/**
 * Tipo estructurado con todos los parámetros del nivel de application
 */
typedef struct knxAppParams_s {
    uint16_t device_ia;                             /**< Dirección individual del dispositivo */
    knxLinkHandle_t *uplink;                        /**< upLink */
    knxLinkHandle_t *downlink;                      /**< downLink */
    ga_set_type gas_boton_0, gas_boton_1;           /**< Dirección del grupo de butons 0 y 1 */
    ga_set_type gas_led_verde, gas_led_amarillo;    /**< Dirección del grupo de LEDs verde y amarillo */
    QueueHandle_t buttonsKNXQueue;                  /**< Cola buttons (datos buttonMessageItem_t) */
    QueueHandle_t ledGreenKNXQueue;                 /**< Cola ledVerde (uint8_t buttonValue) */
    QueueHandle_t ledYellowKNXQueue;                /**< Cola ledAmarillo (uint8_t buttonValue) */
} knxAppParams_t;


typedef struct {
    knxLinkHandle_t *fromlink;
    knxLinkHandle_t *tolink;
} knxAppCouplerThreadArg_t;


/**
 * Variable privada con todos los parámetros del nivel de application
 */
static knxAppParams_t knxAppParams;


/**
 * @brief   Manage incoming KNX telegrams to change the status of LEDs
 */
static void _knxAppRecvThread(void *arg0);

/**
 * Variable privada con todos los parámetros del boton
 */
static buttonMessageItem_t buttonInf;


// ............................................................................
// ============================================================================
//                      DEFINICIONES/IMPLEMENTACIÓN
// ============================================================================
// ............................................................................


struct knxAppParams_s * knxAppInit(uint16_t ia, knxLinkHandle_t *uplink, knxLinkHandle_t *downlink) {
    int i;
    knxAppParams.device_ia = ia;
    knxAppCouplerThreadArg_t couplerargs;

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

#if 0
    knxLinkResetReq(link);
    if (knxLinkResetReq(link) != 1) {
        knxLinkResetReq(link);
        knxLinkResetReq(link);
        knxLinkResetReq(link);
    }


    for (i = 0; i < 3; i++) {

    }
#endif

    knxLinkSetAddressReq(link, knxAppParams.device_ia);

    couplerargs.fromlink = uplink;
    couplerargs.tolink = downlink;
    TaskHandle_t knxAppRecvThreadHandle = NULL;
    xTaskCreate(_knxAppRecvThread, "knxAppRecvThread1", US_STACK_DEPTH, (void*) &couplerargs, tskIDLE_PRIORITY, &knxAppRecvThreadHandle);

    couplerargs.fromlink = downlink;
    couplerargs.tolink = uplink;
    xTaskCreate(_knxAppRecvThread, "knxAppRecvThread2", US_STACK_DEPTH, (void*) &couplerargs, tskIDLE_PRIORITY, &knxAppRecvThreadHandle);

    knxAppParams.buttonsKNXQueue = xQueueCreate(KNX_APP_QUEUE_LENGTH, sizeof(buttonMessageItem_t));

    return &knxAppParams;
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
            knxLinkDataReq(knxAppParams.uplink, slots[i]);
            knxLinkDataReq(knxAppParams.downlink, slots[i]);
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
            knxLinkDataReq(knxAppParams.uplink, slots[i]);
            knxLinkDataReq(knxAppParams.downlink, slots[i]);
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
    int i = 0 ;
    knxLinkFrame_t      *frames[KNX_LINK_FRAME_POOL_SIZE];
    knxLinkFrame_t      *frame;
    knxAppCouplerThreadArg_t *ActuAlArg = (knxAppCouplerThreadArg_t *)arg0;

    recvDataInd(ActuAlArg->fromlink, &i);
    // Forward message to tolink
    //wait for confirmation

    /**
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
    }*/
}





