/*
 * knx_network.c
 */
#include "FreeRTOS.h"
#include "knx_network.h"
#include "knx_link.h"
#include "knx_link_frame.h"
#include "knx_link_frame_pool.h"
#include "knx_link_internal.h"


knxLinkHandle_t     *link;


void sendDataReq(int frame_index) {
    xQueueSend(link->knxLinkDataReq, &frame_index, portMAX_DELAY);
}


void RecvDataConf(void) {
    xQueueReceive(link->knxLinkDataCon, &knxLinkDataParams.frame_index, portMAX_DELAY);
    xQueueReceive(link->knxLinkDataCon, &knxLinkDataParams.confirmation, portMAX_DELAY);
}


void RecvDataInd(int frame_index) {
    xQueueReceive(link->knxLinkDataInd, &frame_index, portMAX_DELAY);
}
