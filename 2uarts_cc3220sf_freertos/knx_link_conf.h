/*
 * knx_link_conf.h
 *
 *  Created on: 2 juil. 2022
 *      Author: Rokhaya Soumare
 */

#ifndef KNX_LINK_CONF_H_
#define KNX_LINK_CONF_H_

struct knxLinkHandle_s;

/**
 * @brief   Write to the knxLinkDataReq Queue
 */
void sendDataReq(struct knxLinkHandle_s *link, int *frame_index);


/**
 * @brief   Read from the knxLinkDataCon Queue
 */
void recvDataCon(struct knxLinkHandle_s *link);


/**
 * @brief   Read from the knxLinkDataInd Queue
 */
void recvDataInd(struct knxLinkHandle_s *link, int *frame_index);


#endif /* KNX_LINK_CONF_H_ */
