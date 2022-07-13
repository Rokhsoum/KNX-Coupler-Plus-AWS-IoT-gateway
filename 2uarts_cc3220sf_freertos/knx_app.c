/*
 * knx_app.c
 */

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
#include "knx_link_adapter.h"
#include "knx_link_conf.h"

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
    knxLink_uart_t uarthandle;
    knxLinkHandle_t *uplink;                        /**< upLink */
    knxLinkHandle_t *downlink;                      /**< downLink */
    ga_set_type gas_boton_0, gas_boton_1;           /**< Dirección del grupo de butons 0 y 1 */
    ga_set_type gas_led_verde, gas_led_amarillo;    /**< Dirección del grupo de LEDs verde y amarillo */
    //QueueHandle_t buttonsKNXQueue;                  /**< Cola buttons (datos buttonMessageItem_t) */
    //QueueHandle_t ledGreenKNXQueue;                 /**< Cola ledVerde (uint8_t buttonValue) */
    //QueueHandle_t ledYellowKNXQueue;                /**< Cola ledAmarillo (uint8_t buttonValue) */
} knxAppParams_t;


typedef struct {
    knxLinkHandle_t *fromlink;
    knxLinkHandle_t *tolink;
} knxAppCouplerThreadArg_t;


/**
 * @brief   Manage button notifications for sending KNX telegrams
 */
static void _knxAppThread(void *arg0);

/**
 * @brief   Manage incoming KNX telegrams to change the status of LEDs
 */
static void _knxAppRecvThread(void *arg0);

/**
 * Variable privada con todos los parámetros del nivel de application
 */
static knxAppParams_t knxAppParams;

/**
 * Variable privada con todos los parámetros del boton
 */
//static buttonMessageItem_t buttonInf;


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

    for (i = 0; i < 3; i++) {
        do {
            knxLinkResetReq(uplink);
            knxLinkResetReq(downlink);
            i++;
        }
        while (knxLinkResetReq(uplink) == 1 && knxLinkResetReq(downlink) == 1 || i = 3);

        if (i == 3 && knxLinkResetReq(uplink) == 0 && knxLinkResetReq(downlink) == 0) {
            TPUART_CTRLFIELD_ACTIVATE_BUSMON;
        }
    }
#endif

    if (knxLinkResetReq(uplink) == 1 && knxLinkResetReq(downlink) == 1) {
        knxLinkSetAddressReq(uplink, knxAppParams.device_ia);
        knxLinkSetAddressReq(downlink, knxAppParams.device_ia);
    }
    else {
        break;
    }


    TaskHandle_t knxAppThreadHandle = NULL;
    xTaskCreate(_knxAppThread , "knxAppThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &knxAppThreadHandle);

    couplerargs.fromlink = uplink;
    couplerargs.tolink = downlink;
    TaskHandle_t knxAppRecvThreadHandle = NULL;
    xTaskCreate(_knxAppRecvThread, "knxAppRecvThread1", US_STACK_DEPTH, (void*) &couplerargs, tskIDLE_PRIORITY, &knxAppRecvThreadHandle);

    couplerargs.fromlink = downlink;
    couplerargs.tolink = uplink;
    xTaskCreate(_knxAppRecvThread, "knxAppRecvThread2", US_STACK_DEPTH, (void*) &couplerargs, tskIDLE_PRIORITY, &knxAppRecvThreadHandle);

    //knxAppParams.buttonsKNXQueue = xQueueCreate(KNX_APP_QUEUE_LENGTH, sizeof(buttonMessageItem_t));

    return &knxAppParams;
}


static void _knxAppThread(void *arg0) {
    /**
    uint8_t             slots[KNX_LINK_FRAME_POOL_SIZE];
    knxLinkFrame_t      *frames[KNX_LINK_FRAME_POOL_SIZE];
    int i;

    xQueueReceive(knxAppParams.buttonsKNXQueue, &buttonInf, portMAX_DELAY);

    if (buttonInf.buttonID == CONFIG_BUTTON_0) {
        for (i = 0; i < knxAppParams.gas_boton_0.used-1; i++) {
            slots[i] = knxLinkPoolAppLock();
        }

        for (i = 0; i < knxAppParams.gas_boton_0.used-1; i++) {
            frames[i] = knxLinkFramePoolAppGet(slots[i]);
            if (frames[i] != NULL) {
                frames[i]->sa = knxAppParams.device_ia;
                frames[i]->da = knxAppParams.gas_boton_0.ga_set[i];
                frames[i]->prio = 0x10;
                frames[i]->rep = 0;
                frames[i]->at = 1;
                frames[i]->hop_count = 7;
                frames[i]->ext_ff = 0;
                frames[i]->length = sizeof(frames[i]->lsdu);
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
                recvDataCon(knxAppParams.uplink);
                recvDataCon(knxAppParams.downlink);
            }
        }

        for (i = 0; i < knxAppParams.gas_boton_0.used-1; i++) {
            knxLinkPoolAppUnLock(slots[i]);
        }
    }

    else {
        for (i = 0; i < knxAppParams.gas_boton_1.used-1; i++) {
            slots[i] = knxLinkPoolAppLock();
        }

        for (i = 0; i < knxAppParams.gas_boton_1.used-1; i++) {
            frames[i] = knxLinkFramePoolAppGet(slots[i]);
            if (frames[i] != NULL) {
                frames[i]->sa = knxAppParams.device_ia;
                frames[i]->da = knxAppParams.gas_boton_1.ga_set[i];
                frames[i]->prio = 0x10;
                frames[i]->rep = 0;
                frames[i]->at = 1;
                frames[i]->hop_count = 7;
                frames[i]->ext_ff = 0;
                frames[i]->length = sizeof(frames[i]->lsdu);
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
                recvDataCon(knxAppParams.uplink);
                recvDataCon(knxAppParams.downlink);
            }
        }

        for (i = 0; i < knxAppParams.gas_boton_1.used-1; i++) {
            knxLinkPoolAppUnLock(slots[i]);
        }
    }
    */
    while (1) {
        vTaskDelay(1000);
    }
}


static void _knxAppRecvThread(void *arg0) {
    int i = 0 ;
    knxLinkFrame_t      *frames[KNX_LINK_FRAME_POOL_SIZE];
    uint8_t encoded_frame[1] = {0};
    char txBuffer[1] = {encoded_frame[1]};
    knxAppCouplerThreadArg_t *ActualArg = (knxAppCouplerThreadArg_t *)arg0;

    while(1) {
        recvDataInd(ActualArg->fromlink, &i);

        i = knxLinkPoolAppLock();
        frames[i] = knxLinkFramePoolAppGet(i);
        if (frames[i]->hop_count > 0) {
            knxLinkEncodeFrame(frames[i], encoded_frame, sizeof(encoded_frame));
            knxLinkAdapterWriteBuffer(knxAppParams.uarthandle, txBuffer, 1);
            frames[i]->hop_count--;
        }

        if (frames[i]->hop_count == 0) {
            while(1);
        }

        recvDataCon(ActualArg->tolink);
    }
}





