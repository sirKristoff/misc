/*
 * trivialBitStreamChecker.h
 *
 *  Created on: 19.08.2018
 *      Author: Krzysztof Lasota
 */

#ifndef TRIVIALBITSTREAMCHECKER_H_
#define TRIVIALBITSTREAMCHECKER_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Verify @a stream of bits
 *
 * Function check @b bit by @b bit, state of stream with identifier @b streamNo.
 * Sequence of given bits is validated separately for each streamNo.
 * @returns false if function gets at least three this same bits in sequence
 * @returns true if sequence of last three bits does not contain identical bits.
 */
bool verify(uint8_t bit, uint8_t streamNo);

#endif /* TRIVIALBITSTREAMCHECKER_H_ */
