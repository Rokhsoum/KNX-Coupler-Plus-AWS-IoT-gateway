/*
 * @file knx_link_frame.c
 * @brief KNX library project
 */
#include <stdint.h>
#include <stdio.h>
#include "knx_link.h"
#include "knx_link_frame.h"
#include "knx_link_encoding.h"

static int _knxLinkEncodeAsStdFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize);
static int _knxLinkEncodeAsExtFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize);
static int _knxLinkDecodeAsStdFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize);
static int _knxLinkDecodeAsExtFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize);

uint8_t knxLinkCalcCHK(uint8_t *buffer, unsigned int bufsize) {

    uint8_t CHK;
    int i;
    for (i = 0 ; i < bufsize ; i++) {
        CHK ^= buffer[i];
    }
    CHK ^= 0xFF;

    return CHK;
}

int knxLinkVerifyCHK(uint8_t *buffer, unsigned int bufsize, uint8_t chk) {

    buffer[bufsize] = chk;

    if (chk = knxLinkCalcCHK(buffer, bufsize)) {
        return 1;
    }
    else {
        return 0;
    }
}

static int _knxLinkEncodeAsStdFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize) {
    int res = 0;
	
    /**
     * @TODO
     * Codificar frame como trama est谩ndar en buffer
     * Retornar n煤mero de bytes de buffer utilizados, o 0 si error
     */
	// Resto del c贸digo ...

    uint8_t frame_type = 0, rep = 0, prio = 0;

    knxLinkEncodeCtrl(frame_type, rep, prio);

    uint8_t highSA = 0, lowSA = 0;

    knxLinkEncodeAddress(highSA, lowSA);

    uint8_t highDA = 0, lowDA = 0;

    knxLinkEncodeAddress(highDA, lowDA);

    uint8_t at = 0, hop_count = 0, lg =0;

    knxLinkEncodeAtLsduLg(at, hop_count, lg);

    res = sizeof(_knxLinkEncodeAsStdFrame);
    if (res != sizeof(_knxLinkEncodeAsStdFrame)) {
        return 0;
    }
    else {
        return sizeof(_knxLinkEncodeAsStdFrame);
    }
}

static int _knxLinkEncodeAsExtFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize) {
    int res = 0;
	
    /**
     * @TODO
     * Codificar frame como trama extendida en buffer
     * Retornar n煤mero de bytes de buffer utilizados, o 0 si error
     */
	// Resto del c贸digo ...

    uint8_t frame_type = 0, rep = 0, prio = 0;

    knxLinkEncodeCtrl(frame_type, rep, prio);

    uint8_t at = 0, hop_count = 0, ext_ff = 0;

    knxLinkEncodeCtrle(at, hop_count, ext_ff);

    uint8_t highSA = 0, lowSA = 0;

    knxLinkEncodeAddress(highSA, lowSA);

    uint8_t highDA = 0, lowDA = 0;

    knxLinkEncodeAddress(highDA, lowDA);

    uint8_t lg = 0;

    knxLinkEncodeAtLsduLg(at, hop_count, lg); //v閞ifier

    res = sizeof(_knxLinkEncodeAsExtFrame);
    if (res != sizeof(_knxLinkEncodeAsExtFrame)) {
        return 0;
    }
    else {
        return sizeof(_knxLinkEncodeAsExtFrame);
    }
}

int knxLinkEncodeFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize) {
    int res;

    res = _knxLinkEncodeAsStdFrame(frame, buffer, bufsize);
    if (res == 0) {
        res = _knxLinkEncodeAsExtFrame(frame, buffer, bufsize);
    }
    return res;
}


static int _knxLinkDecodeAsStdFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize) {
    int res = 0;
	
    /**
     * @TODO
     * Decodificar buffer como trama est谩ndar en frame
     * Retornar 0 si error, 1 si ok
     */
	// Resto del c贸digo ...

    uint8_t ctrl = 0;

    knxLinkDecodeCtrlFt(ctrl);

    knxLinkDecodeCtrlRep(ctrl);

    knxLinkDecodeCtrlPrio(ctrl);

    uint16_t addressSA = 0;

    knxLinkDecodeAddressHigh(addressSA);

    knxLinkDecodeAddressLow(addressSA);

    uint16_t addressDA = 0;

    knxLinkDecodeAddressHigh(addressDA);

    knxLinkDecodeAddressLow(addressDA);

    uint8_t atlsdulg = 0;

    knxLinkDecodeAtLsduLgAt(atlsdulg);

    knxLinkDecodeAtLsduLgHopCount(atlsdulg);

    knxLinkDecodeAtLsduLgLg(atlsdulg);

    res =  _knxLinkDecodeAsStdFrame(frame, buffer, bufsize);
    if (res == _knxLinkDecodeAsExtFrame(frame, buffer, bufsize)) {
        return 0;
    }
    else {
        return 1;
    }
}

static int _knxLinkDecodeAsExtFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize) {
    int res = 0;
	
    /**
     * @TODO
     * Decodificar buffer como trama extendida en frame
     * Retornar 0 si error, 1 si ok
     */
	// Resto del c贸digo ...

    uint8_t ctrl =0;

    knxLinkDecodeCtrlFt(ctrl);

    knxLinkDecodeCtrlRep(ctrl);

    knxLinkDecodeCtrlPrio(ctrl);

    uint16_t addressSA = 0;

    knxLinkDecodeAddressHigh(addressSA);

    knxLinkDecodeAddressLow(addressSA);

    uint16_t addressDA = 0;

    knxLinkDecodeAddressHigh(addressDA);

    knxLinkDecodeAddressLow(addressDA);

    uint8_t atlsdulg = 0;

    knxLinkDecodeAtLsduLgLg(atlsdulg);

    uint8_t ctrle = 0;

    knxLinkDecodeCtrleAt(ctrle);

    knxLinkDecodeCtrleHopCount(ctrle);

    knxLinkDecodeCtrleExtFF(ctrle);

    res =  _knxLinkDecodeAsStdFrame(frame, buffer, bufsize);
    if (res == _knxLinkDecodeAsExtFrame(frame, buffer, bufsize)) {
        return 0;
    }
    else {
        return 1;
    }
}

int knxLinkDecodeFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize) {
    int res;

    res = _knxLinkDecodeAsStdFrame(frame, buffer, bufsize);
    if (res == 0) {
        res = _knxLinkDecodeAsExtFrame(frame, buffer, bufsize);
    }
    return res;
}
