/*
 * @file knx_tpuart.h
 * @brief KNX library project
 */

#ifndef KNX_TPUART_H_
#define KNX_TPUART_H_

// ___---=== Declaraciones relacionadas con órdenes/respuestas de la TP-UART ===---___

#define TPUART_CTRLFIELD_RESET_REQUEST                  0xF1
#define TPUART_CTRLFIELD_STATE_REQUEST                  0x02
#define TPUART_CTRLFIELD_ACTIVATE_BUSMON                0x05

#define TPUART_CTRLFIELD_ACK_INFO_ACK                   0x00
#define TPUART_CTRLFIELD_ACK_INFO_NAK                   0x01
#define TPUART_CTRLFIELD_ACK_INFO_NON_BUSY              0x00
#define TPUART_CTRLFIELD_ACK_INFO_BUSY                  0x01
#define TPUART_CTRLFIELD_ACK_INFO_NON_ADDRESSED         0x00
#define TPUART_CTRLFIELD_ACK_INFO_ADDRESSED             0x01
#define TPUART_CTRLFIELD_ACK_INFO_FRAME_ERROR           0x00  //moi
#define TPUART_CTRLFIELD_ACK_INFO(nak,busy,addressed)   (0x10 | ((nak & 0x01) << 2) | ((busy & 0x01) << 1)| ((addressed & 0x01) << 0) )

#define TPUART_CTRLFIELD_PRODUCT_ID_REQUEST             0x20
#define TPUART_CTRLFIELD_ACTIVATE_BUSY_MODE             0x21
#define TPUART_CTRLFIELD_RESET_BUSY_MODE                0x22
#define TPUART_CTRLFIELD_MXRSTCNT                       0x24
#define TPUART_CTRLFIELD_SET_ADDRESS                    0x28

#define TPUART_CTRLFIELD_DATA_START                     0x80
#define TPUART_CTRLFIELD_DATA_CONT(index)               (0x80 | (index & 0x3F))
#define TPUART_CTRLFIELD_DATA_END(len)                  (0x40 | (len & 0x3F))

#define TPUART_RESPONSE_RESET_CONFIRMATION              0x03

#define TPUART_RESPONSE_DATA_CONFIRMATION_BASE          0x0B
#define TPUART_RESPONSE_DATA_CONFIRMATION_MASK          0x7F
#define TPUART_RESPONSE_DATA_CONFIRMATION_POS           0x8B
#define TPUART_RESPONSE_DATA_CONFIRMATION_NEG           0x0B

#define TPUART_RESPONSE_CTRL_STD_BASE                   0x90
#define TPUART_RESPONSE_CTRL_STD_MASK                   0xD3
#define TPUART_RESPONSE_CTRL_EXT_BASE                   0x90
#define TPUART_RESPONSE_CTRL_EXT_MASK                   0xD3
#define TPUART_RESPONSE_CTRL_POLLING_BASE               0xF0
#define TPUART_RESPONSE_CTRL_POLLING_MASK               0xFF

#endif /* KNX_TPUART_H_ */
