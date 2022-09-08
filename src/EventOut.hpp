/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#pragma once

#include "MbOut.hpp"

#include <cstddef>
#include <memory>

class EventOut : public MbOut {
private:
    std::unique_ptr<uint8_t[]>  local_do;
    std::unique_ptr<uint8_t[]>  local_di;
    std::unique_ptr<uint16_t[]> local_ao;
    std::unique_ptr<uint16_t[]> local_ai;

public:
    EventOut(const std::string &path, std::ostream &out, const std::string &name_prefix = "modbus_");
    void cycle() override;
};
