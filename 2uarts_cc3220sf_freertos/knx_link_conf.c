/*
 * knx_link_conf.c
 */
#include <knx_link_conf.h>
#include "FreeRTOS.h"
#include "knx_link.h"
#include "knx_link_internal.h"


knxLinkDataCon_t    conf;


void sendDataReq(knxLinkHandle_t *link, int *frame_index) {
    xQueueSend(link->knxLinkDataReq, &frame_index, portMAX_DELAY);
}


void recvDataCon(knxLinkHandle_t *link) {
    xQueueReceive(link->knxLinkDataCon, &conf, portMAX_DELAY);
}

void recvDataInd(knxLinkHandle_t *link, int *frame_index) {
    xQueueReceive(link->knxLinkDataInd, &frame_index, portMAX_DELAY);
}
