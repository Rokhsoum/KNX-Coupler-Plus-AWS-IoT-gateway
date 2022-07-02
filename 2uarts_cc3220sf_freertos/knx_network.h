/*
 * knx_network.h
 *
 *  Created on: 2 juil. 2022
 *      Author: Rokhaya Soumare
 */

#ifndef KNX_NETWORK_H_
#define KNX_NETWORK_H_


void sendDataReq(int frame_index);

void recvDataCon(void);

void recvDataInd(int frame_index);


#endif /* KNX_NETWORK_H_ */
