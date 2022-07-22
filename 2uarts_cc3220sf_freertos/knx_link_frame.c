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
    int res = 0, i;
	
    /**
     * @TODO
     * Codificar frame como trama estándar en buffer
     * Retornar número de bytes de buffer utilizados, o 0 si error
     */
	// Resto del código ...

    if (bufsize < (frame->length +7)) {
        return 0; //because buffer is too short
    }

    if (frame->length >= 16) {
        return 0; //because it doesn't conform the standard
    }


    buffer[res++] = knxLinkEncodeCtrl(0, frame->rep, frame->prio);

    buffer[res++] = knxLinkDecodeAddressHigh(frame->sa);
    buffer[res++] = knxLinkDecodeAddressLow(frame->sa);

    buffer[res++] = knxLinkDecodeAddressHigh(frame->da);
    buffer[res++] = knxLinkDecodeAddressLow(frame->da);

    buffer[res++] = knxLinkEncodeAtLsduLg(frame->at, frame->hop_count, frame->length);

    for (i = 0; i< frame->length; i++) {
        buffer[res++] = frame->lsdu[i];
    }

    buffer[res++] = knxLinkCalcCHK(buffer, res);

    return res;
}

static int _knxLinkEncodeAsExtFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize) {
    int res = 0, i;
	
    /**
     * @TODO
     * Codificar frame como trama extendida en buffer
     * Retornar número de bytes de buffer utilizados, o 0 si error
     */
	// Resto del código ...

    if (bufsize < (frame->length +8)) {
        return 0; //because buffer is too short
    }

    if (frame->length >= KNX_LINK_EXT_FRAME_LSDU_MAX) {
        return 0; //because it doesn't conform the extended
    }

    if (frame->length >= KNX_LINK_EXT_FRAME_LSDU_MAX_TPUART) {
        return 0; //because it's too long for the TP uart
    }


    buffer[res++] = knxLinkEncodeCtrl(1, frame->rep, frame->prio);

    buffer[res++] = knxLinkEncodeCtrle(frame->at, frame->hop_count, frame->ext_ff);

    buffer[res++] = knxLinkDecodeAddressHigh(frame->sa);
    buffer[res++] = knxLinkDecodeAddressLow(frame->sa);

    buffer[res++] = knxLinkDecodeAddressHigh(frame->da);
    buffer[res++] = knxLinkDecodeAddressLow(frame->da);

    buffer[res++] = frame->length-1;

    for (i = 0; i< frame->length; i++) {
        buffer[res++] = frame->lsdu[i];
    }

    buffer[res++] = knxLinkCalcCHK(buffer, res);

    return res;
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
    int i;
	
    /**
     * @TODO
     * Decodificar buffer como trama estándar en frame
     * Retornar 0 si error, 1 si ok
     */
	// Resto del código ...

    if(bufsize == 0) {
        return 0; //because there is no data to decode
        }

    if (bufsize > KNX_LINK_STD_FRAME_LSDU_MAX+ 7) {
        return 0; //because too many bytes for a std frame
    }

    uint8_t FT;
    FT = knxLinkDecodeCtrlFt(buffer[0]);
    if (FT != 0) {
        return 0;
    }

    frame->rep = knxLinkDecodeCtrlRep(buffer[0]);
    frame->prio = knxLinkDecodeCtrlPrio(buffer[0]);

    uint8_t saH, saL;
    saH = knxLinkDecodeAddressHigh(buffer[1]);
    saL = knxLinkDecodeAddressLow(buffer[2]);
    frame->sa = knxLinkEncodeAddress(saH, saL);

    uint8_t daH = 0, daL = 0;
    daH = knxLinkDecodeAddressHigh(buffer[3]);
    daL = knxLinkDecodeAddressLow(buffer[4]);
    frame->da = knxLinkEncodeAddress(daH, daL);

    frame->at = knxLinkDecodeAtLsduLgAt(buffer[5]);
    frame->hop_count = knxLinkDecodeAtLsduLgHopCount(buffer[5]);
    frame->length = knxLinkDecodeAtLsduLgLg(buffer[5]);

    for (i =0; i < frame->length; i++) {
        frame->lsdu[i] = buffer[6+i];
    }

    return 1;
}

static int _knxLinkDecodeAsExtFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize) {
    int i;
	
    /**
     * @TODO
     * Decodificar buffer como trama extendida en frame
     * Retornar 0 si error, 1 si ok
     */
	// Resto del código ...

    if(bufsize == 0) {
        return 0; //because there is no data to decode
        }

    if (bufsize > KNX_LINK_EXT_FRAME_LSDU_MAX + 8) {
        return 0; //because too many bytes for a ext frame
    }

    uint8_t FT;
    FT = knxLinkDecodeCtrlFt(buffer[0]);

    if (FT != 1) {
        return 0;
    }

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
    daH = knxLinkDecodeAddressHigh(buffer[4]);
    daL = knxLinkDecodeAddressLow(buffer[5]);
    frame->da = knxLinkEncodeAddress(daH, daL);

    frame->length = buffer[6] + 1;

    for (i = 0; i < frame->length; i++) {
        frame->lsdu[i] = buffer[7+i];
    }


    return 1;
}

int knxLinkDecodeFrame(knxLinkFrame_t *frame, uint8_t *buffer, unsigned int bufsize) {
    int res;


    res = _knxLinkDecodeAsStdFrame(frame, buffer, bufsize);
    if (res == 0) {
        res = _knxLinkDecodeAsExtFrame(frame, buffer, bufsize);
    }
    return res;
}
