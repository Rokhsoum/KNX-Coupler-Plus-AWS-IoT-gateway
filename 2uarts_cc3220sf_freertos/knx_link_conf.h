/*
 * knx_link_conf.h
 *
 *  Created on: 2 juil. 2022
 *      Author: Rokhaya Soumare
 */

#ifndef KNX_LINK_CONF_H_
#define KNX_LINK_CONF_H_

/**
 * @brief   Write to the knxLinkDataReq Queue
 */
void sendDataReq(int frame_index);


/**
 * @brief   Read from the knxLinkDataCon Queue
 */
void recvDataCon(void);


/**
 * @brief   Read from the knxLinkDataInd Queue
 */
void recvDataInd(int frame_index);


#endif /* KNX_LINK_CONF_H_ */
