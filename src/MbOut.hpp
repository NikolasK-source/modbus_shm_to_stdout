/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#pragma once

#include "data_types.hpp"

#include "cxxshm.hpp"
#include <vector>

class MbOut {
protected:
    struct signal_t {
        data_type_t     data_type;
        register_type_t register_type;
        std::size_t     base_index;
        signal_t() = default;
        signal_t(data_type_t data_type, register_type_t register_type, std::size_t base_index)
            : data_type(data_type), register_type(register_type), base_index(base_index) {}
    };

    std::vector<signal_t> signals;

    cxxshm::SharedMemory modbus_do;
    cxxshm::SharedMemory modbus_di;
    cxxshm::SharedMemory modbus_ao;
    cxxshm::SharedMemory modbus_ai;

    std::ostream &out;

    MbOut(const std::string &path, std::ostream &out, const std::string &name_prefix = "modbus_");

    virtual void output_signal(const signal_t &signal);

public:
    virtual ~MbOut() = default;

    virtual void cycle() = 0;

private:
    virtual void parse_config(const std::string &line) final;
};
