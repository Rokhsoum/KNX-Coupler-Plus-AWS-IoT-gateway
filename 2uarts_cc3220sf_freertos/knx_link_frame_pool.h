/*
 * @file knx_link_frame_pool.h
 * @brief KNX library project
 */

#ifndef KNX_LINK_FRAME_POOL_H_
#define KNX_LINK_FRAME_POOL_H_
 
#include <stdint.h>
#include "knx_link.h"
#include "knx_link_frame.h"


#define KNX_LINK_FRAME_POOL_SIZE            12


/**
 * @brief Frame slot flag values
 * A frame slot is available if its flag is KNX_LINK_POOL_FLAG_AVAILABLE,
 * it can be reserved using any flag value > KNX_LINK_POOL_FLAG_AVAILABLE and < KNX_LINK_POOL_FLAG_INVALID
 */
typedef enum knxLinkFramePoolUser_e {
	KNX_LINK_FRAME_POOL_FLAG_AVAILABLE = 0,
	KNX_LINK_FRAME_POOL_FLAG_APP = 1,
	KNX_LINK_FRAME_POOL_FLAG_LINK,
	KNX_LINK_FRAME_POOL_FLAG_INVALID
} knxLinkFramePoolUser_t;

struct knxLinkHandle_s;

/**
 * @brief Init the frame pool
 * @return Nothing
 * All frame slots are initially available (KNX_LINK_FRAME_POOL_FLAG_AVAILABLE).
 * @remarks A second, third call to this function does nothing
 */
void knxLinkFramePoolInit(void);



/**
 * @brief Reserve a frame slot for the knx application layer
 * @return -1 if error, slot index otherwise
 * Invoked from the knx application layer to reserve a frame slot,
 * fill in the data, and request delivery (to the knx link layer).
 * The data request needs the index i returned by this function as a parameter
 */
int knxLinkFramePoolAppLock(void);

/**
 * @brief Reserve a frame slot for the knx link layer
 * @return -1 if error, slot index otherwise
 * Invoked from the knx link layer to reserve a frame slot,
 * decode into it an incoming remote frame, and notify the arrival (indication, to the knx application layer).
 * The data indication needs the index i returned by this function as a parameter
 */
int knxLinkFramePoolLinkLock(void);


/**
 * @brief Transfer "ownership" of a frame slot owned by the knx app layer to the knx link layer
 * @param[in] i Slot index
 * @remarks If the slot isn't currently owned by the knx app layer this function does nothing
 */
void knxLinkFramePoolAppYieldLock(int i);

/**
 * @brief Transfer "ownership" of a frame slot owned by the knx link layer to the knx app layer
 * @param[in] i Slot index
 * @remarks If the slot isn't currently owned by the knx link layer this function does nothing
 */
void knxLinkFramePoolLinkYieldLock(int i);


/**
 * @brief Free a reserved frame slot owned by the knx app layer
 * @param[in] i Slot index
 * @remarks If the slot isn't currently owned by the knx app layer this function does nothing
 */
void knxLinkFramePoolAppUnLock(int i);

/**
 * @brief Free a reserved frame slot owned by the knx link layer
 * @param[in] i Slot index
 * @remarks If the slot isn't currently owned by the knx link layer this function does nothing
 */
void knxLinkFramePoolLinkUnLock(int i);


/**
 * @brief Get a pointer to a reserved frame slot owned by the knx app layer
 * @param[in] i Slot index
 * @return NULL if error, pointer to the slot otherwise
 * @remarks If the slot isn't currently owned by the knx app layer this function returns NULL
 */
knxLinkFrame_t *knxLinkFramePoolAppGet(int i);

/**
 * @brief Get a pointer to a reserved frame slot owned by the knx link layer
 * @param[in] i Slot index
 * @return NULL if error, pointer to the slot otherwise
 * @remarks If the slot isn't currently owned by the knx link layer this function returns NULL
 */
knxLinkFrame_t *knxLinkFramePoolLinkGet(int i);

#endif /* KNX_LINK_FRAME_POOL_H_ */
