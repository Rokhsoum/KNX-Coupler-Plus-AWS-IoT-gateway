/*
 * @file knx_link_internal.h
 * @brief KNX library project
 */
#ifndef KNX_LINK_INTERNAL_H_
#define KNX_LINK_INTERNAL_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include <queue.h>
#include <semphr.h>
#include <pthread.h>
#include "knx_link_frame.h"
#include "knx_link_adapter.h"
#include <ti/drivers/UART.h>

/**
 * Tipo estructurado con todos los par�metros del nivel de enlace
 */
typedef struct knxLinkHandle_s {
    /* UART */
    knxLink_uart_t uartKNX;             /**< Handle de acceso a la UART */
    pthread_mutex_t uartKNXMutex;       /**< Mutex para garantizar acceso en exclusi�n mutua a la UART */
    /* Reset service */
    SemaphoreHandle_t knxLinkResetSem;  /**< Sem�foro para reset request */
    QueueHandle_t knxLinkResetCon;      /**< Cola confirmaci�n reset (datos uint8_t, valor confirmaci�n) */
    /* Data service */
    QueueHandle_t knxLinkDataReq;       /**< Cola request datos (int frame_index) */
    QueueHandle_t knxLinkDataCon;       /**< Cola confirmaci�n datos (datos knxLinkDataCon_t) */
    QueueHandle_t knxLinkDataInd;       /**< Cola indicaci�n datos (uint8_t, valor frame_index) */
    //SemaphoreHandle_t knxLinkDataSem;   /**< Sem�foro para data request */
    /* Resto de par�metros del nivel de enlace */
    uint16_t ia;                        /**< Direcci�n individual del dispositivo */
    //uint16_t ga;                        /**< Direcci�n de grupo relevante para un objeto de comunicaci�n*/
    //uint16_t da;                        /**< Direcci�n individual del destination */
    int state;                          /**< Estado del nivel de enlace */
    int stateR;                         /**< Estado de la recepci�n de datos */
    uint8_t txSlot;                     /**< �ndice del telegrama en el pool, transmisi�n (conservar de data request para data confirmation) */
    //uint8_t rxSlot;                     /**< �ndice del telegrama en el pool, recepci�n (para data indication) */
    uint16_t rxiaframe;                 /**< Direcci�n individual del trama recibida*/  //� comparer avec ia pour savoir si remote or local
} knxLinkHandle_t;

#endif

