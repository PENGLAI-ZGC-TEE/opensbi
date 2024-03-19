// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "hmac/abs_mmio.h"

// `extern` declarations to give the inline functions in the corresponding
// header a link location.

/**
 * Reads uint8_t from MMIO `addr`.
 *
 * @param addr the address to read from.
 * @return the read value.
 */
OT_WARN_UNUSED_RESULT
inline uint8_t abs_mmio_read8(uint32_t addr) {
  return *((volatile uint8_t *)addr);
}

/**
 * Writes uint8_t to the MMIO `addr`.
 *
 * @param addr the address to write to.
 * @param value the value to write.
 */
inline void abs_mmio_write8(uint32_t addr, uint8_t value) {
  *((volatile uint8_t *)addr) = value;
}

/**
 * Writes uint8_t to the MMIO `addr` via
 * two subsequent write operations.
 *
 * @param addr the address to write to.
 * @param value the value to write.
 */
inline void abs_mmio_write8_shadowed(uint32_t addr, uint8_t value) {
  *((volatile uint8_t *)addr) = value;
  *((volatile uint8_t *)addr) = value;
}

/**
 * Reads an aligned uint32_t from MMIO `addr`.
 *
 * @param addr the address to read from.
 * @return the read value.
 */
OT_WARN_UNUSED_RESULT
inline uint32_t abs_mmio_read32(uint32_t addr) {
  return *((volatile uint32_t *)addr);
}

/**
 * Writes an aligned uint32_t to the MMIO `addr`.
 *
 * @param addr the address to write to.
 * @param value the value to write.
 */
inline void abs_mmio_write32(uint32_t addr, uint32_t value) {
  *((volatile uint32_t *)addr) = value;
}

/**
 * Writes an aligned uint32_t to the MMIO `addr` via
 * two subsequent write operations.
 *
 * @param addr the address to write to.
 * @param value the value to write.
 */
inline void abs_mmio_write32_shadowed(uint32_t addr, uint32_t value) {
  *((volatile uint32_t *)addr) = value;
  *((volatile uint32_t *)addr) = value;
}


