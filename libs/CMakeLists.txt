#
# Copyright (C) 2020-2022 Nikolas Koesling <nikolas@koesling.info>.
# This template is free software. You can redistribute it and/or modify it under the terms of the MIT License.
#

# ---------------------------------------- subdirectories --------------------------------------------------------------
# ======================================================================================================================

add_subdirectory(cxxopts EXCLUDE_FROM_ALL)
add_subdirectory(cxxshm EXCLUDE_FROM_ALL)
add_subdirectory(cxxendian EXCLUDE_FROM_ALL)
add_subdirectory(cxxitimer EXCLUDE_FROM_ALL)
add_subdirectory(cxxsignal EXCLUDE_FROM_ALL)

# ---------------------------------------- link libraries --------------------------------------------------------------
# ======================================================================================================================
target_link_libraries(${Target} PRIVATE cxxopts)
target_link_libraries(${Target} PRIVATE cxxshm)
target_link_libraries(${Target} PRIVATE cxxendian)
target_link_libraries(${Target} PRIVATE cxxitimer)
target_link_libraries(${Target} PRIVATE cxxsignal)
target_link_libraries(${Target} PRIVATE rt)
