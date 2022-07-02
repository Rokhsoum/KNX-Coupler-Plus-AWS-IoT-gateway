/*
 * @file knx_link_frame_pool.c
 * @brief KNX library project
 */
#include "knx_link_frame_pool.h"
#include "knx_link_frame.h"
#include "knx_link_internal.h"
#include "knx_link.h"
#include <FreeRTOS.h>
#include <semphr.h>

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

	if (!knxLinkFramePool.initialized) {
		return res;
	}

	if ((flag <= KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) || (flag >= KNX_LINK_FRAME_POOL_FLAG_INVALID)) {
		return res;
	}

    if (xSemaphoreTake(knxLinkFramePool.mutex, portMAX_DELAY) == pdFALSE) {
		return res;
	}

    for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
        if (knxLinkFramePool.flags[i] == KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) {
            knxLinkFramePool.flags[i] = flag;
            res = i;
            break;
        }
    }
    xSemaphoreGive(knxLinkFramePool.mutex);
    return res;
}

int knxLinkPoolAppLock(void) {
    int i, res = -1;

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

int knxLinkPoolLinkLock(void) {
    int i, res;

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

void knxLinkPoolYieldLock(int i) {
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

#if 0
void knxLinkPoolLinkYieldLock(int i) {
    int len = KNX_LINK_STD_FRAME_MAX;
    char frame[len];
    uint8_t confirmation;

    for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {

        if (knxLinkParams.rxiaframe != knxLinkParams.ia) {
        UART_read(knxLinkParams.uartKNX, &frame[i], 1);

        UART_write(knxLinkParams.uartKNX, &frame[i], 1);

        xQueueSend(knxLinkDataInd, &i, portMAX_DELAY);
        }
    }
    else
    {
        UART_read(knxLinkParams.uartKNX, &confirmation, 1);
        for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
            UART_write(uartKNX, &frame[i], 1);

            xQueueSend(knxLinkDataCon, &i, portMAX_DELAY);
            xQueueSend(knxLinkDataCon, &confirmation, portMAX_DELAY);
        }
    }

}
#endif


void knxLinkPoolAppYieldLock(int i) {
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


void knxLinkFramePoolUnlock(int i, knxLinkFramePoolUser_t flag) {
	if (!knxLinkFramePool.initialized) {
		return;
	}

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

void knxLinkPoolAppUnLock(int i) {
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

knxLinkFrame_t *knxLinkFramePoolGet(int i, knxLinkFramePoolUser_t flag) {
	knxLinkFrame_t *res = NULL;
	if (!knxLinkFramePool.initialized) {
		return res;
	}
	if ((flag <= KNX_LINK_FRAME_POOL_FLAG_AVAILABLE) || (flag >= KNX_LINK_FRAME_POOL_FLAG_INVALID)) {
		return res;
	}
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


knxLinkFrame_t *knxLinkFramePoolAppGet(int i) {
    knxLinkFrame_t *res = NULL;

    if ( (i > KNX_LINK_FRAME_POOL_SIZE) || (i < 0) ) {
        return res;
    }

    if (knxLinkFramePool.flags[i] != KNX_LINK_FRAME_POOL_FLAG_APP) {
        return res;
    }
    return res;
}


knxLinkFrame_t *knxLinkPoolLinkGet(int i) {
    knxLinkFrame_t *res = NULL;

    if ( (i > KNX_LINK_FRAME_POOL_SIZE) || (i < 0) ) {
        return res;
    }

    if (knxLinkFramePool.flags[i] != KNX_LINK_FRAME_POOL_FLAG_UPLINK) {
        return res;
    }
    return res;
}

#if 0
    int i;

    for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
        knxLinkFramePool.flags[i] = KNX_LINK_FRAME_POOL_SLOT_LOCKED_APP;
    }

    for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
    knxLinkFramePool.flags[i] = frames[i];
    }

    for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
        UART_write(uart_t uart, const uint8_t *frame[i], size_t len);
    }

    for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
        UART_read(uart_t uart, &confirmation, uint32_t length, uint32_t timeOutMs);
    }

    for (i=0; i < KNX_LINK_FRAME_POOL_SIZE; i++) {
        knxLinkFramePool.flags[i] == KNX_LINK_FRAME_POOL_SLOT_LOCKED_AVAILABLE);
    }

    if (rxiaframe != ia) {
        int frame_index;
        xQueueReceive(knxDataInd, &frame_index, xTicksToWait);
        knxLinkPoolLinkYieldLock(i);

        knxLinkPoolAppUnLock(i);
    }
#endif

