/*
 * @file knx_link_encoding.c
 * @brief KNX library project
 */

#include "knx_link_encoding.h"



uint8_t knxLinkEncodeCtrl(uint8_t frame_type, uint8_t rep, uint8_t prio) {
    return ( KNX_FRAME_CTRL_BASE |
            ((frame_type << KNX_FRAME_CTRL_FT_POS) & KNX_FRAME_CTRL_FT_MASK) |
            ((rep << KNX_FRAME_CTRL_REP_POS) & KNX_FRAME_CTRL_REP_MASK) |
            ((prio << KNX_FRAME_CTRL_PRIO_POS) & KNX_FRAME_CTRL_PRIO_MASK) );
}

uint8_t knxLinkDecodeCtrlFt(uint8_t ctrl) {
    return (ctrl & KNX_FRAME_CTRL_FT_MASK) >> KNX_FRAME_CTRL_FT_POS;
}

uint8_t knxLinkDecodeCtrlRep(uint8_t ctrl) {
    return (ctrl & KNX_FRAME_CTRL_REP_MASK) >> KNX_FRAME_CTRL_REP_POS;
}

uint8_t knxLinkDecodeCtrlPrio(uint8_t ctrl) {
    return (ctrl & KNX_FRAME_CTRL_PRIO_MASK) >> KNX_FRAME_CTRL_PRIO_POS;
}

uint16_t knxLinkEncodeAddress(uint8_t high, uint8_t low) {
    return ((uint16_t)high << 8) | (uint16_t)low;
}

uint8_t knxLinkDecodeAddressHigh(uint16_t address) {
    return (uint8_t)((address >> 8) & 0x00FF);
}

uint8_t knxLinkDecodeAddressLow(uint16_t address) {
    return (uint8_t)(address & 0x00FF);
}

uint8_t knxLinkEncodeAtLsduLg(uint8_t at, uint8_t hop_count, uint8_t lg) {
    return ( ((at << KNX_FRAME_ATLSDULG_AT_POS) & KNX_FRAME_ATLSDULG_AT_MASK) |
             ((hop_count << KNX_FRAME_ATLSDULG_HOPCNT_POS) & KNX_FRAME_ATLSDULG_HOPCNT_MASK) |
             ((lg << KNX_FRAME_ATLSDULG_LG_POS) & KNX_FRAME_ATLSDULG_LG_MASK) );
}

uint8_t knxLinkDecodeAtLsduLgAt(uint8_t atlsdulg) {
    return (atlsdulg & KNX_FRAME_ATLSDULG_AT_MASK) >> KNX_FRAME_ATLSDULG_AT_POS;
}

uint8_t knxLinkDecodeAtLsduLgHopCount(uint8_t atlsdulg) {
    return (atlsdulg & KNX_FRAME_ATLSDULG_HOPCNT_MASK) >> KNX_FRAME_ATLSDULG_HOPCNT_POS;
}

uint8_t knxLinkDecodeAtLsduLgLg(uint8_t atlsdulg) {
    return (atlsdulg & KNX_FRAME_ATLSDULG_LG_MASK) >> KNX_FRAME_ATLSDULG_LG_POS;
}

uint8_t knxLinkEncodeCtrle(uint8_t at, uint8_t hop_count, uint8_t ext_ff) {
    return ( ((at << KNX_FRAME_CTRLE_AT_POS) & KNX_FRAME_CTRLE_AT_MASK) |
             ((hop_count << KNX_FRAME_CTRLE_HOPCNT_POS) & KNX_FRAME_CTRLE_HOPCNT_MASK) |
             ((ext_ff << KNX_FRAME_CTRLE_EXTFF_POS) & KNX_FRAME_CTRLE_EXTFF_MASK) );
}

uint8_t knxLinkDecodeCtrleAt(uint8_t ctrle) {
    return (ctrle & KNX_FRAME_CTRLE_AT_MASK) >> KNX_FRAME_CTRLE_AT_POS;
}

uint8_t knxLinkDecodeCtrleHopCount(uint8_t ctrle) {
    return (ctrle & KNX_FRAME_CTRLE_HOPCNT_MASK) >> KNX_FRAME_CTRLE_HOPCNT_POS;
}

uint8_t knxLinkDecodeCtrleExtFF(uint8_t ctrle) {
    return (ctrle & KNX_FRAME_CTRLE_EXTFF_MASK) >> KNX_FRAME_CTRLE_EXTFF_POS;
}
