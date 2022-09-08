/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "MbOut.hpp"

#include "split_string.hpp"

#include <cstddef>
#include <cxxendian.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

union r16 {
    uint16_t reg;
    uint16_t u_data;
    int16_t  i_data;
    uint8_t  ub[2];
    int8_t   ib[2];
    r16(uint16_t data) : u_data(data) {}  // NOLINT
    r16() = default;
};

union r32 {
    uint32_t u_data;
    int32_t  i_data;
    float    f_data;
    uint16_t reg[2];
    r32(uint32_t data) : u_data(data) {}  // NOLINT
    r32() = default;
};

union r64 {
    uint64_t u_data;
    int64_t  i_data;
    double   f_data;
    uint16_t reg[4];
    r64(uint64_t data) : u_data(data) {}  // NOLINT
    r64() = default;
};

MbOut::MbOut(const std::string &path, std::ostream &out, const std::string &name_prefix)
    : modbus_do(name_prefix + "DO"),
      modbus_di(name_prefix + "DI"),
      modbus_ao(name_prefix + "AO"),
      modbus_ai(name_prefix + "AI"),
      out(out) {
    int line_number;

    if (path.empty() || path == "-") {
        std::string line;
        for (std::size_t line_number = 1; std::getline(std::cin, line); ++line_number) {
            try {
                parse_config(line);
            } catch (const std::exception &e) {
                std::ostringstream sstr;
                sstr << "Failed to parse line " << line_number << " (" << line << "): " << e.what();
                throw std::runtime_error(sstr.str());
            }
        }
    } else {
        std::ifstream infile(path);

        if (!infile.is_open()) throw std::runtime_error("failed to open file");

        std::string line;
        for (std::size_t line_number = 1; std::getline(infile, line); ++line_number) {
            try {
                parse_config(line);
            } catch (const std::exception &e) {
                std::ostringstream sstr;
                sstr << "Failed to parse line " << line_number << " (" << line << "): " << e.what();
                throw std::runtime_error(sstr.str());
            }
        }

        if (infile.bad()) throw std::runtime_error("failed to read file");
    }
}

void MbOut::parse_config(const std::string &line) {
    if (line.empty()) return;

    const auto split_comment = split_string(line, '#', 1);
    if (split_comment.empty()) return;

    const auto split_line = split_string(split_comment[0], ':');

    if (split_line.size() != 2 && split_line.size() != 3) { throw std::runtime_error("to few separators"); }

    const auto  reg_type  = str_to_register_type(split_line[0]);
    const auto &index_str = split_line[1];
    const auto  data_type = split_line.size() == 3 ? str_to_data_type(split_line[2]) : data_type_t::bit;

    unsigned long long base_index;
    bool               fail = false;
    std::size_t        idx  = 0;
    try {
        base_index = std::stoull(index_str, &idx, 0);
    } catch (const std::exception &) { fail = true; }
    fail = fail || idx != index_str.size();

    if (fail) throw std::runtime_error("invalid register index format");

    signals.emplace_back(data_type, reg_type, base_index);

    // check signal

    // check data type
    const auto &signal = signals.back();
    if (signal.data_type == data_type_t::bit) {
        if (signal.register_type == register_type_t::AI || signal.register_type == register_type_t::AO)
            throw std::runtime_error("data type invalid for specified register type");
    } else {
        if (signal.register_type == register_type_t::DO || signal.register_type == register_type_t::DI)
            throw std::runtime_error("data type invalid for specified register type");
    }

    // check size
    auto min_size = (base_index + data_type_registers(signal.data_type)) * register_bytes(signal.register_type);

    cxxshm::SharedMemory *mem;
    switch (signal.register_type) {
        case register_type_t::DO: mem = &modbus_do; break;
        case register_type_t::DI: mem = &modbus_di; break;
        case register_type_t::AO: mem = &modbus_ao; break;
        case register_type_t::AI: mem = &modbus_ai; break;
    }

    if (mem->get_size() < min_size) throw std::runtime_error("register index out of range");
}

