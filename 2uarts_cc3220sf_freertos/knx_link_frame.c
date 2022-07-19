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

int knxLinkVerifyCHK(uint8_t *buffer, unsigned int bufsize, uint8_t CHK) {

    buffer[bufsize] = CHK;

    if (CHK = knxLinkCalcCHK(buffer, bufsize)) {
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
     * Codificar frame como trama estándar en buffer
     * Retornar número de bytes de buffer utilizados, o 0 si error
     */
	// Resto del código ...

    uint8_t FT = 0;
    buffer[0] = knxLinkEncodeCtrl(FT, frame->rep, frame->prio);

    uint8_t saH, saL;
    saH = knxLinkDecodeAddressHigh(frame->sa);
    saL = knxLinkDecodeAddressLow(frame->sa);
    buffer[1] = knxLinkEncodeAddress(saH, saL);

    uint8_t daH, daL;
    daH = knxLinkDecodeAddressHigh(frame->da);
    daL = knxLinkDecodeAddressLow(frame->da);
    buffer[3] = knxLinkEncodeAddress(daH, daL);

    buffer[5] = knxLinkEncodeAtLsduLg(frame->at, frame->hop_count, frame->length);


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
     * Retornar número de bytes de buffer utilizados, o 0 si error
     */
	// Resto del código ...

    uint8_t FT = 1;
    buffer[0] = knxLinkEncodeCtrl(FT, frame->rep, frame->prio);

    buffer[1] = knxLinkEncodeCtrle(frame->at, frame->hop_count, frame->ext_ff);

    uint8_t saH, saL;
    saH = knxLinkDecodeAddressHigh(frame->sa);
    saL = knxLinkDecodeAddressLow(frame->sa);
    buffer[2] = knxLinkEncodeAddress(saH, saL);

    uint8_t daH, daL;
    daH = knxLinkDecodeAddressHigh(frame->da);
    daL = knxLinkDecodeAddressLow(frame->da);
    buffer[4] = knxLinkEncodeAddress(daH, daL);

    //buffer[5] = knxLinkEncodeAtLsduLg(frame->at, frame->hop_count, frame->length);


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
     * Decodificar buffer como trama estándar en frame
     * Retornar 0 si error, 1 si ok
     */
	// Resto del código ...

    uint8_t FT;
    FT = knxLinkDecodeCtrlFt(buffer[0]);
    if (FT == 0) {
        frame->rep = knxLinkDecodeCtrlRep(buffer[0]);
        frame->prio = knxLinkDecodeCtrlPrio(buffer[0]);

        uint8_t saH, saL;
        saH = knxLinkDecodeAddressHigh(buffer[1]);
        saL = knxLinkDecodeAddressLow(buffer[2]);
        frame->sa = knxLinkEncodeAddress(saH, saL);

        uint8_t daH = 0, daL = 0;
        saH = knxLinkDecodeAddressHigh(buffer[3]);
        saL = knxLinkDecodeAddressLow(buffer[4]);
        frame->da = knxLinkEncodeAddress(daH, daL);

        frame->at = knxLinkDecodeAtLsduLgAt(buffer[5]);
        frame->hop_count = knxLinkDecodeAtLsduLgHopCount(buffer[5]);
        frame->length = knxLinkDecodeAtLsduLgLg(buffer[5]);
    }
    else {
        return 0;
    }


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
	// Resto del código ...

    uint8_t FT;
    FT = knxLinkDecodeCtrlFt(buffer[0]);
        if (FT == 1) {
            frame->rep = knxLinkDecodeCtrlRep(buffer[0]);
            frame->prio = knxLinkDecodeCtrlPrio(buffer[0]);

            frame->at = knxLinkDecodeCtrleAt(buffer[1]);
            frame->hop_count = knxLinkDecodeCtrleHopCount(buffer[1]);
            frame->ext_ff = knxLinkDecodeCtrleExtFF(buffer[1]);


            uint8_t saH, saL;
            saH = knxLinkDecodeAddressHigh(buffer[2]);
            saL = knxLinkDecodeAddressLow(buffer[3]);
            frame->sa = knxLinkEncodeAddress(saH, saL);

            uint8_t daH = 0, daL = 0;
            saH = knxLinkDecodeAddressHigh(buffer[4]);
            saL = knxLinkDecodeAddressLow(buffer[5]);
            frame->da = knxLinkEncodeAddress(daH, daL);

            //frame->length = knxLinkDecodeAtLsduLgLg(buffer[6]);
        }
        else {
            return 0;
        }


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
