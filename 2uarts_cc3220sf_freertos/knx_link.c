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
static knxLinkHandle_t knxLinkParams;

/**
 * Variable privada con todos los parámetros del data confirmation
 */
static knxLinkDataCon_t knxLinkDataParams;

/**
 * @brief Reception task, manage incoming data from TP-UART 
 */
static void _knxLinkRecvThread(void *arg0);

/**
 * @brief Transmission task, manage requests from upper level
 */
static void _knxLinkTxThread(void *arg0);


// ............................................................................
// ============================================================================
//                      DEFINICIONES/IMPLEMENTACIÓN
// ============================================================================
// ............................................................................


struct knxLinkHandle_s * knxLinkInit(int link, int bps, int parity) {

	// Resto del código ...

    knxLinkParams.uartKNX = knxLinkAdapterOpen(link, bps, parity);
    /**
     * @TODO
     * Inicializar mutex uartKNXMutex.
     * Si error, while(1);
     */

    int res;
    res = pthread_mutex_init(&knxLinkParams.uartKNXMutex, NULL);

    if (res != NULL) {
           while(1);
    }


    /**
     * @TODO
     * Crear cola knxLinkResetCon
     * Crear semáforo knxLinkResetSem
     * Si error, while(1);
     */

    knxLinkParams.knxLinkResetCon = xQueueCreate(KNX_LINK_QUEUE_LENGTH, 1);

    knxLinkParams.knxLinkResetSem = xSemaphoreCreateBinary();
    if (knxLinkParams.knxLinkResetSem == NULL) {
        while(1);
    }

    /**
     * @TODO
     * Crear colas knxLinkDataCon, knxLinkDataInd
     * Crear semáforo knxLinkDataSem
     * Si error, while(1);
     */

    knxLinkParams.knxLinkDataCon = xQueueCreate(KNX_LINK_QUEUE_LENGTH, sizeof(knxLinkDataCon_t));

    knxLinkParams.knxLinkDataInd = xQueueCreate(KNX_LINK_QUEUE_LENGTH, sizeof(int));

    knxLinkParams.knxLinkDataSem = xSemaphoreCreateBinary();
       if (knxLinkParams.knxLinkDataSem == NULL) {
           while(1);
           }

    /**
     * @TODO
     * Crear tarea knxLinkRecvThread
     * Si error, while(1);
     */

