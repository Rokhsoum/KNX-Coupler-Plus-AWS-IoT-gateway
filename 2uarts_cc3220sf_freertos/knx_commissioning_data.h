/* @file knx_commissioning_data.h */
#ifndef KNX_COMMISSIONING_DATA_H
#define KNX_COMMISSIONING_DATA_H


#include "knx_link.h"
#include "knx_link_gadd_pool.h"

#define KNX_PERSONAL_ID						1

#define MY_IA_ADDRESS						KNX_BUILD_IA_ADDRESS(1,1,KNX_PERSONAL_ID)			// 1.1.x

#define ARRAY_SIZE(arr)						(sizeof(arr)/sizeof(arr[0]))

#define KNX_BUILD_IA_ADDRESS(a,b,c)         0

#define KNX_BUILD_GA_ADDRESS(a,b,c)         0


/* Boton SW2 genera dos mensajes: 5/1/x y 5/1/100, siendo x = identificador único del proyecto */ 
const uint16_t GA_SET_BOTON_0[] = {
	KNX_BUILD_GA_ADDRESS(5,1,KNX_PERSONAL_ID),			// 5/1/x
	KNX_BUILD_GA_ADDRESS(5,1,100)						// 5/1/100
};

/* Boton SW3 genera dos mensajes: 5/2/x y 5/2/100, siendo x = identificador único del proyecto */ 
const uint16_t GA_SET_BOTON_1[] = {
	KNX_BUILD_GA_ADDRESS(5,2,KNX_PERSONAL_ID), 			// 5/2/x
	KNX_BUILD_GA_ADDRESS(5,2,100)						// 5/2/100
};

/* Led verde se suscribe a dos mensajes: 5/1/x y 5/1/100, siendo x = identificador único del proyecto */ 
const uint16_t GA_SET_LED_0[] = {
	KNX_BUILD_GA_ADDRESS(5,1,KNX_PERSONAL_ID),			// 5/1/x
	KNX_BUILD_GA_ADDRESS(5,1,100)						// 5/1/100
};

/* Led amarillo se suscribe a tres mensajes: 5/1/x, 5/2/x y 5/2/100, siendo x = identificador único del proyecto */ 
const uint16_t GA_SET_LED_1[] = {
	KNX_BUILD_GA_ADDRESS(5,1,100),						// 5/1/100
	KNX_BUILD_GA_ADDRESS(5,2,KNX_PERSONAL_ID), 			// 5/2/x
	KNX_BUILD_GA_ADDRESS(5,2,100)						// 5/2/100
};

#endif	/* KNX_COMMISSIONING_DATA_H */
