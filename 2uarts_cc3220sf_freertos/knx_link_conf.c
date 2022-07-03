/*
 * knx_link_conf.c
 */
#include <knx_link_conf.h>
#include "FreeRTOS.h"
#include "knx_link.h"
#include "knx_link_internal.h"


knxLinkHandle_t     *link;


void sendDataReq(int frame_index) {
    xQueueSend(link->knxLinkDataReq, &frame_index, portMAX_DELAY);
}


void recvDataCon(void) {
    xQueueReceive(link->knxLinkDataCon, &knxLinkDataParams.frame_index, portMAX_DELAY);
    xQueueReceive(link->knxLinkDataCon, &knxLinkDataParams.confirmation, portMAX_DELAY);
}


void recvDataInd(int frame_index) {
    xQueueReceive(link->knxLinkDataInd, &frame_index, portMAX_DELAY);
}
