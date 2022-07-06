/*
 * @file knx_link.h
 * @brief KNX library project
 */

#ifndef KNX_LINK_H_
#define KNX_LINK_H_

#include <stdint.h>

struct knxLinkHandle_s;

/**
 * Initialize KNX link
 * @return NULL if error, pointer to allocated link handle otherwise
 */
struct knxLinkHandle_s * knxLinkInit(int link, int bps, int parity);

// ___---=== Set-Address service ===---___

/**
 * @brief Set-Address service, request primitive
 * @param[in] link KNX link handle from knxLinkInit()
 * @param[in] ia Physical/individual address of this device
 * @return 0 if error, 1 otherwise
 */
int knxLinkSetAddressReq(struct knxLinkHandle_s *link, uint16_t ia);


// ___---=== Reset service ===---___

#define KNX_LINK_QUEUE_LENGTH    10             //Number of units that a queue can handle


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

//#define portMAX_DELAY           10      //The maximum amount of time the task should block waiting for space,
                                        //to become available on the queue, should it already be full.

/**
 * @brief Datatype for the data confirmation primitive
 */
typedef struct {
    int frame_index;    	/**< Frame index in pool for the data service */
    uint8_t confirmation;   /**< Confirmation value for the data service */
} knxLinkDataCon_t;


/**
 * @brief Data service, request primitive
 * @param[in] link KNX link handle from knxLinkInit()
 * @param[in] frame_index Frame index in the frame pool
 * The frame slot in the frame pool must be previously reserved and the frame fields filled
 * @return 0 if error, 1 otherwise
 */
int knxLinkDataReq(struct knxLinkHandle_s *link, int frame_index);


/**
 * @brief Data service, confirmation primitive
 * @param[in] link KNX link handle from knxLinkInit()
 * @remarks Blocks caller on the service confirmation queue
 * @return A confirmation value of KNX_LINK_DATA_CON_ERROR if error, either KNX_LINK_DATA_CON_POS or KNX_LINK_DATA_CON_NEG otherwise.
 * In case of KNX_LINK_DATA_CON_ERROR confirmation value the frame_index value is invalid (-1).
 */
knxLinkDataCon_t knxLinkSendDataCon(struct knxLinkHandle_s *link);

/**
 * @brief Data service, indication primitive
 * @param[in] link KNX link handle from knxLinkInit()
 * @remarks Blocks caller on the service indication queue
 * @return -1 if error, frame index in pool otherwise
 */
int knxLinkDataInd(struct knxLinkHandle_s *link);

/**
 * @brief Reception task, manage incoming data from TP-UART
 */
//void *_knxLinkRxThread(void *arg0);


/**
 * @brief Transmission task, manage requests from upper level
 */
//void *_knxLinkTxThread(void *arg0);


#define US_STACK_DEPTH  200 //Number of words to allocate for use as the task's stack.


/**
 * @brief Link level initialization values
 */
#define KNX_IA_ADDRESS    0x0000
#define KNX_DA_ADDRESS    0x0000
#define KNX_GA_ADDRESS    0x0000


#endif /* KNX_LINK_H_ */

