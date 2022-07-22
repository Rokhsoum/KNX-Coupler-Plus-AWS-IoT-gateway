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


void knxLinkFramePoolInit(void) {
    int i;

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

int knxLinkFramePoolLock(knxLinkFramePoolUser_t flag) {
    int i, res = -1;

    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
		return res;
	}
    else {
        if ((flag <= KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) || (flag >= KNX_LINK_FRAME_POOL_FLAG_INVALID)) {
            return res;
        }

        if (xSemaphoreTake(knxLinkFramePool.mutex, portMAX_DELAY) == pdFALSE) {
            return res;
        }

        for (i = 0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
            if (knxLinkFramePool.flags[i] == KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) {
                knxLinkFramePool.flags[i] = flag;
                res = i;
                break;
            }
        }
        xSemaphoreGive(knxLinkFramePool.mutex);
        return res;
    }
}


int knxLinkPoolAppLock(void) {
    int i, res = -1;

    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
        return res;
    }
    else {
        for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
            if(knxLinkFramePool.flags[i] == KNX_LINK_FRAME_POOL_FLAG_APP) {
                res = i;
            }
            else {
                res = -1;
            }
        }
        return res;
    }
}


int knxLinkPoolLinkLock(void) {
    int i, res = -1;

    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
        return res;
    }
    else {
        for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
            if(knxLinkFramePool.flags[i] == KNX_LINK_FRAME_POOL_FLAG_UPLINK) {
                res = i;
            }
            else {
                res = -1;
            }
        }
        return res;
    }
}

void knxLinkPoolYieldLock(int i) {

    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
        return;
    }
    else {
        knxLinkFramePool.flags[i] = KNX_LINK_FRAME_POOL_FLAG_UPLINK;


        for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
            if (knxLinkFramePool.flags[i] == KNX_LINK_FRAME_POOL_FLAG_UPLINK) {
                knxLinkFramePool.flags[i] = KNX_LINK_FRAME_POOL_FLAG_APP;
            }
            else {
                return;
            }
        }
    }
}


void knxLinkPoolLinkYieldLock(int i) {

    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
        return;
    }
    else {
        knxLinkHandle_t *link = 0;
        knxLinkDataCon_t con;

        if(knxLinkFramePool.frames[i].sa != link->ia) {
            xQueueSend(link->knxLinkDataInd, &i, portMAX_DELAY);
        }
        else {
            con.frame_index = i;
            xQueueSend(link->knxLinkDataCon, &con, portMAX_DELAY);
        }
    }
}


void knxLinkPoolAppYieldLock(int i) {

    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
        return;
    }
    else {
        knxLinkFramePool.flags[i] = KNX_LINK_FRAME_POOL_FLAG_APP;

        for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
            if (knxLinkFramePool.flags[i] == KNX_LINK_FRAME_POOL_FLAG_APP) {
                knxLinkFramePool.flags[i] = KNX_LINK_FRAME_POOL_FLAG_UPLINK;
            }
            else {
                return;
            }
        }
    }
}


void knxLinkFramePoolUnlock(int i, knxLinkFramePoolUser_t flag) {
	if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
		return;
	}
	else {
	    if ((flag <= KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) || (flag >= KNX_LINK_FRAME_POOL_FLAG_INVALID)) {
	        return;
	    }

	    if ( (i >= 0) && (i < KNX_LINK_FRAME_POOL_SIZE) ) {
	        if (xSemaphoreTake(knxLinkFramePool.mutex, portMAX_DELAY) == pdFALSE) {
	            return;
	        }

	        if (knxLinkFramePool.flags[i] == flag) {
	            knxLinkFramePool.flags[i] = KNX_LINK_FRAME_POOL_FLAG_AVAILABLE;
	        }

	        xSemaphoreGive(knxLinkFramePool.mutex);
	    }
	}
}

void knxLinkPoolLinkUnLock(int i) {

    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
        return;
    }
    else {
        knxLinkFramePool.flags[i] = KNX_LINK_FRAME_POOL_FLAG_UPLINK? KNX_LINK_FRAME_POOL_FLAG_UPLINK : KNX_LINK_FRAME_POOL_FLAG_DOWNLINK;

        for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
            if (knxLinkFramePool.flags[i] == KNX_LINK_FRAME_POOL_FLAG_UPLINK || KNX_LINK_FRAME_POOL_FLAG_DOWNLINK) {
                knxLinkFramePool.flags[i] = KNX_LINK_FRAME_POOL_FLAG_AVAILABLE;
            }
            else {
                return;
            }

            if ( (i > KNX_LINK_FRAME_POOL_SIZE) || (i < 0) ) {
                return;
            }
        }
    }
}

void knxLinkPoolAppUnLock(int i) {

    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
        return;
    }
    else {
        knxLinkFramePool.flags[i] = KNX_LINK_FRAME_POOL_FLAG_APP;

        for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
            if (knxLinkFramePool.flags[i] == KNX_LINK_FRAME_POOL_FLAG_APP) {
                knxLinkFramePool.flags[i] = KNX_LINK_FRAME_POOL_FLAG_AVAILABLE;
            }
            else {
                return;
            }

            if ( (i > KNX_LINK_FRAME_POOL_SIZE) || (i < 0) ) {
                return;
            }
        }
    }
}

knxLinkFrame_t *knxLinkFramePoolGet(int i, knxLinkFramePoolUser_t flag) {
	knxLinkFrame_t *res = NULL;
	if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
		return res;
	}

	if ((flag <= KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) || (flag >= KNX_LINK_FRAME_POOL_FLAG_INVALID)) {
		return res;
	}

	else {
	    if ( (i >= 0) && (i < KNX_LINK_FRAME_POOL_SIZE) ) {
	        if (xSemaphoreTake(knxLinkFramePool.mutex, portMAX_DELAY) == pdFALSE) {
	            return res;
	        }
	        if (knxLinkFramePool.flags[i] == flag) {
	            res = &knxLinkFramePool.frames[i];
	        }
	        xSemaphoreGive(knxLinkFramePool.mutex);
	    }
	    return res;
	}
}


knxLinkFrame_t *knxLinkFramePoolAppGet(int i) {
    knxLinkFrame_t *res = NULL;

    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
        return res;
    }
    else {
        if ( (i > KNX_LINK_FRAME_POOL_SIZE) || (i < 0) ) {
            return res;
        }

        if (knxLinkFramePool.flags[i] != KNX_LINK_FRAME_POOL_FLAG_APP) {
            return res;
        }
        return res;
    }
}


knxLinkFrame_t *knxLinkPoolLinkGet(int i) {
    knxLinkFrame_t *res = NULL;

    if (!knxLinkFramePool.initialized || (knxLinkFramePool.mutex == NULL)) {
        return res;
    }

    if ( (i > KNX_LINK_FRAME_POOL_SIZE) || (i < 0) ) {
        return res;
    }

    if (knxLinkFramePool.flags[i] != KNX_LINK_FRAME_POOL_FLAG_UPLINK) {
        return res;
    }
    return res;
}
