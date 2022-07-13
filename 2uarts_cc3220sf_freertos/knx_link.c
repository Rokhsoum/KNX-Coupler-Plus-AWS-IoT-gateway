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
#include "knx_commissioning_data.h"

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
static knxLinkHandle_t knxLinkParams[knxLink_handleused];

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
       knxLinkParams[knxLink_handleused].ia = KNX_PERSONAL_ID;
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
    if (knxLinkParams[knxLink_handleused].knxLinkResetSem == NULL && knxLinkGetState(link) != KNX_LINK_INIT_STATE) {
        return 0;
    }

    if (knxLinkParams[knxLink_handleused].knxLinkResetSem != NULL) {
        if( xSemaphoreTake(knxLinkParams[knxLink_handleused].knxLinkResetSem, portMAX_DELAY ) == pdTRUE ) {

            if (knxLinkGetState(link) == KNX_LINK_INIT_STATE) {

                pthread_mutex_lock(&knxLinkParams[knxLink_handleused].uartKNXMutex);

                knxLinkAdapterWriteBuffer(knxLinkParams[knxLink_handleused].uartKNX, buf, 1);

                pthread_mutex_unlock(&knxLinkParams[knxLink_handleused].uartKNXMutex);

                //xSemaphoreGive(knxLinkParams[knxLink_handleused].knxLinkResetSem); on le give aprés dans RecvThread
            }
        }

    }

    return 1;
}

uint8_t knxLinkResetCon(knxLinkHandle_t *link) {

    uint8_t con = 0;
/**
    if (knxLinkResetReq(link) == 1) {
        con = KNX_LINK_RESET_CON_POS;
    }
    else if (knxLinkResetReq(link) == 0) {
        con = KNX_LINK_RESET_CON_NEG;
    }
    else {
        con = KNX_LINK_RESET_CON_ERROR;
    }
*/
    xQueueSend(link->knxLinkResetCon, &con, portMAX_DELAY );

    return con;
}

// ___---=== Definiciones relacionadas con el servicio de datos ===---___

static void _knxLinkDataReqThread(void *arg0) {
    knxLinkHandle_t *link = (knxLinkHandle_t *)arg0;
    // Variables locales que nos harán falta:
    int frame_index, k = 0;
    int len = KNX_LINK_STD_FRAME_MAX;
    uint8_t encoded_frame[len];
    char Bufftemporal[2];
    frame_index = knxLinkPoolLinkLock();

    /**
     * @TODO
     * Implementar primitiva request del servicio datos
     * Hacerlo sólo si el estado del nivel de enlace es KNX_LINK_NORMAL_STATE
     * Obtener puntero a la trama (frame) a partir del índice dentro del pool (frame_index),
     * verificando antes que frame_index corresponde a un hueco del pool
     * reservado al alto nivel
     */

    while(1) {

        xQueueReceive(knxLinkParams[knxLink_handleused].knxLinkDataReq, &frame_index, portMAX_DELAY);

        if (knxLinkGetState(link) == KNX_LINK_NORMAL_STATE && knxLinkParams[knxLink_handleused].ia == 0) {
            while(1);
        }

        if (knxLinkGetState(link) != KNX_LINK_NORMAL_STATE) {

            if (knxLinkParams[knxLink_handleused].ia != 0) {

                knxLinkPoolAppYieldLock(frame_index);

                knxLinkFrame_t frame[frame_index];
                knxLinkEncodeFrame(frame, encoded_frame, sizeof(encoded_frame)); //juste frame si y erreur

                pthread_mutex_lock(&knxLinkParams[knxLink_handleused].uartKNXMutex);

                for (k=0; k < len-1; k++) {
                    Bufftemporal[0] = TPUART_CTRLFIELD_DATA_START || TPUART_CTRLFIELD_DATA_CONT(frame_index) || TPUART_CTRLFIELD_DATA_END(len);
                    Bufftemporal[1] = encoded_frame[k];
                    knxLinkAdapterWriteBuffer(knxLinkParams[knxLink_handleused].uartKNX, Bufftemporal, 2);
                }

                pthread_mutex_unlock(&knxLinkParams[knxLink_handleused].uartKNXMutex);
            }
        }
    }
}


