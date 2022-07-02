/*
 * @file knx_link_state.c
 * @brief KNX library project
 */
#include "knx_link_state.h"
#include "knx_link_internal.h"

void knxLinkInitState(knxLinkHandle_t *link) {
    link->state = KNX_LINK_ILLEGAL_STATE;
}

void knxLinkSetState(knxLinkHandle_t *link, int newState) {
    /* Only transitions stated in the KNX standard are allowed */
    switch (link->state) {

    case KNX_LINK_ILLEGAL_STATE:    // Valid transition: KNX_LINK_INIT_STATE
        if (newState == KNX_LINK_INIT_STATE) {
            link->state = newState;
        }
        break;

    case KNX_LINK_INIT_STATE:       // Valid transitions: KNX_LINK_NORMAL_STATE, KNX_LINK_STOP_STATE
        if ( (newState == KNX_LINK_NORMAL_STATE) || (newState == KNX_LINK_STOP_STATE) ) {
            link->state = newState;
        }
        break;

    case KNX_LINK_NORMAL_STATE:     // Valid transitions: KNX_LINK_STOP_STATE, KNX_LINK_MONITOR_STATE
        if ( (newState == KNX_LINK_STOP_STATE) || (newState == KNX_LINK_MONITOR_STATE) ) {
            link->state = newState;
        }
        break;

    case KNX_LINK_MONITOR_STATE:    // Valid transition: KNX_LINK_NORMAL_STATE
        if (newState == KNX_LINK_NORMAL_STATE) {
            link->state = newState;
        }
        break;

    default: // KNX_LINK_STOP_STATE, no valid transition. To exit from KNX_LINK_STOP_STATE must initialize KNX link
        ;
    }
}

int knxLinkGetState(knxLinkHandle_t *link) {
    return link->state;
}
