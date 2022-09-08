/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#pragma once

#include "MbOut.hpp"

class CyclicOut : public MbOut {
public:
    CyclicOut(const std::string &path, std::ostream &out, const std::string &name_prefix = "modbus_")
        : MbOut(path, out, name_prefix) {}
    void cycle() override;
};
