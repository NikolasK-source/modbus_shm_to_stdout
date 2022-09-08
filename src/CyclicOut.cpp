/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "CyclicOut.hpp"
void CyclicOut::cycle() {
    for (const auto &signal : signals)
        output_signal(signal);
}