knxLinkDataCon_t knxLinkSendDataCon(knxLinkHandle_t *link) {

#if 0
    if (knxLinkDataReq(link, knxLinkDataParams.frame_index) == 1) {
        knxLinkDataParams.confirmation = KNX_LINK_DATA_CON_POS;
    }
    else if (knxLinkDataReq(link, knxLinkDataParams.frame_index) == 0) {
        knxLinkDataParams.confirmation = KNX_LINK_DATA_CON_NEG;
    }
    else {
        knxLinkDataParams.confirmation = KNX_LINK_DATA_CON_ERROR;
    }
#endif

    knxLinkDataCon_t dataCon;

    if (knxLinkAdapterReadChar(knxLinkParams[knxLink_handleused].uartKNX) == TPUART_RESPONSE_DATA_CONFIRMATION_POS) {
        dataCon.confirmation = KNX_LINK_DATA_CON_POS;
    }
    else if (knxLinkAdapterReadChar(knxLinkParams[knxLink_handleused].uartKNX) == TPUART_RESPONSE_DATA_CONFIRMATION_NEG) {
        dataCon.confirmation = KNX_LINK_DATA_CON_NEG;
    }
    else {
        dataCon.confirmation = KNX_LINK_DATA_CON_ERROR;
    }

    xQueueSend(knxLinkParams[knxLink_handleused].knxLinkDataCon, &dataCon, portMAX_DELAY);

    return dataCon;
}


int knxLinkDataInd(knxLinkHandle_t *link) {

    int frame_index = 0;
    if (knxLinkAdapterReadChar(knxLinkParams[knxLink_handleused].uartKNX) == TPUART_RESPONSE_DATA_CONFIRMATION_POS) {
        xQueueSend(knxLinkParams[knxLink_handleused].knxLinkDataInd, &frame_index, portMAX_DELAY);
        return frame_index;
    }
    else {
        return -1;
    }
}


