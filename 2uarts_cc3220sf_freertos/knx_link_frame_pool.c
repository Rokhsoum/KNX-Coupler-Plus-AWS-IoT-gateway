/*
 * @file knx_link_frame_pool.c
 * @brief KNX library project
 */
#include "knx_link_frame_pool.h"
#include "knx_link_frame.h"
#include "knx_link_internal.h"
#include "knx_link.h"
#include <FreeRTOS.h>

/**
 * Tipo estructurado con todos los parámetros del pool
 */
typedef struct knxLinkFramePool_s {
    knxLinkFrame_t    frames[KNX_LINK_FRAME_POOL_SIZE];   /**< Array de telegramas de tamaño KNX_LINK_FRAME_POOL_SIZE */
    uint8_t           flags[KNX_LINK_FRAME_POOL_SIZE];    /**< Array de flags uint8_t de tamaño KNX_LINK_FRAME_POOL_SIZE */
    SemaphoreHandle_t mutex;                              /**< Mutex para garantizar acceso del pool */
    uint8_t           initialized;                        /**< Inicialización del pool de telegramas */
} knxLinkFramePool_t;


static knxLinkFramePool_t knxLinkFramePool = { .initialized = 0 };

/**
 * @brief Reserve a frame slot
 * @param[in] flag Ownership flag
 * @return -1 if error, slot index otherwise
 * The reserved frame slot is marked with the ownership flag; further operations
 * on the reserved frame must match this value:
 * eg. a slot locked with KNX_LINK_POOL_FLAG_APP can only be unlocked with this same flag value.
 * If (flag <= KNX_LINK_POOL_FLAG_AVAILABLE) or (flag >= KNX_LINK_POOL_FLAG_INVALID)
 * the request is invalid.
 */
static int _knxLinkFramePoolLock(knxLinkFramePoolUser_t flag);

/**
 * @brief Free a previously reserved frame slot
 * @param[in] i Slot index
 * @param[in] flag Ownership flag
 * @remarks If i is not a valid slot index or the slot is not reserved, or the ownership flag value does not match the slot flag value,
 * this function does nothing.
 * If (flag <= KNX_LINK_POOL_FLAG_AVAILABLE) or (flag >= KNX_LINK_POOL_FLAG_INVALID)
 * the request is invalid.
 */
static void _knxLinkFramePoolUnlock(int i, knxLinkFramePoolUser_t flag);

/**
 * @brief Transfer ownership of a frame slot
 * @param[in] i Slot index
 * @param[in] flag_from Current ownership flag
 * @param[in] flag_to New ownership flag
 */
static void _knxLinkFramePoolYieldLock(int i, knxLinkFramePoolUser_t flag_from, knxLinkFramePoolUser_t flag_to);

/**
 * @brief Get a pointer to the reserved frame slot
 * @param[in] i Slot index returned by knxLinkPoolLock()
 * @param[in] flag Ownership flag
 * @return NULL if error, pointer to the i-th slot otherwise
 * @remarks If i is not a valid slot index or the slot is not reserved, or the ownership flag does not match the slot flag value,
 * NULL is returned.
 * If (flag <= KNX_LINK_POOL_FLAG_AVAILABLE) or (flag >= KNX_LINK_POOL_FLAG_INVALID)
 * the request is invalid.
 */
static knxLinkFrame_t *_knxLinkFramePoolGet(int i, knxLinkFramePoolUser_t flag);


void knxLinkFramePoolInit(void) {
    int i;

    if (knxLinkFramePool.initialized) {
        // Already initialized. Avoid a 2nd, 3rd, etc. initialization
        return;
    }
    knxLinkFramePool.initialized = 0;
    knxLinkFramePool.mutex = xSemaphoreCreateMutex();
    if (knxLinkFramePool.mutex == NULL) {
        return;
    }
    for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
        knxLinkFramePool.flags[i] = KNX_LINK_FRAME_POOL_FLAG_AVAILABLE;
    }
    knxLinkFramePool.initialized = 1;
}


static int _knxLinkFramePoolLock(knxLinkFramePoolUser_t flag) {
    int i, res = -1;

    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
		return res;
	}

    if ((flag <= KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) || (flag >= KNX_LINK_FRAME_POOL_FLAG_INVALID)) {
        return res;
    }

    if (xSemaphoreTake(knxLinkFramePool.mutex, portMAX_DELAY) == pdFALSE) {
        return res;
    }

    for (i = 0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
        if (knxLinkFramePool.flags[i] == KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) {
            // Found an available slot, reserve it and return its index
            knxLinkFramePool.flags[i] = flag;
            res = i;
            break;
        }
    }
    xSemaphoreGive(knxLinkFramePool.mutex);
    return res;
}

