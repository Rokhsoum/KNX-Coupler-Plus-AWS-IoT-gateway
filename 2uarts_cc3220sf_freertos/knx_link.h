/*
 * @file knx_link.h
 * @brief KNX library project
 */

#ifndef KNX_LINK_H_
#define KNX_LINK_H_

#include <stdint.h>
#include "knx_link_adapter.h"
#include "knx_link_state.h"
#include "knx_link_frame.h"
#include "knx_link_frame_pool.h"
#include "knx_link_gadd_pool.h"

#define US_STACK_DEPTH              512         //Number of words to allocate for use as the task's stack.

#define KNX_LINK_QUEUE_LENGTH       10          //Number of units that a queue can handle

#define KNXLINK_RESET_CON_TIMEOUT   50 / portTICK_PERIOD_MS

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


int knxLinkDataReq(struct knxLinkHandle_s *link, int frame_index);

/**
 * @brief Data service, confirmation primitive
 * @param[in] link KNX link handle from knxLinkInit()
 * @param[in] datacon pointer to the confirmation return value, a confirmation value of KNX_LINK_DATA_CON_ERROR if error, either KNX_LINK_DATA_CON_POS or KNX_LINK_DATA_CON_NEG otherwise.
 * @remarks Blocks caller on the service confirmation queue
 * @return 1 in NORMAL MODE, 0 otherwise
 * In case of KNX_LINK_DATA_CON_ERROR confirmation value the frame_index value is invalid (-1).
 */
int knxLinkDataCon(struct knxLinkHandle_s *link, knxLinkDataCon_t *dataCon);
/**
 * @brief Data service, indication primitive
 * @param[in] link KNX link handle from knxLinkInit()
 * @param[in] frame_index pointer to the frame_index return value
 * @remarks Blocks caller on the service indication queue
 * @return 0 if error, 1 otherwise
 */
int knxLinkDataInd(struct knxLinkHandle_s *link, int *frame_index);

#endif /* KNX_LINK_H_ */

