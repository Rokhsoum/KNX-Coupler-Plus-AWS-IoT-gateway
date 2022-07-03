/*
 * knx_link_conf.h
 *
 *  Created on: 2 juil. 2022
 *      Author: Rokhaya Soumare
 */

#ifndef KNX_LINK_CONF_H_
#define KNX_LINK_CONF_H_


void sendDataReq(int frame_index);

void recvDataCon(void);

void recvDataInd(int frame_index);


#endif /* KNX_LINK_CONF_H_ */