int knxLinkFramePoolAppLock(void) {
    return _knxLinkFramePoolLock(KNX_LINK_FRAME_POOL_FLAG_APP);
}

int knxLinkFramePoolLinkLock(void) {
    return _knxLinkFramePoolLock(KNX_LINK_FRAME_POOL_FLAG_LINK);
}



static void _knxLinkFramePoolYieldLock(int i, knxLinkFramePoolUser_t flag_from, knxLinkFramePoolUser_t flag_to) {
    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
        return;
    }
    if ((flag_from <= KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) || (flag_from >= KNX_LINK_FRAME_POOL_FLAG_INVALID)) {
        return;
    }
    if ((flag_to <= KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) || (flag_to >= KNX_LINK_FRAME_POOL_FLAG_INVALID)) {
        return;
    }
    if (flag_from == flag_to) {
        return;
    }
    if ( (i < 0) || (i >= KNX_LINK_FRAME_POOL_SIZE) ) {
        return;
    }
    if (xSemaphoreTake(knxLinkFramePool.mutex, portMAX_DELAY) == pdFALSE) {
        return;
    }
    if (knxLinkFramePool.flags[i] == flag_from) {
        knxLinkFramePool.flags[i] = flag_to;
    }
    xSemaphoreGive(knxLinkFramePool.mutex);
}

void knxLinkFramePoolAppYieldLock(int i) {
    _knxLinkFramePoolYieldLock(i, KNX_LINK_FRAME_POOL_FLAG_APP, KNX_LINK_FRAME_POOL_FLAG_LINK);
}

void knxLinkFramePoolLinkYieldLock(int i) {
    _knxLinkFramePoolYieldLock(i, KNX_LINK_FRAME_POOL_FLAG_LINK, KNX_LINK_FRAME_POOL_FLAG_APP);
}



static void _knxLinkFramePoolUnlock(int i, knxLinkFramePoolUser_t flag) {
    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
        return;
    }
    if ((flag <= KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) || (flag >= KNX_LINK_FRAME_POOL_FLAG_INVALID)) {
        return;
    }
    if ( (i < 0) || (i >= KNX_LINK_FRAME_POOL_SIZE) ) {
        return;
    }
    if (xSemaphoreTake(knxLinkFramePool.mutex, portMAX_DELAY) == pdFALSE) {
        return;
    }
    if (knxLinkFramePool.flags[i] == flag) {
        knxLinkFramePool.flags[i] = KNX_LINK_FRAME_POOL_FLAG_AVAILABLE;
    }
    xSemaphoreGive(knxLinkFramePool.mutex);
}

void knxLinkFramePoolAppUnLock(int i) {
    _knxLinkFramePoolUnlock(i, KNX_LINK_FRAME_POOL_FLAG_APP);
}

void knxLinkFramePoolLinkUnLock(int i) {
    _knxLinkFramePoolUnlock(i, KNX_LINK_FRAME_POOL_FLAG_LINK);
}



static knxLinkFrame_t *_knxLinkFramePoolGet(int i, knxLinkFramePoolUser_t flag) {
	knxLinkFrame_t *res = NULL;

	if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
		return res;
	}
	if ((flag <= KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) || (flag >= KNX_LINK_FRAME_POOL_FLAG_INVALID)) {
		return res;
	}
    if ( (i < 0) || (i >= KNX_LINK_FRAME_POOL_SIZE) ) {
        return res;
    }
    if (xSemaphoreTake(knxLinkFramePool.mutex, portMAX_DELAY) == pdFALSE) {
        return res;
    }
    if (knxLinkFramePool.flags[i] == flag) {
        res = &knxLinkFramePool.frames[i];
    }
    xSemaphoreGive(knxLinkFramePool.mutex);
    return res;
}

knxLinkFrame_t *knxLinkFramePoolAppGet(int i) {
    return _knxLinkFramePoolGet(i, KNX_LINK_FRAME_POOL_FLAG_APP);
}

knxLinkFrame_t *knxLinkFramePoolLinkGet(int i) {
    return _knxLinkFramePoolGet(i, KNX_LINK_FRAME_POOL_FLAG_LINK);
}
