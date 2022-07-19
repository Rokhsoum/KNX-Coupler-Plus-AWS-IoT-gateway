/*
 * @file knx_link.h
 * @brief KNX library project
 */

#ifndef KNX_LINK_H_
#define KNX_LINK_H_

#include <stdint.h>
#include "knx_link_adapter.h"


#define US_STACK_DEPTH              200         //Number of words to allocate for use as the task's stack.

#define KNX_LINK_QUEUE_LENGTH       10          //Number of units that a queue can handle


struct knxLinkHandle_s;

/**
 * Initialize KNX link
 * @return NULL if error, pointer to allocated link handle otherwise
 */
struct knxLinkHandle_s * knxLinkInit(uint16_t ia, knxLink_uart_t uartlink);

// ___---=== Set-Address service ===---___

/**
 * @brief Set-Address service, request primitive
 * @param[in] link KNX link handle from knxLinkInit()
 * @param[in] ia Physical/individual address of this device
 * @return 0 if error, 1 otherwise
 */
int knxLinkSetAddressReq(struct knxLinkHandle_s *link, uint16_t ia);


// ___---=== Reset service ===---___


#define KNX_LINK_RESET_CON_NEG   0       		// Negative confirmation value for the reset service
#define KNX_LINK_RESET_CON_POS   1       		// Positive confirmation value for the reset service
#define KNX_LINK_RESET_CON_ERROR ((uint8_t)-1) 	// Confirmation error value for the reset service

/**
 * @brief Reset service, request primitive
 * @param[in] link KNX link handle from knxLinkInit()
 * @return 0 if error, 1 otherwise
 */
int knxLinkResetReq(struct knxLinkHandle_s *link);

/**
 * @brief Reset service, confirmation primitive
 * @param[in] link KNX link handle from knxLinkInit()
 * @remarks Blocks caller on the service confirmation queue
 * @return KNX_LINK_RESET_CON_ERROR if error, either KNX_LINK_RESET_CON_POS or KNX_LINK_RESET_CON_NEG otherwise
 */
uint8_t knxLinkResetCon(struct knxLinkHandle_s *link);


// ___---=== Data service ===---___


#define KNX_LINK_DATA_CON_NEG   0       		// Negative confirmation value for the data service
#define KNX_LINK_DATA_CON_POS   1       		// Positive confirmation value for the data service
#define KNX_LINK_DATA_CON_ERROR ((uint8_t)-1)	// Confirmation error value for the data service


/**
 * @brief Datatype for the data confirmation primitive
 */
typedef struct {
    int frame_index;    	/**< Frame index in pool for the data service */
    uint8_t confirmation;   /**< Confirmation value for the data service */
} knxLinkDataCon_t;


/**
 * @brief Data service, confirmation primitive
 * @param[in] link KNX link handle from knxLinkInit()
 * @remarks Blocks caller on the service confirmation queue
 * @return A confirmation value of KNX_LINK_DATA_CON_ERROR if error, either KNX_LINK_DATA_CON_POS or KNX_LINK_DATA_CON_NEG otherwise.
 * In case of KNX_LINK_DATA_CON_ERROR confirmation value the frame_index value is invalid (-1).
 */
knxLinkDataCon_t knxLinkDataCon(struct knxLinkHandle_s *link);

/**
 * @brief Data service, indication primitive
 * @param[in] link KNX link handle from knxLinkInit()
 * @remarks Blocks caller on the service indication queue
 * @return -1 if error, frame index in pool otherwise
 */
int knxLinkDataInd(struct knxLinkHandle_s *link);

#endif /* KNX_LINK_H_ */

