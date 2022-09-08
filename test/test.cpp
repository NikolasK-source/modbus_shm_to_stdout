/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This template is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "cxxshm.hpp"
#include <array>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

static std::pair<std::string, int> exec(const char *cmd) {
    std::array<char, 4096> buffer {};
    buffer.fill(0);
    std::string result;

    FILE *pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    int exit_code = pclose(pipe);
    return {result, exit_code};
}

int main() {
    // create shared memories
    cxxshm::SharedMemory shm_ao("modbus_AO", 4096, false, true);
    cxxshm::SharedMemory shm_ai("modbus_AI", 4096, false, true);
    cxxshm::SharedMemory shm_do("modbus_DO", 4096, false, true);
    cxxshm::SharedMemory shm_di("modbus_DI", 4096, false, true);

    {  // test 1
        const int         EXPECT_EXIT = 0;
        const std::string EXPECT_OUT  = "do:0:0\n"
                                        "do:1:0\n"
                                        "ao:0:x16l:0\n"
                                        "ao:2:f32l:0.000000e+00\n"
                                        "ao:4:x16b:0\n";

        std::pair<std::string, int> result = exec("../modbus-shm-to-stdout ../../test/test_signals.txt -s");
        if (result.second != EXPECT_EXIT) {
            std::cerr << "test 1: wrong exit code" << std::endl;
            return EXIT_FAILURE;
        }

        if (result.first != EXPECT_OUT) {
            std::cerr << "test 1: wrong output: >>" << result.first << "<<" << std::endl;
            return EXIT_FAILURE;
        }
    }

    shm_do.at<uint8_t>(0)  = 1;
    shm_ao.at<uint16_t>(0) = 0x42ff;
    shm_ao.at<uint16_t>(4) = 0xff42;

    {  // test 2
        const int         EXPECT_EXIT = 0;
        const std::string EXPECT_OUT  = "do:0:1\n"
                                        "do:1:0\n"
                                        "ao:0:x16l:42ff\n"  // FIXME: will fail on big endian arch
                                       "ao:2:f32l:0.000000e+00\n"
                                       "ao:4:x16b:42ff\n";

        std::pair<std::string, int> result = exec("../modbus-shm-to-stdout ../../test/test_signals.txt -s");
        if (result.second != EXPECT_EXIT) {
            std::cerr << "test 2: wrong exit code" << std::endl;
            return EXIT_FAILURE;
        }

        if (result.first != EXPECT_OUT) {
            std::cerr << "test 2: wrong output: >>" << result.first << "<<" << std::endl;
            return EXIT_FAILURE;
        }
    }

    shm_ao.at<float>(1) = 3.141f;

    {  // test 3
        const int         EXPECT_EXIT = 0;
        const std::string EXPECT_OUT  = "do:0:1\n"
                                        "do:1:0\n"
                                        "ao:0:x16l:42ff\n"  // FIXME: will fail on big endian arch
                                       "ao:2:f32l:3.141000e+00\n"
                                       "ao:4:x16b:42ff\n";

        std::pair<std::string, int> result = exec("../modbus-shm-to-stdout ../../test/test_signals.txt -s");
        if (result.second != EXPECT_EXIT) {
            std::cerr << "test 3: wrong exit code" << std::endl;
            return EXIT_FAILURE;
        }

        if (result.first != EXPECT_OUT) {
            std::cerr << "test 3: wrong output: >>" << result.first << "<<" << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
