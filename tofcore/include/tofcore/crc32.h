#ifndef CRC32_H
#   define CRC32_H
/**
 * @file crc32
 *
 * Copyright 2021 PreAct Technologies
 *
 */
#   include <stdint.h>

#include <cstddef>

/// Calculate the CRC32 of a buffer
///
/// @param buffer Pointer to the data
/// @param length Number of bytes in the buffer
uint32_t calcCrc32(const uint8_t *buffer, std::size_t length);

/// Update a running CRC32 calculation
///
/// @param crc    Existing CRC value
/// @param buffer Pointer to next chunk of data
/// @param length Number of bytes in the chunk
/// @return New CRC value
uint32_t updateCrc32(uint32_t crc, const uint8_t *buffer, std::size_t length);

#endif // CRC32_H