static void _knxLinkRecvThread(void *arg0) {
	knxLinkHandle_t *link = (knxLinkHandle_t *)arg0;
    uint8_t dato;
    uint8_t reset_con;
    uint8_t length = 0;
    //uint8_t CHK = 0;
    int len = KNX_LINK_EXT_FRAME_MAX;
    knxLinkFrame_t frame[len];
    uint8_t FT[1];
    uint8_t saH[1];
    uint8_t saL[1];
    uint8_t daH[1];
    uint8_t daL[1];
    uint8_t LG[KNX_LINK_EXT_FRAME_LSDU_MAX];
    uint8_t *buffer;
    uint8_t iaH = 0, iaL = 0;
    char L_Data_Confirm = 0;
    int i;
    knxLinkDataCon_t con;
    int frame_index = knxLinkPoolLinkLock();

    while(1) {
        knxLinkAdapterReadChar(knxLinkParams[knxLink_handleused].uartKNX);
        switch(knxLinkParams[knxLink_handleused].state) {
        case KNX_LINK_INIT_STATE:
            /**
             * @TODO
             * Enviar KNX_LINK_RESET_CON_POS a cola knxLinkResetCon si dato == TPUART_RESPONSE_RESET_CONFIRMATION,
             * enviar KNX_LINK_RESET_CON_NEG a cola knxLinkResetCon en otro caso
             */

            if (dato == TPUART_RESPONSE_RESET_CONFIRMATION) {
                reset_con = KNX_LINK_RESET_CON_POS;
            }
            else {
                reset_con = KNX_LINK_RESET_CON_NEG;
            }
            xQueueSend(knxLinkParams[knxLink_handleused].knxLinkResetCon, &reset_con, portMAX_DELAY);
            break;

        case KNX_LINK_NORMAL_STATE:
            /**
             * @TODO
             * Máquina de estados con almacenamiento de la trama entrante
             */

            switch(link->stateR) {
            case E_CTRL:
                memcpy(FT, &frame[7], 1);
                if ( FT == 0 ) {
                    link->stateR = E_SA_H;
                }
                else {
                    link->stateR = E_CTRLE;
                }
                break;

            case E_SA_H:
                if (FT == 0) {
                    memcpy(saH, &frame[7], 8);
                }
                else {
                    memcpy(saH, &frame[15], 8);
                }
                link->stateR = E_SA_L;
                break;

            case E_SA_L:
                if (FT == 0) {
                    memcpy(saL, &frame[15], 8);
                }
                else {
                    memcpy(saL, &frame[23], 8);
                }
                link->stateR = E_DA_H;
                break;

            case E_DA_H:
                if (FT == 0) {
                    memcpy(daH, &frame[23], 8);
                }
                else {
                    memcpy(daH, &frame[31], 8);
                }
                link->stateR = E_DA_L;
                break;

            case E_DA_L:
                if (FT == 0) {
                    memcpy(daL, &frame[31], 8);
                link->stateR = E_AT_LSDU_LG;
                }
                else {
                    memcpy(daL, &frame[39], 8);
                    link->stateR = E_LG;
                }
                break;

            case E_AT_LSDU_LG:
                memcpy(LG, &frame[43], len-1);
                link->stateR = E_LSDU;
                break;

            case E_LG:
                memcpy(LG, &frame[47], len-1);
                link->stateR = E_LSDU;
                break;

            case E_LSDU:
                length = sizeof(LG + 1);
                for (i = length; i >= 0; i--) {
                    LG[i] = knxLinkAdapterReadChar(knxLinkParams[knxLink_handleused].uartKNX);
                }

                if (length != 0) {
                    link->stateR = E_LSDU;
                }
                else {
                    link->stateR = E_CHK;
                }
                break;

            case E_CHK:
                iaH = (uint8_t)((knxLinkParams[knxLink_handleused].ia & 0xFF00) >> 8);
                iaL = (uint8_t)(knxLinkParams[knxLink_handleused].ia & 0x00FF);
                if (daH[1] == iaH && daL[1] == iaL) {
                    xQueueSend(knxLinkParams[knxLink_handleused].knxLinkDataCon, &con, portMAX_DELAY);
                }
                else {
                    frame_index = knxLinkDecodeFrame(frame, buffer, 1);
                    xQueueSend(knxLinkParams[knxLink_handleused].knxLinkDataInd, &frame_index, portMAX_DELAY);
                    knxLinkPoolLinkLock();
                }
                link->stateR = E_DATA_CONF;
                break;

            case E_DATA_CONF:
                L_Data_Confirm = knxLinkAdapterReadChar(knxLinkParams[knxLink_handleused].uartKNX);
                xQueueSend(knxLinkParams[knxLink_handleused].knxLinkDataCon, &L_Data_Confirm, portMAX_DELAY);
                break;

            default:
                break;
            }

            default: // KNX_LINK_ILLEGAL_STATE, KNX_LINK_MONITOR_STATE o KNX_LINK_STOP_STATE
                /* No hacer nada = descartar dato recibido */
                break;
        }
    }
}

static void _knxLinkTxThread(void *arg0) {
	//knxLinkHandle_t *link = (knxLinkHandle_t *)arg0;
	int frame_index = 0;
    knxLinkFrame_t frame[frame_index];
    uint8_t encoded_frame[1] = {0} ;
    char txBuffer[1] = {encoded_frame[1]};

    while(1) {

        xQueueReceive(knxLinkParams[knxLink_handleused].knxLinkDataReq, &frame_index, portMAX_DELAY);

        knxLinkEncodeFrame(frame, encoded_frame, sizeof(encoded_frame));

        knxLinkAdapterWriteBuffer(knxLinkParams[knxLink_handleused].uartKNX, txBuffer, 1);
    }
}
