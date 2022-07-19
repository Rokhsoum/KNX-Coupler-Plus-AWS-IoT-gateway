/*
 * knx_commissioning_data.c
 *
 *  Created on: 14 juil. 2022
 *      Author: Rokhaya Soumare
 */

#include <stdint.h>
#include "knx_commissioning_data.h"


const object_commissioning_t objects[] = {
    {0, 1, 0, 2},
    {1, 1, 2, 2},
    {2, 0, 4, 2},
    {3, 0, 6, 3}
    //{object_id = 0, outgoing = 1, gas_start_index = 0, gas_length = 2},
    //{object_id = 1, outgoing = 1, gas_start_index = 2, gas_length = 2},
    //{object_id = 2, outgoing = 0, gas_start_index = 4, gas_length = 2},
    //{object_id = 3, outgoing = 0, gas_start_index = 6, gas_length = 3}
};


const uint16_t ga_commissioning[] = {
    /* Boton SW2 genera dos mensajes: 5/1/x y 5/1/100, siendo x = identificador único del proyecto */
    KNX_BUILD_GA_ADDRESS(5,1,KNX_PERSONAL_ID),          // 5/1/x
    KNX_BUILD_GA_ADDRESS(5,1,100),                      // 5/1/100
    /* Boton SW3 genera dos mensajes: 5/2/x y 5/2/100, siendo x = identificador único del proyecto */
    KNX_BUILD_GA_ADDRESS(5,2,KNX_PERSONAL_ID),          // 5/2/x
    KNX_BUILD_GA_ADDRESS(5,2,100),                      // 5/2/100
    /* Led verde se suscribe a dos mensajes: 5/1/x y 5/1/100, siendo x = identificador único del proyecto */
    KNX_BUILD_GA_ADDRESS(5,1,KNX_PERSONAL_ID),          // 5/1/x
    KNX_BUILD_GA_ADDRESS(5,1,100),                      // 5/1/100
    /* Led amarillo se suscribe a tres mensajes: 5/1/x, 5/2/x y 5/2/100, siendo x = identificador único del proyecto */
    KNX_BUILD_GA_ADDRESS(5,1,100),                      // 5/1/100
    KNX_BUILD_GA_ADDRESS(5,2,KNX_PERSONAL_ID),          // 5/2/x
    KNX_BUILD_GA_ADDRESS(5,2,100),                      // 5/2/100
};


const object_commissioning_t *getCommissioningObjects(void) {
    return objects;
}

int getCommissioningObjectsNum(void) {
    return ARRAY_SIZE(objects);
}

const uint16_t *getCommissioningGAs(void) {
    return ga_commissioning;
}

int getCommissioningGAsNum(void) {
    return ARRAY_SIZE(ga_commissioning);
}


