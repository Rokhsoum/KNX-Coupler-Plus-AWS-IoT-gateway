/*
 * @file knx_link_gadd_pool.h
 * @brief KNX library project
 */

#ifndef KNX_LINK_GADD_POOL_H_
#define KNX_LINK_GADD_POOL_H_



/**
 * Group Addresses (GA) set datatype
 */
typedef struct {
    unsigned int size;  /**< Size of array ga_set */
    unsigned int used;  /**< Used elements in array ga_set (used elements are indexes 0 to used-1) */
    uint16_t *ga_set;   /**< Array with GAs */
} ga_set_type;

/**
 * @brief Inicializar conjunto de GAs
 * @param[in,out] gas  Conjunto de GAs
 * @param[in]     size Tamaño del array ga_set de gas
 * @return 0 en caso de error, 1 si operación exitosa
 * @details
 * Reserva memoria del pool de GAs y establece el campo (puntero) ga_set,
 * inicializa el campo used a 0 y el campo size al valor del parámetro size.
 * Si gas es NULL o no hay suficientes elementos en el pool para satisfacer
 * la petición la función retorna un código de error 0.
 */
int ga_set_init(ga_set_type *gas, unsigned int size);

/**
 * @brief Añade una GA a un conjunto de GAs evitando duplicados
 * @param[in,out] gas Conjunto de GAs
 * @param[in]     ga  GA a añadir
 * @return 0 en caso de error, 1 si operación exitosa
 * @details
 * Si gas es NULL, gas->ga_set es NULL, o no hay hueco en ga_set para añadir la nueva GA
 * (después de comprobar que no está ya en el conjunto) la función retorna un código de error 0.
 * Si la GA ya está en el conjunto la función retorna 1, pero no la añade para evitar duplicados.
 * Si la GA no está en el conjunto y queda un hueco libre en ga_set
 * la función añade la nueva GA y actualiza el campo used.
 */
int ga_set_add(ga_set_type *gas, uint16_t ga);

/**
 * @brief Busca una GA en un conjunto de GAs
 * @param[in] gas Conjunto de GAs
 * @param[in] ga  GA a buscar
 * @return 0 en caso de no encontrar el elemento, 1 si la GA ya está en el conjunto
 * @details
 * Si gas es NULL, gas->ga_set es NULL, o la GA no está en el conjunto
 * la función retorna 0.
 * Si la GA sí está en el conjunto la función retorna 1.
 */
int ga_set_in(ga_set_type *gas, uint16_t ga);

#endif /* KNX_LINK_GADD_POOL_H_ */
