/*
 * Copyright (C) 2020-2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include <iostream>
#include <omp.h>

int main() {
    // TODO implement
    std::cout << PROJECT_NAME << ' ' << PROJECT_VERSION << ' ' << "Hello CMake!!!" << std::endl;

#ifdef _OPENMP
#    pragma omp parallel default(none)
    { printf("Hello from thread: %d\n", omp_get_thread_num()); }
#endif
}
