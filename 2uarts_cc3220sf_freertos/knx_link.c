/*
 * knx_link.c
 */
#include <string.h>
#include "FreeRTOS.h"
#include <task.h>
#include <semphr.h>
#include <pthread.h>
#include "knx_link_internal.h"
#include "knx_link_adapter.h"
#include "knx_link.h"
#include "knx_link_frame.h"
#include "knx_link_frame_pool.h"
#include "knx_link_gadd_pool.h"
#include "knx_link_state.h"
#include "knx_tpuart.h"
#include "knx_link_encoding.h"
//#include "knx_commissioning_data.h"

/* Driver configuration */
//#include "ti_drivers_config.h"

/* Driver Header files */
#include <ti/drivers/UART.h>


/* Stack size in bytes */
#define KNX_LINK_THREADSTACKSIZE   1024

/**
 * @brief Posibles valores del estado del máquina de estados en KNX_LINK_NORMAL_STATE
 */
#define E_CTRL              1
#define E_CTRLE             2
#define E_SA_H              3
#define E_SA_L              4
#define E_DA_H              5
#define E_DA_L              6
#define E_LG                7
#define E_AT_LSDU_LG        8
#define E_LSDU              9
#define E_CHK               10
#define E_DATA_CONF         11


/**
 * Variable privada con todos los parámetros del nivel de enlace
 */
static int knxLink_handleused = 0;
static knxLinkHandle_t knxLinkParams[2];

/**
 * @brief Reception task, manage incoming data from TP-UART 
 */
static void _knxLinkRecvThread(void *arg0);

/**
 * @brief Transmission task, manage requests from upper level
 */
static void _knxLinkTxThread(void *arg0);

/**
 * @brief Data request task, (do requests from upper level:changer)
 */
static void _knxLinkDataReqThread(void *arg0);


// ............................................................................
// ============================================================================
//                      DEFINICIONES/IMPLEMENTACIÓN
// ============================================================================
// ............................................................................


struct knxLinkHandle_s * knxLinkInit(uint16_t ia, knxLink_uart_t uartlink) {

    if (knxLink_handleused >= 2 ) {
        return NULL;
    }

    if (uartlink == NULL) {
        return NULL;
    }

    knxLinkParams[knxLink_handleused].uartKNX = uartlink;
    /**
     * @TODO
     * Inicializar mutex uartKNXMutex.
     * Si error, while(1);
     */

    int res;
    res = pthread_mutex_init(&knxLinkParams[knxLink_handleused].uartKNXMutex, NULL);

    if (res != NULL) {
           while(1);
    }


    /**
     * @TODO
     * Crear cola knxLinkResetCon
     * Crear semáforo knxLinkResetSem
     * Si error, while(1);
     */

    knxLinkParams[knxLink_handleused].knxLinkResetCon = xQueueCreate(KNX_LINK_QUEUE_LENGTH, 1);

    knxLinkParams[knxLink_handleused].knxLinkResetSem = xSemaphoreCreateBinary();
    if (knxLinkParams[knxLink_handleused].knxLinkResetSem == NULL) {
        while(1);
    }
    xSemaphoreGive(knxLinkParams[knxLink_handleused].knxLinkResetSem);

    /**
     * @TODO
     * Crear colas knxLinkDataReq, knxLinkDataCon, knxLinkDataInd
     * Crear semáforo knxLinkDataSem
     * Si error, while(1);
     */

    knxLinkParams[knxLink_handleused].knxLinkDataReq = xQueueCreate(KNX_LINK_QUEUE_LENGTH, sizeof(int));

    knxLinkParams[knxLink_handleused].knxLinkDataCon = xQueueCreate(KNX_LINK_QUEUE_LENGTH, sizeof(knxLinkDataCon_t));

    knxLinkParams[knxLink_handleused].knxLinkDataInd = xQueueCreate(KNX_LINK_QUEUE_LENGTH, sizeof(int));


    //create a thread to reform the dataReq queue and perfom all he tasks to send the message
    /**
     * @TODO
     * Crear tarea knxLinkRecvThread, knxLinkDataReqThread, knxLinkTxThreadHandle
     * Si error, while(1);
     */

