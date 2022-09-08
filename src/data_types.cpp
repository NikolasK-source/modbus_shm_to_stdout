/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "data_types.hpp"

#include <stdexcept>
#include <unordered_map>

const static std::unordered_map<std::string, data_type_t> DATA_TYPE_MAP = {
        {"bit", data_type_t::bit},      {"u16_lo", data_type_t::u8_lo}, {"u16_hi", data_type_t::u8_hi},
        {"i16_lo", data_type_t::i8_lo}, {"i16_hi", data_type_t::i8_hi}, {"x16_lo", data_type_t::x8_lo},
        {"x16_hi", data_type_t::x8_hi}, {"u16l", data_type_t::u16l},    {"u16b", data_type_t::u16b},
        {"i16l", data_type_t::i16l},    {"i16b", data_type_t::i16b},    {"x16l", data_type_t::x16l},
        {"x16b", data_type_t::x16b},    {"u32l", data_type_t::u32l},    {"u32lr", data_type_t::u32lr},
        {"u32b", data_type_t::u32b},    {"u32br", data_type_t::u32br},  {"i32l", data_type_t::i32l},
        {"i32lr", data_type_t::i32lr},  {"i32b", data_type_t::i32b},    {"i32br", data_type_t::i32br},
        {"x32l", data_type_t::x32l},    {"x32lr", data_type_t::x32lr},  {"x32b", data_type_t::x32b},
        {"x32br", data_type_t::x32br},  {"u64l", data_type_t::u64l},    {"u64lr", data_type_t::u64lr},
        {"u64b", data_type_t::u64b},    {"u64br", data_type_t::u64br},  {"i64l", data_type_t::i64l},
        {"i64lr", data_type_t::i64lr},  {"i64b", data_type_t::i64b},    {"i64br", data_type_t::i64br},
        {"x64l", data_type_t::x64l},    {"x64lr", data_type_t::x64lr},  {"x64b", data_type_t::x64b},
        {"x64br", data_type_t::x64br},  {"f32l", data_type_t::f32l},    {"f32lr", data_type_t::f32lr},
        {"f32b", data_type_t::f32b},    {"f32br", data_type_t::f32br},  {"f64l", data_type_t::f64l},
        {"f64lr", data_type_t::f64lr},  {"f64b", data_type_t::f64b},    {"f64br", data_type_t::f64br},
};

const static std::unordered_map<std::string, register_type_t> REGISTER_TYPE_MAP = {
        {"do", register_type_t::DO},
        {"dO", register_type_t::DO},
        {"Do", register_type_t::DO},
        {"DO", register_type_t::DO},
        {"di", register_type_t::DI},
        {"dI", register_type_t::DI},
        {"Di", register_type_t::DI},
        {"DI", register_type_t::DI},
        {"ao", register_type_t::AO},
        {"aO", register_type_t::AO},
        {"Ao", register_type_t::AO},
        {"AO", register_type_t::AO},
        {"ai", register_type_t::AI},
        {"aI", register_type_t::AI},
        {"Ai", register_type_t::AI},
        {"AI", register_type_t::AI},
};

data_type_t str_to_data_type(const std::string &str) {
    try {
        return DATA_TYPE_MAP.at(str);
    } catch (const std::out_of_range &) { throw std::runtime_error("unknown data type string"); }
}

std::size_t data_type_registers(data_type_t data_type) {
    switch (data_type) {
        case data_type_t::bit:
        case data_type_t::u8_lo:
        case data_type_t::u8_hi:
        case data_type_t::i8_lo:
        case data_type_t::i8_hi:
        case data_type_t::x8_lo:
        case data_type_t::x8_hi:
        case data_type_t::u16l:
        case data_type_t::u16b:
        case data_type_t::i16l:
        case data_type_t::i16b:
        case data_type_t::x16l:
        case data_type_t::x16b: return 1;
        case data_type_t::u32l:
        case data_type_t::u32lr:
        case data_type_t::u32b:
        case data_type_t::u32br:
        case data_type_t::i32l:
        case data_type_t::i32lr:
        case data_type_t::i32b:
        case data_type_t::i32br:
        case data_type_t::x32l:
        case data_type_t::x32lr:
        case data_type_t::x32b:
        case data_type_t::x32br:
        case data_type_t::f32l:
        case data_type_t::f32lr:
        case data_type_t::f32b:
        case data_type_t::f32br: return 2;
        case data_type_t::u64l:
        case data_type_t::u64lr:
        case data_type_t::u64b:
        case data_type_t::u64br:
        case data_type_t::i64l:
        case data_type_t::i64lr:
        case data_type_t::i64b:
        case data_type_t::i64br:
        case data_type_t::x64l:
        case data_type_t::x64lr:
        case data_type_t::x64b:
        case data_type_t::x64br:
        case data_type_t::f64l:
        case data_type_t::f64lr:
        case data_type_t::f64b:
        case data_type_t::f64br: return 4;
    }
}

register_type_t str_to_register_type(const std::string &str) {
    try {
        return REGISTER_TYPE_MAP.at(str);
    } catch (const std::out_of_range &) { throw std::runtime_error("unknown register type string"); }
}

std::size_t register_bytes(register_type_t register_type) {
    switch (register_type) {
        case register_type_t::DO:
        case register_type_t::DI: return 1;
        case register_type_t::AO:
        case register_type_t::AI: return 2;
    }
}
