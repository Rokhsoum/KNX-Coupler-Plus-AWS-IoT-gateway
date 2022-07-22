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
 * Tipo estructurado con todos los parámetros del nivel de enlace
 */
typedef struct knxLinkHandle_s {
    /* UART */
    knxLink_uart_t uartKNX;             /**< Handle de acceso a la UART */
    pthread_mutex_t uartKNXMutex;       /**< Mutex para garantizar acceso en exclusión mutua a la UART */
    /* Reset service */
    SemaphoreHandle_t knxLinkResetSem;  /**< Semáforo para reset request */
    QueueHandle_t knxLinkResetCon;      /**< Cola confirmación reset (datos uint8_t, valor confirmación) */
    /* Data service */
    QueueHandle_t knxLinkDataReq;       /**< Cola request datos (int frame_index) */
    QueueHandle_t knxLinkDataCon;       /**< Cola confirmación datos (datos knxLinkDataCon_t) */
    QueueHandle_t knxLinkDataInd;       /**< Cola indicación datos (uint8_t, valor frame_index) */
    //SemaphoreHandle_t knxLinkDataSem;   /**< Semáforo para data request */
    /* Resto de parámetros del nivel de enlace */
    uint16_t ia;                        /**< Dirección individual del dispositivo */
    //uint16_t ga;                        /**< Dirección de grupo relevante para un objeto de comunicación*/
    //uint16_t da;                        /**< Dirección individual del destination */
    int state;                          /**< Estado del nivel de enlace */
    int stateR;                         /**< Estado de la recepción de datos */
    uint8_t txSlot;                     /**< Índice del telegrama en el pool, transmisión (conservar de data request para data confirmation) */
    //uint8_t rxSlot;                     /**< Índice del telegrama en el pool, recepción (para data indication) */
    uint16_t rxiaframe;                 /**< Dirección individual del trama recibida*/  //à comparer avec ia pour savoir si remote or local
} knxLinkHandle_t;

#endif

