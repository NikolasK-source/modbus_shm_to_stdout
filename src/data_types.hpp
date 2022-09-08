/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#pragma once

#include <string>
/**
 * List of possible data types
 */
enum class data_type_t {
    bit,   /**< single bit (only option for do and di registers; not available for ao and ai registers) */
    u8_lo, /**< low byte of register as unsigned int */
    u8_hi, /**< high byte of register as unsigned int */
    i8_lo, /**< low byte of register as signed int */
    i8_hi, /**< high byte of register as signed int */
    x8_lo, /**< low byte of register as hex value */
    x8_hi, /**< high byte of register as hex value */
    u16l,  /**< 16 bit unsigned integer little endian */
    u16b,  /**< 16 bit unsigned integer big endian */
    i16l,  /**< 16 bit signed integer little endian */
    i16b,  /**< 16 bit signed integer big endian */
    x16l,  /**< 16 bit hexadecimal little endian */
    x16b,  /**< 16 bit hexadecimal big endian */
    u32l,  /**< 32 bit unsigned integer little endian */
    u32lr, /**< 32 bit unsigned integer little endian; reversed register order */
    u32b,  /**< 32 bit unsigned integer big endian */
    u32br, /**< 32 bit unsigned integer big endian; reversed register order */
    i32l,  /**< 32 bit signed integer little endian */
    i32lr, /**< 32 bit signed integer little endian; reversed register order */
    i32b,  /**< 32 bit signed integer big endian */
    i32br, /**< 32 bit signed integer big endian; reversed register order */
    x32l,  /**< 32 bit hexadecimal little endian */
    x32lr, /**< 32 bit hexadecimal little endian; reversed register order */
    x32b,  /**< 32 bit hexadecimal big endian */
    x32br, /**< 32 bit hexadecimal big endian; reversed register order */
    u64l,  /**< 64 bit unsigned integer little endian */
    u64lr, /**< 64 bit unsigned integer little endian; reversed register order */
    u64b,  /**< 64 bit unsigned integer big endian */
    u64br, /**< 64 bit unsigned integer big endian; reversed register order */
    i64l,  /**< 64 bit signed integer little endian */
    i64lr, /**< 64 bit signed integer little endian; reversed register order */
    i64b,  /**< 64 bit signed integer big endian */
    i64br, /**< 64 bit signed integer big endian; reversed register order */
    x64l,  /**< 64 bit hexadecimal little endian */
    x64lr, /**< 64 bit hexadecimal little endian; reversed register order */
    x64b,  /**< 64 bit hexadecimal big endian */
    x64br, /**< 64 bit hexadecimal big endian; reversed register order */
    f32l,  /**< 32 bit floating point little endian */
    f32lr, /**< 32 bit floating point little endian; reversed register order */
    f32b,  /**< 32 bit floating point big endian */
    f32br, /**< 32 bit floating point big endian; reversed register order */
    f64l,  /**< 64 bit floating point little endian */
    f64lr, /**< 64 bit floating point little endian; reversed register order */
    f64b,  /**< 64 bit floating point big endian */
    f64br, /**< 64 bit floating point big endian; reversed register order */
};

data_type_t str_to_data_type(const std::string &str);

std::size_t data_type_registers(data_type_t data_type);

enum class register_type_t {
    DO, /**< digital output register */
    DI, /**< digital input register */
    AO, /**< analog output register */
    AI, /**< analog input register */
};

register_type_t str_to_register_type(const std::string &str);

std::size_t register_bytes(register_type_t register_type);
