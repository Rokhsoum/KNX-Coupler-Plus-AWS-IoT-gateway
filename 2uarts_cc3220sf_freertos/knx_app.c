/*
 * knx_app.c
 */

#include "FreeRTOS.h"
#include <task.h>
#include <stdint.h>
#include "knx_app.h"
#include "app.h"
#include "knx_link.h"
#include "knx_commissioning_data.h"
#include "knx_link_adapter.h"




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
    knxLink_uart_t uartUplink;
    knxLink_uart_t uartDownlink;
    struct knxLinkHandle_s *uplink;                        /**< upLink */
    struct knxLinkHandle_s *downlink;                      /**< downLink */
    ga_set_type gas_boton_0, gas_boton_1;           /**< Dirección del grupo de butons 0 y 1 */
    ga_set_type gas_led_verde, gas_led_amarillo;    /**< Dirección del grupo de LEDs verde y amarillo */
    //QueueHandle_t buttonsKNXQueue;                  /**< Cola buttons (datos buttonMessageItem_t) */
    //QueueHandle_t ledGreenKNXQueue;                 /**< Cola ledVerde (uint8_t buttonValue) */
    //QueueHandle_t ledYellowKNXQueue;                /**< Cola ledAmarillo (uint8_t buttonValue) */
} knxAppParams_t;


typedef struct {
    struct knxLinkHandle_s *fromlink;
    struct knxLinkHandle_s *tolink;
    knxLink_uart_t uarthandle;
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


struct knxAppParams_s * knxAppInit(uint16_t ia, commissioning_data_t *comm_data, struct knxLinkHandle_s *uplink, struct knxLinkHandle_s *downlink) {
    int i;
    knxAppParams.device_ia = ia;
    knxAppCouplerThreadArg_t couplerargs;
    knxAppCouplerThreadArg_t couplerargs2;

    debugPointer("knxAppInit, link = %p\r\n", uplink);

    ga_set_init(&knxAppParams.gas_boton_0, comm_data->objects[0].gas_length);
    for (i = comm_data->objects[0].gas_start_index; i < (comm_data->objects[0].gas_length+comm_data->objects[0].gas_start_index); i++) {
        ga_set_add(&knxAppParams.gas_boton_0, comm_data->gas[i]);
    }

    ga_set_init(&knxAppParams.gas_boton_1, comm_data->objects[1].gas_length);
    for (i = comm_data->objects[1].gas_start_index ; i < (comm_data->objects[1].gas_length+comm_data->objects[1].gas_start_index); i++) {
        ga_set_add(&knxAppParams.gas_boton_1, comm_data->gas[i]);
    }

    ga_set_init(&knxAppParams.gas_led_verde, comm_data->objects[2].gas_length);
    for ( i = comm_data->objects[2].gas_start_index; i <(comm_data->objects[2].gas_length+comm_data->objects[2].gas_start_index); i++) {
        ga_set_add(&knxAppParams.gas_led_verde, comm_data->gas[i]);
    }

    ga_set_init(&knxAppParams.gas_led_amarillo, comm_data->objects[3].gas_length);
    for (i = comm_data->objects[3].gas_start_index; i < (comm_data->objects[3].gas_length+comm_data->objects[3].gas_start_index); i++) {
        ga_set_add(&knxAppParams.gas_led_amarillo, comm_data->gas[i]);
    }

    vTaskDelay(10);

    if (downlink != NULL) {
        for (i = 0; i < 3; i++) {
            knxLinkResetReq(downlink);
            if (knxLinkResetCon(downlink) == KNX_LINK_DATA_CON_POS) {
                break;
            }
        }

        if (i >= 3) {
            knxLinkSetState(downlink, KNX_LINK_STOP_STATE);
        }
        else {
            knxLinkSetAddressReq(downlink, knxAppParams.device_ia);
        }
    }


    if (uplink != NULL) {
        for (i = 0; i < 3; i++) {
            knxLinkResetReq(uplink);
            if (knxLinkResetCon(uplink) == KNX_LINK_DATA_CON_POS) {
                break;
            }
        }

        if (i >= 3) {
            knxLinkSetState(uplink, KNX_LINK_STOP_STATE);
        }
        else {
            knxLinkSetAddressReq(uplink, knxAppParams.device_ia);
        }
    }

#if 0
    TaskHandle_t knxAppRecvThreadHandle = NULL;
    TaskHandle_t knxAppThreadHandle = NULL;
    xTaskCreate(_knxAppThread , "knxAppThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &knxAppThreadHandle);

    if (uplink!= NULL) {
        couplerargs.fromlink = uplink;
        couplerargs.tolink = downlink;
        couplerargs.uarthandle = knxAppParams.uartDownlink;
        xTaskCreate(_knxAppRecvThread, "knxAppRecvThread1", US_STACK_DEPTH, (void*) &couplerargs, tskIDLE_PRIORITY, &knxAppRecvThreadHandle);
    }

    if (downlink!= NULL) {
        couplerargs2.fromlink = downlink;
        couplerargs2.tolink = uplink;
        couplerargs2.uarthandle = knxAppParams.uartUplink;
        xTaskCreate(_knxAppRecvThread, "knxAppRecvThread2", US_STACK_DEPTH, (void*) &couplerargs2, tskIDLE_PRIORITY, &knxAppRecvThreadHandle);
    }
#endif
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
    int frame_index;
    knxLinkFrame_t      *frame;
    knxLinkDataCon_t dataCon;
    knxAppCouplerThreadArg_t *ActualArg = (knxAppCouplerThreadArg_t *)arg0;

    while(1) {
        if (knxLinkDataInd(ActualArg->fromlink, &frame_index) == 1) {
            frame = knxLinkFramePoolAppGet(frame_index);
            if ((frame != NULL) && (ActualArg->tolink != NULL)) {
                frame->hop_count--;
                if (frame->hop_count > 0) {
                    for (i = 0; i < 3; i++) {
                        if(knxLinkDataReq(ActualArg->tolink, frame_index) == 1) {
                            knxLinkDataCon(ActualArg->tolink, &dataCon);
                            if (dataCon.confirmation == KNX_LINK_DATA_CON_POS) {
                                break;
                            }
                        }
                        else {
                            break;
                        }
                    }
                }
            }
            knxLinkPoolAppUnLock(frame_index);
        }
    }
}