       TaskHandle_t knxLinkRecvThreadHandle = NULL;
       BaseType_t ret;
       ret = xTaskCreate(_knxLinkRecvThread, "knxLinkRecvThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &knxLinkRecvThreadHandle);

       if ( ret == pdPASS ) {
           while(1);
       }


       TaskHandle_t knxLinkTxThreadHandle = NULL;
       BaseType_t ret1;
       ret1 = xTaskCreate(_knxLinkTxThread, "knxLinkTxThread", US_STACK_DEPTH, (void*) 0, tskIDLE_PRIORITY, &knxLinkTxThreadHandle);

       if ( ret1 == pdPASS ) {
           while(1);
       }

    /**
     * @TODO
     * Inicializar parámetros nivel de enlace, pool, etc.
     */

       knxLinkParams.state = KNX_LINK_INIT_STATE;
       knxLinkParams.stateR = E_CTRL;
       knxLinkParams.ia = KNX_IA_ADDRESS;
       knxLinkParams.da = KNX_DA_ADDRESS;
       knxLinkParams.ga = KNX_GA_ADDRESS;

       knxLinkFramePoolInit();

       return UART_STATUS_SUCCESS;
}



// ___---=== Definiciones relacionadas con el servicio de set-address ===---___

int knxLinkSetAddressReq(knxLinkHandle_t *link, uint16_t ia) {
	// Variables locales que nos harán falta:
    uint8_t ctrl = 0;
    char buf[1] = {TPUART_CTRLFIELD_SET_ADDRESS};
    char buffer[2] = {ctrl, ia};

    /**
     * @TODO
     * Implementar primitiva request del servicio set-address
     * Hacerlo sólo si el estado del nivel de enlace es KNX_LINK_INIT_STATE
     */
    if (knxLinkGetState(link) == KNX_LINK_INIT_STATE) {

	// Resto del código ...

        pthread_mutex_lock(&knxLinkParams.uartKNXMutex);

        knxLinkAdapterWriteBuffer(knxLinkParams.uartKNX, buf, 1);
        knxLinkAdapterWriteBuffer(knxLinkParams.uartKNX, buffer, 2);

        pthread_mutex_unlock(&knxLinkParams.uartKNXMutex);
    }

    return 1;
}



// ___---=== Definiciones relacionadas con el servicio de reset ===---___

int knxLinkResetReq(knxLinkHandle_t *link) {
	// Variables locales que nos harán falta:
    uint8_t ctrl = 0;
    char buf[1] = {TPUART_CTRLFIELD_RESET_REQUEST};
    char buffer[1] = {ctrl};
    //à transmetre: ctrl seulement

    /**
     * @TODO
     * Implementar primitiva request del servicio reset
     * Hacerlo sólo si el estado del nivel de enlace es KNX_LINK_INIT_STATE
     */

    if (knxLinkParams.knxLinkResetSem != NULL) {
        if( xSemaphoreTake(knxLinkParams.knxLinkResetSem, portMAX_DELAY ) == pdTRUE ) {

            if (knxLinkGetState(link) == KNX_LINK_INIT_STATE) {

                // Resto del código ...

                pthread_mutex_lock(&knxLinkParams.uartKNXMutex);

                knxLinkAdapterWriteBuffer(knxLinkParams.uartKNX, buf, 1);
                knxLinkAdapterWriteBuffer(knxLinkParams.uartKNX, buffer, 1);

                pthread_mutex_unlock(&knxLinkParams.uartKNXMutex);

                xSemaphoreGive(knxLinkParams.knxLinkResetSem);

            }
        }

    }

    return 1;
}

uint8_t knxLinkResetCon(knxLinkHandle_t *link) {

    uint8_t con = 0;

    if (knxLinkResetReq(link) == 1) {
        con = KNX_LINK_RESET_CON_POS;
    }
    else if (knxLinkResetReq(link) == 0) {
        con = KNX_LINK_RESET_CON_NEG;
    }
    else {
        con = KNX_LINK_RESET_CON_ERROR;
    }

    xQueueSend(knxLinkParams.knxLinkResetCon, &con, portMAX_DELAY );

    return con;
}

// ___---=== Definiciones relacionadas con el servicio de datos ===---___

int knxLinkDataReq(struct knxLinkHandle_s *link, int frame_index) {
	// Variables locales que nos harán falta:
    uint16_t ia = 0;
    char buffer[1] = {ia};
    int k = 0;
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

    xQueueReceive(knxLinkParams.knxLinkDataReq, &frame_index, portMAX_DELAY);

    if (knxLinkParams.knxLinkDataSem != NULL) {
        if( xSemaphoreTake(knxLinkParams.knxLinkDataSem, portMAX_DELAY ) == pdTRUE ) {

            if (knxLinkGetState(link) != KNX_LINK_NORMAL_STATE) {

                if (buffer != 0) {

                    int i = 0;
                    knxLinkPoolAppYieldLock(i);

                    knxLinkFrame_t frame[frame_index];
                    knxLinkEncodeFrame(frame, encoded_frame, sizeof(encoded_frame));

                    pthread_mutex_lock(&knxLinkParams.uartKNXMutex);

                    for (k=0; k < len-1; k++) {
                        Bufftemporal[0] = TPUART_CTRLFIELD_DATA_START || TPUART_CTRLFIELD_DATA_CONT(frame_index) || TPUART_CTRLFIELD_DATA_END(len);
                        Bufftemporal[1] = encoded_frame[k];
                        knxLinkAdapterWriteBuffer(knxLinkParams.uartKNX, Bufftemporal, 2);
                    }

                    pthread_mutex_unlock(&knxLinkParams.uartKNXMutex);

                    xSemaphoreGive(knxLinkParams.knxLinkDataSem);
                }
            }
        }
    }
    return 1;
}


knxLinkDataCon_t knxLinkSendDataCon(knxLinkHandle_t *link) {

    if (knxLinkDataReq(link, knxLinkDataParams.frame_index) == 1) {
        knxLinkDataParams.confirmation = KNX_LINK_DATA_CON_POS;
    }
    else if (knxLinkDataReq(link, knxLinkDataParams.frame_index) == 0) {
        knxLinkDataParams.confirmation = KNX_LINK_DATA_CON_NEG;
    }
    else {
        knxLinkDataParams.confirmation = KNX_LINK_DATA_CON_ERROR;
    }

    xQueueSend(knxLinkParams.knxLinkDataCon, &knxLinkDataParams.confirmation, portMAX_DELAY);
    xQueueSend(knxLinkParams.knxLinkDataCon, &knxLinkDataParams.frame_index, portMAX_DELAY);

    return knxLinkDataParams;
}


int knxLinkDataInd(knxLinkHandle_t *link) {

    int frame_index = 0;
    if (knxLinkDataReq(link, frame_index) == 1) {
        xQueueSend(knxLinkParams.knxLinkDataInd, &frame_index, portMAX_DELAY);
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
    uint8_t CHK = 0;
    int len = KNX_LINK_STD_FRAME_MAX;
    char frame[len];
    uint8_t FT[1];
    uint8_t saH[1];
    uint8_t saL[1];
    uint8_t daH[1];
    uint8_t daL[1];
    uint8_t LG[len-1];
    uint8_t *buffer;
    char L_Data_Confirm = 0;
    int i, frame_index;
    knxLinkDataCon_t conf;

    while(1) {
        knxLinkAdapterReadChar(knxLinkParams.uartKNX);
        switch(knxLinkParams.state) {
        case KNX_LINK_INIT_STATE:
            /**
             * @TODO
             * Enviar KNX_LINK_RESET_CON_POS a cola knxLinkResetCon si dato == TPUART_RESPONSE_RESET_CONFIRMATION,
             * enviar KNX_LINK_RESET_CON_NEG a cola knxLinkResetCon en otro caso
             */
			// Resto del código ...

            if (dato == TPUART_RESPONSE_RESET_CONFIRMATION) {
                reset_con = KNX_LINK_RESET_CON_POS;
            }
            else {
                reset_con = KNX_LINK_RESET_CON_NEG;
            }
            xQueueSend(knxLinkParams.knxLinkResetCon, &reset_con, portMAX_DELAY);
            break;

        case KNX_LINK_NORMAL_STATE:
            /**
             * @TODO
             * Máquina de estados con almacenamiento de la trama entrante
             */
			// Resto del código ...

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
                    frame[i] = knxLinkAdapterReadChar(knxLinkParams.uartKNX);
                }

                if (length != 0) {
                    link->stateR = E_LSDU;
                }
                else {
                    link->stateR = E_CHK;
                }
                break;

            case E_CHK:
                CHK = knxLinkCalcCHK(buffer, 1);
                knxLinkVerifyCHK(buffer, 1, CHK);

                if (knxLinkVerifyCHK(buffer, 1, CHK) == 1) {
                    if ((daH[1] + daL[1]) == knxLinkParams.ia) {
                        xQueueSend(knxLinkParams.knxLinkDataCon, &conf, portMAX_DELAY);
                    }
                    else {
                        xQueueSend(knxLinkParams.knxLinkDataInd, &frame_index, portMAX_DELAY);
                    }
                }
                link->stateR = E_DATA_CONF;
                break;

            case E_DATA_CONF:
                L_Data_Confirm = knxLinkAdapterReadChar(knxLinkParams.uartKNX);
                knxLinkVerifyCHK(buffer, 1, CHK);

                if (knxLinkVerifyCHK(buffer, 1, CHK) == 1) {
                    L_Data_Confirm = TPUART_RESPONSE_DATA_CONFIRMATION_POS;
                }
                else {
                    L_Data_Confirm = TPUART_RESPONSE_DATA_CONFIRMATION_NEG;
                }
                knxLinkAdapterWriteBuffer(knxLinkParams.uartKNX, &L_Data_Confirm, 1);
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
	xQueueReceive(knxLinkParams.knxLinkDataReq, &frame_index, portMAX_DELAY);

    knxLinkEncodeFrame(frame, encoded_frame, sizeof(encoded_frame));

    knxLinkAdapterWriteBuffer(knxLinkParams.uartKNX, txBuffer, 1);

    }
}
