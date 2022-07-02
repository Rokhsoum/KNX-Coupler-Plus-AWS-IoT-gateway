/*
 * @file knx_link_encoding.h
 * @brief KNX library project
 */

#ifndef KNX_LINK_ENCODING_H_
#define KNX_LINK_ENCODING_H_

#include <stdint.h>

#define KNX_FRAME_CTRL_BASE                             0x10
#define KNX_FRAME_CTRL_FT_MASK                          0x80
#define KNX_FRAME_CTRL_FT_POS                           7
#define KNX_FRAME_CTRL_REP_MASK                         0x20
#define KNX_FRAME_CTRL_REP_POS                          5
#define KNX_FRAME_CTRL_PRIO_MASK                        0x0C
#define KNX_FRAME_CTRL_PRIO_POS                         2

#define KNX_FRAME_CTRLE_AT_MASK                         0x80
#define KNX_FRAME_CTRLE_AT_POS                          7
#define KNX_FRAME_CTRLE_HOPCNT_MASK                     0x70
#define KNX_FRAME_CTRLE_HOPCNT_POS                      4
#define KNX_FRAME_CTRLE_EXTFF_MASK                      0x0F
#define KNX_FRAME_CTRLE_EXTFF_POS                       0


#define KNX_FRAME_ATLSDULG_AT_MASK                      0x80
#define KNX_FRAME_ATLSDULG_AT_POS                       7
#define KNX_FRAME_ATLSDULG_HOPCNT_MASK                  0x70
#define KNX_FRAME_ATLSDULG_HOPCNT_POS                   4
#define KNX_FRAME_ATLSDULG_LG_MASK                      0x0F
#define KNX_FRAME_ATLSDULG_LG_POS                       0


/**
 * @brief Codificación de un byte CTRL
 * @param[in] frame_type Campo FT
 * @param[in] rep Campo repetition
 * @param[in] prio Campo priority
 * @return El byte CTRL codificado con los tres campos junto con los bits fijos
 */
uint8_t knxLinkEncodeCtrl(uint8_t frame_type, uint8_t rep, uint8_t prio);

/**
 * @brief Decodificación de un byte CTRL para obtener campo FT
 * @param[in] ctrl Byte CTRL con los campos FT, REP y PRIO
 * @return El campo FT codificado en el byte CTRL
 */
uint8_t knxLinkDecodeCtrlFt(uint8_t ctrl);

/**
 * @brief Decodificación de un byte CTRL para obtener campo PRIO
 * @param[in] ctrl Byte CTRL con los campos FT, REP y PRIO
 * @return El campo PRIO codificado en el byte CTRL
 */
uint8_t knxLinkDecodeCtrlRep(uint8_t ctrl);

/**
 * @brief Decodificación de un byte CTRL para obtener campo PRIO
 * @param[in] ctrl Byte CTRL con los campos FT, REP y PRIO
 * @return El campo PRIO codificado en el byte CTRL
 */
uint8_t knxLinkDecodeCtrlPrio(uint8_t ctrl);

/**
 * @brief Codificar una dirección de 16 bits a aprtir de sus bytes parte baja y parte alta
 * @param[in] high Byte con la parte alta de la dirección
 * @param[in] low Byte con la parte baja de la dirección
 * @return La dirección de 16 bits
 */
uint16_t knxLinkEncodeAddress(uint8_t high, uint8_t low);

/**
 * @brief Extraer de una dirección de 16 bits su parte alta
 * @param[in] address Dirección de 16 bits
 * @return El byte alto de la dirección
 */
uint8_t knxLinkDecodeAddressHigh(uint16_t address);

/**
 * @brief Extraer de una dirección de 16 bits su parte baja
 * @param[in] address Dirección de 16 bits
 * @return El byte bajo de la dirección
 */
uint8_t knxLinkDecodeAddressLow(uint16_t address);

/**
 * @brief Codificación de un byte AT/LSDU/LG
 * @param[in] at Campo AT
 * @param[in] hop_count Campo HOP COUNT (también denominado LSDU de 3 bits)
 * @param[in] lg Campo LG, length o longitud del LSDU
 * @return El byte AT/LSDU/LG codificado con los tres campos
 */
uint8_t knxLinkEncodeAtLsduLg(uint8_t at, uint8_t hop_count, uint8_t lg);

/**
 * @brief Decodificación de un byte AT/LSDU/LG para obtener campo AT
 * @param[in] atlsdulg Byte AT/LSDU/LG con los campos AT, HOP COUNT y LG
 * @return El campo AT codificado en el byte AT/LSDU/LG
 */
uint8_t knxLinkDecodeAtLsduLgAt(uint8_t atlsdulg);

/**
 * @brief Decodificación de un byte AT/LSDU/LG para obtener campo HOP COUNT
 * @param[in] atlsdulg Byte AT/LSDU/LG con los campos AT, HOP COUNT y LG
 * @return El campo HOP COUNT codificado en el byte AT/LSDU/LG
 */
uint8_t knxLinkDecodeAtLsduLgHopCount(uint8_t atlsdulg);

/**
 * @brief Decodificación de un byte AT/LSDU/LG para obtener campo LG
 * @param[in] atlsdulg Byte AT/LSDU/LG con los campos AT, HOP COUNT y LG
 * @return El campo LG codificado en el byte AT/LSDU/LG
 */
uint8_t knxLinkDecodeAtLsduLgLg(uint8_t atlsdulg);

/**
 * @brief Codificación de un byte CTRLE
 * @param[in] at Campo AT
 * @param[in] hop_count Campo HOP COUNT
 * @param[in] ext_ff Campo EXT FRAME FORMAT
 * @return El byte CTRLE codificado con los tres campos
 */
uint8_t knxLinkEncodeCtrle(uint8_t at, uint8_t hop_count, uint8_t ext_ff);

/**
 * @brief Decodificación de un byte CTRLE para obtener campo AT
 * @param[in] ctrle Byte CTRLE con los campos AT, HOP COUNT y EXT FRAME FORMAT (EXT FF)
 * @return El campo AT codificado en el byte CTRLE
 */
uint8_t knxLinkDecodeCtrleAt(uint8_t ctrle);

/**
 * @brief Decodificación de un byte CTRLE para obtener campo HOP COUNT
 * @param[in] ctrle Byte CTRLE con los campos AT, HOP COUNT y EXT FRAME FORMAT (EXT FF)
 * @return El campo HOP COUNT codificado en el byte CTRLE
 */
uint8_t knxLinkDecodeCtrleHopCount(uint8_t ctrle);

/**
 * @brief Decodificación de un byte CTRLE para obtener campo EXT FF
 * @param[in] ctrle Byte CTRLE con los campos AT, HOP COUNT y EXT FRAME FORMAT (EXT FF)
 * @return El campo EXT FF codificado en el byte CTRLE
 */
uint8_t knxLinkDecodeCtrleExtFF(uint8_t ctrle);

#endif /* KNX_LINK_ENCODING_H_ */