       TaskHandle_t knxLinkRecvThreadHandle = NULL;
       BaseType_t ret;
       ret = xTaskCreate(_knxLinkRecvThread, "knxLinkRecvThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &knxLinkRecvThreadHandle);

       if ( ret != pdPASS ) {
           while(1);
       }

       TaskHandle_t knxLinkDataReqThreadHandle = NULL;
       BaseType_t ret1;
       ret1 = xTaskCreate(_knxLinkDataReqThread, "knxLinkDataReqThreadHandle", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &knxLinkDataReqThreadHandle);

       if (ret1 != pdPASS ) {
           while(1);
       }


       TaskHandle_t knxLinkTxThreadHandle = NULL;
       BaseType_t ret2;
       ret2 = xTaskCreate(_knxLinkTxThread, "knxLinkTxThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &knxLinkTxThreadHandle);

       if (ret2 != pdPASS ) {
           while(1);
       }

    /**
     * @TODO
     * Inicializar parámetros nivel de enlace, pool, etc.
     */

       knxLinkParams[knxLink_handleused].state = KNX_LINK_INIT_STATE;
       knxLinkParams[knxLink_handleused].stateR = E_CTRL;
       knxLinkParams[knxLink_handleused].ia = ia;
       //knxLinkParams[knxLink_handleused].da = KNX_DA_ADDRESS;
       //knxLinkParams[knxLink_handleused].ga = KNX_GA_ADDRESS;

       knxLinkFramePoolInit();


       return &knxLinkParams[knxLink_handleused++];
}



// ___---=== Definiciones relacionadas con el servicio de set-address ===---___

int knxLinkSetAddressReq(knxLinkHandle_t *link, uint16_t ia) {
	// Variables locales que nos harán falta:
    char buf[3] = {TPUART_CTRLFIELD_SET_ADDRESS, 0, 0};

    /**
     * @TODO
     * Implementar primitiva request del servicio set-address
     * Hacerlo sólo si el estado del nivel de enlace es KNX_LINK_INIT_STATE
     */
    if (knxLinkGetState(link) == KNX_LINK_INIT_STATE) {

        pthread_mutex_lock(&knxLinkParams[knxLink_handleused].uartKNXMutex);

        buf[0] = (uint8_t)((ia & 0xFF00) >> 8);
        buf[1] = (uint8_t)(ia & 0x00FF);
        knxLinkAdapterWriteBuffer(knxLinkParams[knxLink_handleused].uartKNX, buf, 3);

        pthread_mutex_unlock(&knxLinkParams[knxLink_handleused].uartKNXMutex);
    }
    else {
        return 0;
    }

    return 1;
}



// ___---=== Definiciones relacionadas con el servicio de reset ===---___

int knxLinkResetReq(knxLinkHandle_t *link) {
	// Variables locales que nos harán falta:
    char buf[1] = {TPUART_CTRLFIELD_RESET_REQUEST};


    /**
     * @TODO
     * Implementar primitiva request del servicio reset
     * Hacerlo sólo si el estado del nivel de enlace es KNX_LINK_INIT_STATE
     */
    if (link->knxLinkResetSem == NULL && knxLinkGetState(link) != KNX_LINK_INIT_STATE) {
        return 0;
    }

    if (link->knxLinkResetSem != NULL) {
        if( xSemaphoreTake(link->knxLinkResetSem, portMAX_DELAY ) == pdTRUE ) {

            if (knxLinkGetState(link) == KNX_LINK_INIT_STATE) {

                pthread_mutex_lock(&link->uartKNXMutex);

                knxLinkAdapterWriteBuffer(link->uartKNX, buf, 1);

                pthread_mutex_unlock(&link->uartKNXMutex);

                //xSemaphoreGive(knxLinkParams[knxLink_handleused].knxLinkResetSem); on le give aprés dans RecvThread
            }
        }
        else {
            return 0;
        }
    }
    return 1;
}


uint8_t knxLinkResetCon(knxLinkHandle_t *link) {

    uint8_t con = 0;
    xQueueSend(link->knxLinkResetCon, &con, portMAX_DELAY );

    return con;
}

// ___---=== Definiciones relacionadas con el servicio de datos ===---___

static void _knxLinkDataReqThread(void *arg0) {
    knxLinkHandle_t *link = (knxLinkHandle_t *)arg0;
    // Variables locales que nos harán falta:
    int k = 0, frame_index = 0;
    int len = KNX_LINK_STD_FRAME_MAX;
    uint8_t encoded_frame[len];
    char Bufftemporal[2];

    /**
     * @TODO
     * Implementar primitiva request del servicio datos
     * Hacerlo sólo si el estado del nivel de enlace es KNX_LINK_NORMAL_STATE
     * Obtener puntero a la trama (frame) a partir del índice dentro del pool (frame_index),
     * verificando antes que frame_index corresponde a un hueco del pool
     * reservado al alto nivel
     */

    while(1) {

        xQueueReceive(link->knxLinkDataReq, &frame_index, portMAX_DELAY);

        if (knxLinkGetState(link) == KNX_LINK_NORMAL_STATE && link->ia == 0) {
            while(1);
        }

        if (knxLinkGetState(link) != KNX_LINK_NORMAL_STATE) {

            if (link->ia != 0) {

                knxLinkPoolAppYieldLock(frame_index);

                knxLinkFrame_t frame[frame_index];
                knxLinkEncodeFrame(frame, encoded_frame, sizeof(encoded_frame)); //juste frame si y erreur

                pthread_mutex_lock(&link->uartKNXMutex);

                for (k=0; k < len-1; k++) {
                    Bufftemporal[0] = TPUART_CTRLFIELD_DATA_START || TPUART_CTRLFIELD_DATA_CONT(frame_index) || TPUART_CTRLFIELD_DATA_END(len);
                    Bufftemporal[1] = encoded_frame[k];
                    knxLinkAdapterWriteBuffer(link->uartKNX, Bufftemporal, 2);
                }

                pthread_mutex_unlock(&link->uartKNXMutex);
            }
        }
    }
}


knxLinkDataCon_t knxLinkDataCon(knxLinkHandle_t *link) {

    knxLinkDataCon_t dataCon;
    xQueueReceive(link->knxLinkDataCon, &dataCon, portMAX_DELAY);

    return dataCon;
}


int knxLinkDataInd(knxLinkHandle_t *link) {

    int frame_index = 0;
    xQueueReceive(link->knxLinkDataInd, &frame_index, portMAX_DELAY);

    return frame_index;
}


static void _knxLinkRecvThread(void *arg0) {
	knxLinkHandle_t *link = (knxLinkHandle_t *)arg0;
    uint8_t data;
    uint8_t resetCon;
    uint8_t length = 0;
    knxLinkFrame_t *frame;
    uint8_t FT;
    uint8_t AT;
    uint8_t hopCount;
    uint8_t extFF;
    uint8_t saH;
    uint8_t saL;
    uint8_t daH;
    uint8_t daL;
    uint8_t LG;
    uint8_t buffer[KNX_LINK_EXT_FRAME_MAX];
    int buf_index = 0;
    uint16_t sa, da = 0;
    int L_Data_Confirm = 0;
    knxLinkDataCon_t con;
    int frame_index = knxLinkPoolLinkLock();

    while(1) {
        data = knxLinkAdapterReadChar(link->uartKNX);
        switch(link->state) {
        case KNX_LINK_INIT_STATE:
            /**
             * @TODO
             * Enviar KNX_LINK_RESET_CON_POS a cola knxLinkResetCon si dato == TPUART_RESPONSE_RESET_CONFIRMATION,
             * enviar KNX_LINK_RESET_CON_NEG a cola knxLinkResetCon en otro caso
             */

            if (data == TPUART_RESPONSE_RESET_CONFIRMATION) {
                resetCon = KNX_LINK_RESET_CON_POS;
            }
            else {
                resetCon = KNX_LINK_RESET_CON_NEG;
            }
            xQueueSend(link->knxLinkResetCon, &resetCon, portMAX_DELAY);
            break;

        case KNX_LINK_NORMAL_STATE:
            /**
             * @TODO
             * Máquina de estados con almacenamiento de la trama entrante
             */

            switch(link->stateR) {
            case E_CTRL:
                //from data, extract FT; data is the whole CTRL byte
                FT = knxLinkDecodeCtrlFt(data);
                if (FT == 0 ) {
                    link->stateR = E_SA_H;
                }
                else {
                    link->stateR = E_CTRLE;
                }
                buffer[0] = data;
                buf_index = 1;
                break;

            case E_CTRLE:
                AT = knxLinkDecodeCtrleAt(data);
                hopCount = knxLinkDecodeCtrleHopCount(data);
                extFF = knxLinkDecodeCtrleExtFF(data);
                link->stateR = E_SA_H;
                buffer[buf_index++] = data;
                break;

            case E_SA_H:
                saH = data;
                link->stateR = E_SA_L;
                buffer[buf_index++] = data;
                break;

            case E_SA_L:
                saL = data;
                sa = knxLinkEncodeAddress(saH, saL);
                link->stateR = E_DA_H;
                buffer[buf_index++] = data;
                break;

            case E_DA_H:
                daH = data;
                link->stateR = E_DA_L;
                buffer[buf_index++] = data;
                break;

            case E_DA_L:
                daL = data;
                da = knxLinkEncodeAddress(daH, daL);
                if (FT == 0) {
                    link->stateR = E_AT_LSDU_LG;
                }
                else {
                    link->stateR = E_LG;
                }
                buffer[buf_index++] = data;
                break;

            case E_AT_LSDU_LG:
                AT = knxLinkDecodeAtLsduLgAt(data);
                hopCount = knxLinkDecodeAtLsduLgHopCount(data);
                LG = knxLinkDecodeAtLsduLgLg(data);
                length = (LG + 1);
                link->stateR = E_LSDU;
                buffer[buf_index++] = data;
                break;

            case E_LG:
                LG = data;
                length = (LG + 1);
                link->stateR = E_LSDU;
                buffer[buf_index++] = data;
                break;

            case E_LSDU:
                length--;
                if (length != 0) {
                    link->stateR = E_LSDU;
                }
                else {
                    link->stateR = E_CHK;
                }
                if (buf_index < KNX_LINK_EXT_FRAME_MAX) {
                    buffer[buf_index++] = data;
                }
                break;

            case E_CHK:
                if (buf_index < KNX_LINK_EXT_FRAME_MAX) {
                    buffer[buf_index++] = data;
                }
                if (sa == link->ia) {
                    link->stateR = E_DATA_CONF;
                }
                else {
                    frame_index = knxLinkPoolLinkLock();
                    if (frame_index >= 0) {
                        frame = knxLinkPoolLinkGet(frame_index);
                        if (frame != NULL) {
                            if (knxLinkDecodeFrame(frame, buffer, buf_index) == 1) {
                                xQueueSend(link->knxLinkDataInd, &frame_index, portMAX_DELAY);
                                knxLinkPoolLinkYieldLock(frame_index);
                            }
                            else {
                                knxLinkPoolLinkUnLock(frame_index);
                            }
                        }
                        else {
                            knxLinkPoolLinkUnLock(frame_index);
                        }
                    }
                    link->stateR = E_CTRL;
                }
                break;

            case E_DATA_CONF:
                if (data == TPUART_RESPONSE_DATA_CONFIRMATION_POS) {
                    L_Data_Confirm = KNX_LINK_DATA_CON_POS;
                }
                else {
                    L_Data_Confirm = KNX_LINK_DATA_CON_NEG;
                }
                con.frame_index = link->txSlot;
                con.confirmation = L_Data_Confirm;
                xQueueSend(link->knxLinkDataCon, &con, portMAX_DELAY);
                break;

            default:
                break;
            }

            default: // KNX_LINK_ILLEGAL_STATE, KNX_LINK_MONITOR_STATE o KNX_LINK_STOP_STATE
                /* No hacer nada = descartar dato recibido */
                break;
        }
        xSemaphoreGive(link->knxLinkResetSem);
    }
}

static void _knxLinkTxThread(void *arg0) {
	knxLinkHandle_t *link = (knxLinkHandle_t *)arg0;
	int frame_index = 0;
    knxLinkFrame_t frame[frame_index];
    uint8_t encoded_frame[1] = {0} ;
    char txBuffer[1] = {encoded_frame[1]};

    while(1) {

        xQueueReceive(link->knxLinkDataReq, &frame_index, portMAX_DELAY);

        knxLinkEncodeFrame(frame, encoded_frame, sizeof(encoded_frame));

        knxLinkAdapterWriteBuffer(link->uartKNX, txBuffer, 1);
    }
}
