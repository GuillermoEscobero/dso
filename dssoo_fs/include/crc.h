/*
 * OPERATING SYSTEMS DESING - 17/18
 *
 * @file 	crc.h
 * @brief 	Headers for the CRC functionality.
 * @date	04/03/2018
 *
 * DO NOT MODIFY
 */

#ifndef _CRC_H_
#define _CRC_H_

#include <stdint.h>

/*
 * @brief	CRC16 implementation.
 *
 * @param	<buffer> to compute the CRC on.
 * @param	<length> of the buffer, in bytes.
 * @param	<prev_crc> CRC of the previous block.
 * @return	A 16-bit unsigned integer containing the resulting CRC.
 */
uint16_t CRC16(const unsigned char* buffer, unsigned int length, uint16_t prev_crc);

/*
 * @brief	CRC32 implementation.
 *
 * @param	<buffer> to compute the CRC on.
 * @param	<length> of the buffer, in bytes.
 * @param	<prev_crc> CRC of the previous block.
 * @return	A 32-bit unsigned integer containing the resulting CRC.
 */
uint32_t CRC32(const unsigned char* buffer, unsigned int length, uint32_t prev_crc);

/*
 * @brief	CRC64 implementation.
 *
 * @param	<buffer> to compute the CRC on.
 * @param	<length> of the buffer, in bytes.
 * @return	A 64-bit unsigned integer containing the resulting CRC.
 */
uint64_t CRC64(const unsigned char * buffer, unsigned int length);

#endif