void MbOut::output_signal(const MbOut::signal_t &signal) {
    out << std::dec;

    cxxshm::SharedMemory *mem;
    switch (signal.register_type) {
        case register_type_t::DO:
            mem = &modbus_do;
            out << "do:";
            break;
        case register_type_t::DI:
            mem = &modbus_di;
            out << "di:";
            break;
        case register_type_t::AO:
            mem = &modbus_ao;
            out << "ao:";
            break;
        case register_type_t::AI:
            mem = &modbus_ai;
            out << "ai:";
            break;
        default: throw std::logic_error("unknown register type");
    }

    out << signal.base_index << ':';

    if (signal.data_type == data_type_t::bit) {
        out << (mem->at<uint8_t>(signal.base_index) ? '1' : '0') << std::endl;
        return;
    }

    union {
        r16 data16;
        r32 data32;
        r64 data64;
    } data {};

    switch (data_type_registers(signal.data_type) * register_bytes(signal.register_type)) {
        case 2: data.data16 = mem->at<uint16_t>(signal.base_index); break;
        case 4:
            data.data32 = *reinterpret_cast<uint32_t *>(reinterpret_cast<uint16_t *>(mem->get_addr()) +
                                                        register_bytes(signal.register_type) * signal.base_index);
            break;
        case 8:
            data.data64 = *reinterpret_cast<uint64_t *>(reinterpret_cast<uint16_t *>(mem->get_addr()) +
                                                        register_bytes(signal.register_type) * signal.base_index);
            break;
        default: throw std::logic_error("unexpected load size");
    }

    switch (signal.data_type) {
        case data_type_t::u8_lo: out << std::dec << "u8_lo:" << static_cast<unsigned>(data.data16.ub[0]); break;
        case data_type_t::u8_hi: out << std::dec << "u8_hi:" << static_cast<unsigned>(data.data16.ub[1]); break;
        case data_type_t::i8_lo: out << std::dec << "i8_lo:" << static_cast<int>(data.data16.ub[0]); break;
        case data_type_t::i8_hi: out << std::dec << "i8_hi:" << static_cast<int>(data.data16.ub[1]); break;
        case data_type_t::x8_lo: out << std::hex << "i8_lo:" << static_cast<unsigned>(data.data16.ub[0]); break;
        case data_type_t::x8_hi: out << std::hex << "i8_hi:" << static_cast<unsigned>(data.data16.ub[1]); break;
        case data_type_t::u16l: {
            cxxendian::LE_Int<uint16_t>   mem_e(data.data16.u_data);
            cxxendian::Host_Int<uint16_t> hots_e(data.data16.u_data);
            out << std::dec << "u16l:" << hots_e.get();
            break;
        }
        case data_type_t::u16b: {
            cxxendian::BE_Int<uint16_t>   mem_e(data.data16.u_data);
            cxxendian::Host_Int<uint16_t> hots_e(data.data16.u_data);
            out << std::dec << "u16b:" << hots_e.get();
            break;
        }
        case data_type_t::i16l: {
            cxxendian::LE_Int<int16_t>   mem_e(data.data16.i_data);
            cxxendian::Host_Int<int16_t> hots_e(data.data16.i_data);
            out << std::dec << "i16l:" << hots_e.get();
            break;
        }
        case data_type_t::i16b: {
            cxxendian::BE_Int<int16_t>   mem_e(data.data16.i_data);
            cxxendian::Host_Int<int16_t> hots_e(data.data16.i_data);
            out << std::dec << "i16b:" << hots_e.get();
            break;
        }
        case data_type_t::x16l: {
            cxxendian::LE_Int<uint16_t>   mem_e(data.data16.u_data);
            cxxendian::Host_Int<uint16_t> hots_e(data.data16.u_data);
            out << std::hex << "x16l:" << hots_e.get();
            break;
        }
        case data_type_t::x16b: {
            cxxendian::BE_Int<uint16_t>   mem_e(data.data16.u_data);
            cxxendian::Host_Int<uint16_t> hots_e(data.data16.u_data);
            out << std::hex << "x16b:" << hots_e.get();
            break;
        }
        case data_type_t::u32l: {
            cxxendian::LE_Int<uint32_t>   mem_e(data.data32.u_data);
            cxxendian::Host_Int<uint32_t> hots_e(data.data32.u_data);
            out << std::dec << "u32l:" << hots_e.get();
            break;
        }
        case data_type_t::u32lr: {
            std::swap(data.data32.reg[0], data.data32.reg[1]);
            cxxendian::LE_Int<uint32_t>   mem_e(data.data32.u_data);
            cxxendian::Host_Int<uint32_t> hots_e(data.data32.u_data);
            out << std::dec << "u32lr:" << hots_e.get();
            break;
        }
        case data_type_t::u32b: {
            cxxendian::BE_Int<uint32_t>   mem_e(data.data32.u_data);
            cxxendian::Host_Int<uint32_t> hots_e(data.data32.u_data);
            out << std::dec << "u32b:" << hots_e.get();
            break;
        }
        case data_type_t::u32br: {
            std::swap(data.data32.reg[0], data.data32.reg[1]);
            cxxendian::BE_Int<uint32_t>   mem_e(data.data32.u_data);
            cxxendian::Host_Int<uint32_t> hots_e(data.data32.u_data);
            out << std::dec << "u32br:" << hots_e.get();
            break;
        }
        case data_type_t::i32l: {
            cxxendian::LE_Int<int32_t>   mem_e(data.data32.i_data);
            cxxendian::Host_Int<int32_t> hots_e(data.data32.i_data);
            out << std::dec << "i32l:" << hots_e.get();
            break;
        }
        case data_type_t::i32lr: {
            std::swap(data.data32.reg[0], data.data32.reg[1]);
            cxxendian::LE_Int<int32_t>   mem_e(data.data32.i_data);
            cxxendian::Host_Int<int32_t> hots_e(data.data32.i_data);
            out << std::dec << "i32lr:" << hots_e.get();
            break;
        }
        case data_type_t::i32b: {
            cxxendian::BE_Int<int32_t>   mem_e(data.data32.i_data);
            cxxendian::Host_Int<int32_t> hots_e(data.data32.i_data);
            out << std::dec << "i32b:" << hots_e.get();
            break;
        }
        case data_type_t::i32br: {
            std::swap(data.data32.reg[0], data.data32.reg[1]);
            cxxendian::BE_Int<int32_t>   mem_e(data.data32.i_data);
            cxxendian::Host_Int<int32_t> hots_e(data.data32.i_data);
            out << std::dec << "i32br:" << hots_e.get();
            break;
        }
        case data_type_t::x32l: {
            cxxendian::LE_Int<uint32_t>   mem_e(data.data32.u_data);
            cxxendian::Host_Int<uint32_t> hots_e(data.data32.u_data);
            out << std::hex << "x32l:" << hots_e.get();
            break;
        }
        case data_type_t::x32lr: {
            std::swap(data.data32.reg[0], data.data32.reg[1]);
            cxxendian::LE_Int<uint32_t>   mem_e(data.data32.u_data);
            cxxendian::Host_Int<uint32_t> hots_e(data.data32.u_data);
            out << std::hex << "x32lr:" << hots_e.get();
            break;
        }
        case data_type_t::x32b: {
            cxxendian::BE_Int<uint32_t>   mem_e(data.data32.u_data);
            cxxendian::Host_Int<uint32_t> hots_e(data.data32.u_data);
            out << std::hex << "x32b:" << hots_e.get();
            break;
        }
        case data_type_t::x32br: {
            std::swap(data.data32.reg[0], data.data32.reg[1]);
            cxxendian::BE_Int<uint32_t>   mem_e(data.data32.u_data);
            cxxendian::Host_Int<uint32_t> hots_e(data.data32.u_data);
            out << std::hex << "x32br:" << hots_e.get();
            break;
        }
        case data_type_t::f32l: {
            cxxendian::LE_Float<float>   mem_e(data.data32.f_data);
            cxxendian::Host_Float<float> hots_e(data.data32.f_data);
            out << std::scientific << std::setprecision(std::numeric_limits<float>::digits10)
                << "f32l:" << hots_e.get();
            break;
        }
        case data_type_t::f32lr: {
            std::swap(data.data32.reg[0], data.data32.reg[1]);
            cxxendian::LE_Float<float>   mem_e(data.data32.f_data);
            cxxendian::Host_Float<float> hots_e(data.data32.f_data);
            out << std::scientific << std::setprecision(std::numeric_limits<float>::digits10)
                << "f32lr:" << hots_e.get();
            break;
        }
        case data_type_t::f32b: {
            cxxendian::BE_Float<float>   mem_e(data.data32.f_data);
            cxxendian::Host_Float<float> hots_e(data.data32.f_data);
            out << std::scientific << std::setprecision(std::numeric_limits<float>::digits10)
                << "f32b:" << hots_e.get();
            break;
        }
        case data_type_t::f32br: {
            std::swap(data.data32.reg[0], data.data32.reg[1]);
            cxxendian::BE_Float<float>   mem_e(data.data32.f_data);
            cxxendian::Host_Float<float> hots_e(data.data32.f_data);
            out << std::scientific << std::setprecision(std::numeric_limits<float>::digits10)
                << "f32br:" << hots_e.get();
            break;
        }
        case data_type_t::u64l: {
            cxxendian::LE_Int<uint64_t>   mem_e(data.data64.u_data);
            cxxendian::Host_Int<uint64_t> hots_e(data.data64.u_data);
            out << std::dec << "u64l:" << hots_e.get();
            break;
        }
        case data_type_t::u64lr: {
            std::swap(data.data64.reg[0], data.data64.reg[3]);
            std::swap(data.data64.reg[1], data.data64.reg[2]);
            cxxendian::LE_Int<uint64_t>   mem_e(data.data64.u_data);
            cxxendian::Host_Int<uint64_t> hots_e(data.data64.u_data);
            out << std::dec << "u64lr:" << hots_e.get();
            break;
        }
        case data_type_t::u64b: {
            cxxendian::BE_Int<uint64_t>   mem_e(data.data64.u_data);
            cxxendian::Host_Int<uint64_t> hots_e(data.data64.u_data);
            out << std::dec << "u64b:" << hots_e.get();
            break;
        }
        case data_type_t::u64br: {
            std::swap(data.data64.reg[0], data.data64.reg[3]);
            std::swap(data.data64.reg[1], data.data64.reg[2]);
            cxxendian::BE_Int<uint64_t>   mem_e(data.data64.u_data);
            cxxendian::Host_Int<uint64_t> hots_e(data.data64.u_data);
            out << std::dec << "u64br:" << hots_e.get();
            break;
        }
        case data_type_t::i64l: {
            cxxendian::LE_Int<int64_t>   mem_e(data.data64.i_data);
            cxxendian::Host_Int<int64_t> hots_e(data.data64.i_data);
            out << std::dec << "i64l:" << hots_e.get();
            break;
        }
        case data_type_t::i64lr: {
            std::swap(data.data64.reg[0], data.data64.reg[3]);
            std::swap(data.data64.reg[1], data.data64.reg[2]);
            cxxendian::LE_Int<int64_t>   mem_e(data.data64.i_data);
            cxxendian::Host_Int<int64_t> hots_e(data.data64.i_data);
            out << std::dec << "i64lr:" << hots_e.get();
            break;
        }
        case data_type_t::i64b: {
            cxxendian::BE_Int<int64_t>   mem_e(data.data64.i_data);
            cxxendian::Host_Int<int64_t> hots_e(data.data64.i_data);
            out << std::dec << "i64b:" << hots_e.get();
            break;
        }
        case data_type_t::i64br: {
            std::swap(data.data64.reg[0], data.data64.reg[3]);
            std::swap(data.data64.reg[1], data.data64.reg[2]);
            cxxendian::BE_Int<int64_t>   mem_e(data.data64.i_data);
            cxxendian::Host_Int<int64_t> hots_e(data.data64.i_data);
            out << std::dec << "i64br:" << hots_e.get();
            break;
        }
        case data_type_t::x64l: {
            cxxendian::LE_Int<uint64_t>   mem_e(data.data64.u_data);
            cxxendian::Host_Int<uint64_t> hots_e(data.data64.u_data);
            out << std::hex << "x64l:" << hots_e.get();
            break;
        }
        case data_type_t::x64lr: {
            std::swap(data.data64.reg[0], data.data64.reg[3]);
            std::swap(data.data64.reg[1], data.data64.reg[2]);
            cxxendian::LE_Int<uint64_t>   mem_e(data.data64.u_data);
            cxxendian::Host_Int<uint64_t> hots_e(data.data64.u_data);
            out << std::hex << "x64lr:" << hots_e.get();
            break;
        }
        case data_type_t::x64b: {
            cxxendian::BE_Int<uint64_t>   mem_e(data.data64.u_data);
            cxxendian::Host_Int<uint64_t> hots_e(data.data64.u_data);
            out << std::hex << "x64b:" << hots_e.get();
            break;
        }
        case data_type_t::x64br: {
            std::swap(data.data64.reg[0], data.data64.reg[3]);
            std::swap(data.data64.reg[1], data.data64.reg[2]);
            cxxendian::BE_Int<uint64_t>   mem_e(data.data64.u_data);
            cxxendian::Host_Int<uint64_t> hots_e(data.data64.u_data);
            out << std::hex << "x64br:" << hots_e.get();
            break;
        }
        case data_type_t::f64l: {
            cxxendian::LE_Float<double>   mem_e(data.data64.f_data);
            cxxendian::Host_Float<double> hots_e(data.data64.f_data);
            out << std::scientific << std::setprecision(std::numeric_limits<double>::digits10)
                << "f64l:" << hots_e.get();
            break;
        }
        case data_type_t::f64lr: {
            std::swap(data.data64.reg[0], data.data64.reg[3]);
            std::swap(data.data64.reg[1], data.data64.reg[2]);
            cxxendian::LE_Float<double>   mem_e(data.data64.f_data);
            cxxendian::Host_Float<double> hots_e(data.data64.f_data);
            out << std::scientific << std::setprecision(std::numeric_limits<double>::digits10)
                << "f64lr:" << hots_e.get();
            break;
        }
        case data_type_t::f64b: {
            cxxendian::BE_Float<double>   mem_e(data.data64.f_data);
            cxxendian::Host_Float<double> hots_e(data.data64.f_data);
            out << std::scientific << std::setprecision(std::numeric_limits<double>::digits10)
                << "f64b:" << hots_e.get();
            break;
        }
        case data_type_t::f64br: {
            std::swap(data.data64.reg[0], data.data64.reg[3]);
            std::swap(data.data64.reg[1], data.data64.reg[2]);
            cxxendian::BE_Float<double>   mem_e(data.data64.f_data);
            cxxendian::Host_Float<double> hots_e(data.data64.f_data);
            out << std::scientific << std::setprecision(std::numeric_limits<double>::digits10)
                << "f64br:" << hots_e.get();
            break;
        }
        default: throw std::logic_error("unknown data type");
    }

    out << std::endl;
}
