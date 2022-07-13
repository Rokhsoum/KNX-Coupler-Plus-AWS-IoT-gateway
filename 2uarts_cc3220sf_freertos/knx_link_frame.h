/*
 * @file knx_link_frame.h
 * @brief KNX library project
 */

#ifndef KNX_LINK_FRAME_H_
#define KNX_LINK_FRAME_H_

#include <stdlib.h>
#include "knx_link.h"

// ___---=== Declaraciones relacionadas con el tipo de telegramas KXN ===---___

/**
 * @brief Max length of LSDU field in a standard frame
 */
#define KNX_LINK_STD_FRAME_LSDU_MAX  	16

/**
 * @brief Max length of a standard frame
 */
#define KNX_LINK_STD_FRAME_MAX     		(KNX_LINK_STD_FRAME_LSDU_MAX+7)

/**
 * @brief Max length of LSDU field in a extended frame as stated in KNX standard
 */
#define KNX_LINK_EXT_FRAME_LSDU_MAX    	256

/**
 * @brief Max length of LSDU field in a extended frame restricted to TP-UART buffer memory size
 */
#define KNX_LINK_EXT_FRAME_LSDU_MAX_TPUART	56

/**
 * @brief Max length of LSDU field in a extended frame we can manage (with a TP-UART)
 */
//#define KNX_LINK_EXT_FRAME_LSDU_MAX			min(KNX_LINK_EXT_FRAME_LSDU_MAX, TPUART_LG_FRAME_LSDU_MAX)

/**
 * @brief Max length of a extended frame we can manage (with a TP-UART)
 */
#define KNX_LINK_EXT_FRAME_MAX    			(KNX_LINK_EXT_FRAME_LSDU_MAX+8)


/**
 * @brief KNX frame from the application point of view
 * This is the datatype used in the frame pool repository
 */
typedef struct {
    uint8_t rep;                                        /**< Repeated frame in the pool (0 = yes, 1 = no) */
    uint8_t prio;                                       /**< Frame priority to ordinate them in the pool */
    uint16_t sa;                                        /**< Source address (16-bit Individual address) */
    uint16_t da;                                        /**< Destination address (16-bit Individual address / 15-bit group address) */
    uint8_t at;                                         /**< Type of Destination address (0 = individual, 1 = group) */
    uint8_t hop_count;                                  /**< Hop count : used to limit breaks between lines (default: 7),
						                                decreased each time a coupler forwards the frame */
    uint8_t ext_ff;                                     /**< Ext. frame format (for EXT frames only),
                                                        unused (default: 0) */
    uint16_t length;                                    /**< LSDU length */
    uint8_t lsdu[KNX_LINK_EXT_FRAME_LSDU_MAX];          /**< Link Service Data Unit */
} knxLinkFrame_t;


/**
 * Calcular el campo checksum de un array de bytes según estándar KNX
 * @param[in] buffer Array con los bytes a utilizar en el cálculo
 * @param[in] bufsize Número de elementos utilizados en el array buffer
 * @return Checksum calculado
 * @TOOD: A desarrollar
 */
uint8_t knxLinkCalcCHK(uint8_t *buffer, unsigned int bufsize);

/**
 * Verificar si el checksum calculado para un array de bytes según el estándar KNX es correcto
 * @param[in] buffer Array con los bytes a utilizar en el cálculo
 * @param[in] bufsize Número de elementos utilizados en el array buffer
 * @param[in] chk Valor esperado del checksum
 * @return 1 si la verificación es exitosa, 0 en caso contrario
 * @TOOD: A desarrollar
 */
int knxLinkVerifyCHK(uint8_t *buffer, unsigned int bufsize, uint8_t CHK);

/**
 * Codificar una trama de nivel de enlace como un array de bytes
 * @param[in] frame Telegrama como valor estructurado
 * @param[in] buffer Array para almacenar los bytes de la codificación
 * @param[in] bufsize Tamaño del array, número máximo de bytes de la codificación
 * @return Longitud de bytes del telegrama codificado si la codificación es exitosa, -1 en caso contrario
 * @TOOD: A desarrollar
 */
int knxLinkEncodeFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize);

/**
 * Decodificar un array de bytes como una trama de nivel de enlace
 * @param[in] frame Telegrama como valor estructurado
 * @param[in] buffer Array de donde extraer los bytes para la decodificación
 * @param[in] bufsize Tamaño del array, número máximo de bytes de la decodificación
 * @return 1 si la decodificación es exitosa, 0 en caso contrario
 * @TOOD: A desarrollar
 */
int knxLinkDecodeFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize);

#endif /* KNX_LINK_FRAME_H_ */

