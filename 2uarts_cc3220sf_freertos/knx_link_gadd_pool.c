/*
 * @file knx_link_gadd_pool.c
 * @brief KNX library project
 */

#include <stdlib.h>
#include <stdint.h>
#include "knx_link_gadd_pool.h"

#define GA_POOL_SIZE    400

static uint16_t ga_pool[GA_POOL_SIZE];
static unsigned int ga_pool_used = 0;
static unsigned int ga_pool_avail = GA_POOL_SIZE;

int ga_set_init(ga_set_type *gas, unsigned int size) {
    if ((gas == NULL) || (size > ga_pool_avail)) {
        return 0;
    }

    gas->ga_set = &ga_pool[ga_pool_used];
    gas->size = size;
    gas->used = 0;
    ga_pool_used += size;
    ga_pool_avail -= size;
    return 1;
}

int ga_set_add(ga_set_type *gas, uint16_t ga) {
    if (gas == NULL) {
        return 0;
    }

    if (gas->ga_set == NULL) {
        return 0;
    }

    if (ga_set_in(gas, ga) == 1) {
        return 1;
    }

    if (gas->used >= gas->size) {
        return 0;
    }

    gas->ga_set[gas->used++] = ga;
    return 1;
}

int ga_set_in(ga_set_type *gas, uint16_t ga) {
    unsigned int i;

    if (gas == NULL) {
        return 0;
    }

    if (gas->ga_set == NULL) {
        return 0;
    }

    for (i=0; i < gas->used; i++) {
        if (gas->ga_set[i] == ga) {
            return 1;
        }
    }
    return 0;
}
