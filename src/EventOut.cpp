/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include <cstring>
#include <ostream>

#include "EventOut.hpp"

EventOut::EventOut(const std::string &path, std::ostream &out, const std::string &name_prefix)
    : MbOut(path, out, name_prefix) {
    local_do = std::make_unique<uint8_t[]>(modbus_do.get_size());
    local_di = std::make_unique<uint8_t[]>(modbus_di.get_size());
    local_ao = std::make_unique<uint16_t[]>(modbus_ao.get_size() / 2);
    local_ai = std::make_unique<uint16_t[]>(modbus_ai.get_size() / 2);

    memcpy(local_do.get(), modbus_do.get_addr(), modbus_do.get_size());
    memcpy(local_di.get(), modbus_di.get_addr(), modbus_di.get_size());
    memcpy(local_ao.get(), modbus_ao.get_addr(), (modbus_ao.get_size() / 2) * 2);
    memcpy(local_ai.get(), modbus_ai.get_addr(), (modbus_ai.get_size() / 2) * 2);
}

void EventOut::cycle() {
    bool output = false;
    for (const auto &signal : signals) {
        bool diff  = false;
        auto cells = data_type_registers(signal.data_type);

        switch (signal.register_type) {
            case register_type_t::DO: {
                diff = local_do[signal.base_index] != modbus_do.at<uint8_t>(signal.base_index);
                break;
            }
            case register_type_t::DI:
                diff = local_di[signal.base_index] != modbus_di.at<uint8_t>(signal.base_index);
                break;
            case register_type_t::AO:
                for (size_t i = 0; i < cells; ++i) {
                    if (local_ao[signal.base_index + i] != modbus_ao.at<uint16_t>(signal.base_index + i)) {
                        diff = true;
                        break;
                    }
                }
                break;
            case register_type_t::AI:
                for (size_t i = 0; i < cells; ++i) {
                    if (local_ai[signal.base_index + i] != modbus_ai.at<uint16_t>(signal.base_index + i)) {
                        diff = true;
                        break;
                    }
                }
                break;
        }

        if (diff) {
            output = true;
            output_signal(signal);
            switch (signal.register_type) {
                case register_type_t::DO: local_do[signal.base_index] = modbus_do.at<uint8_t>(signal.base_index); break;
                case register_type_t::DI: local_di[signal.base_index] = modbus_di.at<uint8_t>(signal.base_index); break;
                case register_type_t::AO:
                    for (size_t i = 0; i < cells; ++i) {
                        local_ao[signal.base_index + i] = modbus_ao.at<uint16_t>(signal.base_index + i);
                    }
                    break;
                case register_type_t::AI:
                    for (size_t i = 0; i < cells; ++i) {
                        local_ai[signal.base_index + i] = modbus_ai.at<uint16_t>(signal.base_index + i);
                    }
                    break;
            };
        }

        if (output) out << std::flush;
    }
}
