/*
 * @file knx_link_state.h
 * @brief KNX library project
 */

#ifndef KNX_LINK_STATE_H_
#define KNX_LINK_STATE_H_

struct knxLinkHandle_s;
//typedef struct knxLinkHandle_s knxLinkHandle_t;


/**
 * @brief Posibles valores del estado del nivel de enlace
 */
#define KNX_LINK_ILLEGAL_STATE     -1
#define KNX_LINK_INIT_STATE         0
#define KNX_LINK_NORMAL_STATE       1
#define KNX_LINK_MONITOR_STATE      2
#define KNX_LINK_STOP_STATE         3

/**
 * @brief Inicializar el estado del nivel de enlace a KNX_LINK_ILLEGAL_STATE
 */
void knxLinkInitState(struct knxLinkHandle_s *link);
/**
 * @brief Modificar el estado del nivel de enlace
 * @param[in] link KNX link handle from knxLinkInit()
 * @param[in] newState Nuevo estado del nivel de enlace
 * La función comprueba que la transición desde el estado actual al nuevo estado newState
 * es válida; de no ser así no hace nada
 */
void knxLinkSetState(struct knxLinkHandle_s *link, int newState);
/**
 * @brief Retorna el estado actual del nivel de enlace
 * @return El estado actual del nivel de enlace
 */
int knxLinkGetState(struct knxLinkHandle_s *link);


#endif /* KNX_LINK_STATE_H_ */

