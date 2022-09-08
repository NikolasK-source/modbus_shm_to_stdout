#
# Copyright (C) 2020-2022 Nikolas Koesling <nikolas@koesling.info>.
# This template is free software. You can redistribute it and/or modify it under the terms of the MIT License.
#

# ---------------------------------------- source files (*.cpp, *.cc, ...) ---------------------------------------------
# ======================================================================================================================

target_sources(${Target} PRIVATE main.cpp)
target_sources(${Target} PRIVATE data_types.cpp)
target_sources(${Target} PRIVATE MbOut.cpp)
target_sources(${Target} PRIVATE CyclicOut.cpp)
target_sources(${Target} PRIVATE license.cpp)
target_sources(${Target} PRIVATE EventOut.cpp)


# ---------------------------------------- header files (*.jpp, *.h, ...) ----------------------------------------------
# ======================================================================================================================

target_sources(${Target} PRIVATE data_types.hpp)
target_sources(${Target} PRIVATE MbOut.hpp)
target_sources(${Target} PRIVATE split_string.hpp)
target_sources(${Target} PRIVATE CyclicOut.hpp)
target_sources(${Target} PRIVATE license.hpp)
target_sources(${Target} PRIVATE EventOut.hpp)


# ---------------------------------------- subdirectories --------------------------------------------------------------
# ======================================================================================================================
