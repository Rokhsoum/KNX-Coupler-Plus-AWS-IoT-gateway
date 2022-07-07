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
	KNX_LINK_FRAME_POOL_FLAG_DOWNLINK,
	KNX_LINK_FRAME_POOL_FLAG_UPLINK,
	KNX_LINK_FRAME_POOL_FLAG_INVALID
} knxLinkFramePoolUser_t;

struct knxLinkHandle_s;

/**
 * @brief Init the frame pool
 * @return Nothing
 * All frame slots are initially available
 */
void knxLinkFramePoolInit(void);

/**
 * @brief Reserve a frame slot
 * @param[in] flag Frame slot flag
 * @return -1 if error, slot index otherwise
 * The reserved frame slot is marked with the value flag; further operations
 * on the reserved frame must match this value;
 * eg. a slot locked with KNX_LINK_POOL_FLAG_APP can only be unlocked with this same flag value.
 * If (flag <= KNX_LINK_POOL_FLAG_AVAILABLE) or (flag >= KNX_LINK_POOL_FLAG_INVALID)
 * the request is invalid.
 */
int knxLinkFramePoolLock(knxLinkFramePoolUser_t flag);

/**
 * @brief Reserve a frame slot for the application
 * @return -1 if error, slot index otherwise
 * Invoked from the application to get a frame
 * Fill in the data and request delivery
 * The data request needs the index i returned by this function as a parameter
 */
int knxLinkPoolAppLock(void);

/**
 * @brief Reserve a frame slot for the link level
 * The pool slot becomes available to the application after the reception of the frame
 * A new pool is reserved for another incoming frame
 */
int knxLinkPoolLinkLock(void);

/**
 * @brief Reserve a frame slot for the application
 * @param[in] i Slot index returned by knxLinkAppLock()
 * @return if error, reserve a slot otherwise
 */
void knxLinkPoolYieldLock(int i);


/**
 * @brief Reserve a frame slot for the link level
 * @param[in] i Slot index returned by knxLinkAppLock()
 * @return if error, reserve a slot otherwise
 */
void knxLinkPoolAppYieldLock(int i);

/**
 * @brief Free a previously reserved frame slot
 * @param[in] i Slot index returned by knxLinkFramePoolLock()
 * @param[in] flag Frame slot flag
 * @return Nothing
 * @remarks If i is not a valid slot index or the slot is not reserved, or the flag value does not match the slot flag value, 
 * this function does nothing.
 * If (flag <= KNX_LINK_POOL_FLAG_AVAILABLE) or (flag >= KNX_LINK_POOL_FLAG_INVALID)
 * the request is invalid.
 */
void knxLinkFramePoolUnlock(int i, knxLinkFramePoolUser_t flag);

/**
 * @brief Free a previously reserved frame slot
 * @param[in] i Slot index returned by knxLinkFramePoolLock()
 * @return Nothing
 * @remarks If i is not a valid slot index or the slot is not reserved,
 * this function does nothing.
 */
void knxLinkPoolAppUnLock(int i);

/**
 * @brief Get a pointer to the reserved frame slot
 * @param[in] i Slot index returned by knxLinkPoolLock()
 * @param[in] flag Frame slot flag
 * @return NULL if error, pointer to the i-th slot otherwise
 * @remarks If i is not a valid slot index or the slot is not reserved, or the flag value does not match the slot flag value, 
 * NULL is returned.
 * If (flag <= KNX_LINK_POOL_FLAG_AVAILABLE) or (flag >= KNX_LINK_POOL_FLAG_INVALID)
 * the request is invalid.
 */
knxLinkFrame_t *knxLinkFramePoolGet(int i, knxLinkFramePoolUser_t flag);

/**
 * @brief Get a pointer to the reserved frame slot
 * @param[in] i Slot index returned by knxLinkPoolAppLock()
 * @return NULL if error, pointer to the i-th slot otherwise
 * @remarks If i is not a valid slot index or the slot is not reserved,
 * NULL is returned.
 */
knxLinkFrame_t *knxLinkFramePoolAppGet(int i);

/**
 * @brief Get a pointer to the reserved frame slot
 * @param[in] i Slot index returned by knxLinkPoolLock()
 * Send : Reserve as many pool slots as messages to send
 * Reception of remote frame : i is dequeued in the pool, already reserved for the application
 */
knxLinkFrame_t *knxLinkPoolLinkGet(int i);

#endif /* KNX_LINK_FRAME_POOL_H_ */
