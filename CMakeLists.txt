#
# Copyright (C) 2020-2022 Nikolas Koesling <nikolas@koesling.info>.
# This template is free software. You can redistribute it and/or modify it under the terms of the MIT License.
#

cmake_minimum_required(VERSION 3.13.4 FATAL_ERROR)

# ----------------------------------------------- User settings --------------------------------------------------------
# ======================================================================================================================

# project
project(CMAKE_Template_Project LANGUAGES CXX VERSION 0.0.1)

# settings
set(Target "Template_Project")    # Executable name (without file extension!)
set(STANDARD 20)                  # C++ Standard
set(ARCHITECTURE "native")        # CPU architecture to optimize for (only relevant if OPTIMIZE_FOR_ARCHITECTURE is ON)

# options
option(BUILD_DOC "Build documentation" ON)
option(COMPILER_WARNINGS "Enable compiler warnings" ON)
option(ENABLE_MULTITHREADING "Link the default multithreading library for the current target system" ON)
option(MAKE_32_BIT_BINARY "Compile as 32 bit application. No effect on 32 bit Systems" OFF)
option(OPENMP "enable openmp" ON)
option(OPTIMIZE_DEBUG "apply optimizations also in debug mode" ON)
option(CLANG_FORMAT "use clang-format" ON)
option(CLANG_TIDY "use clang-tidy" OFF)
option(OPTIMIZE_FOR_ARCHITECTURE "enable optimizations for specified architecture" ON)
option(LTO_ENABLED "enable interprocedural and link time optimizations" ON)
option(COMPILER_EXTENSIONS "enable compiler specific C++ extensions" OFF)
option(ENABLE_TEST "enable test builds" ON)


# ======================================================================================================================
# ======================================================================================================================


# ----------------------------------------------- Do not change --------------------------------------------------------
# ======================================================================================================================

# TODO select useful checks
if(CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY
            clang-tidy;
            -checks=*;
            )
endif()

# add executable
add_executable(${Target})
install(TARGETS ${Target})

# set source and libraries directory
add_subdirectory("src")
add_subdirectory("libs")

include(warnings.cmake)
include(define.cmake)
include(compileropts.cmake)

# force C++ Standard and disable/enable compiler specific extensions
set_target_properties(${Target} PROPERTIES
        CXX_STANDARD ${STANDARD}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ${COMPILER_EXTENSIONS}
    )

# compiler definitions and options
set_definitions(${Target})
set_options(${Target} ENABLE_MULTITHREADING AND OPENMP)

if(COMPILER_WARNINGS)
    enable_warnings(${Target})
    message(STATUS "Compiler warnings enabled.")
else()
    disable_warnings(${Target})
    message(STATUS "Compiler warnings disabled.")
endif()

if(ENABLE_MULTITHREADING)
    # required by threading lib (std::thread)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${Target} PRIVATE Threads::Threads)
endif()

# lto
if(LTO_ENABLED)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT error)
    if( ipo_supported )
        message(STATUS "IPO / LTO enabled")
        set_property(TARGET ${Target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO / LTO not supported: <${error}>")
    endif()
endif()

if(BUILD_DOC)
    # doxygen documentation (https://vicrucann.github.io/tutorials/quick-cmake-doxygen/)
    # check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        if(EXISTS ${DOXYGEN_IN})
            # request to configure the file
            configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
            message(STATUS "Doxygen configured")

            # note the option ALL which allows to build the docs together with the application
            add_custom_target( doc_doxygen ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM )
            message(STATUS "Added target doc_doxygen")
        else()
            message(WARNING "doxygen documentation requested, but file ${DOXYGEN_IN} does not exist.")
        endif()
    else (DOXYGEN_FOUND)
      message(WARNING "Doxygen need to be installed and accessible to generate the doxygen documentation.")
    endif (DOXYGEN_FOUND)
endif()

# add clang format target
if(CLANG_FORMAT)
    set(CLANG_FORMAT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/.clang-format)

    if(EXISTS ${CLANG_FORMAT_FILE})
        include(ClangFormat.cmake)
        target_clangformat_setup(${Target})
        message(STATUS "Added clang format target(s)")
    else()
        message(WARNING "Clang format enabled, but file ${CLANG_FORMAT_FILE}  does not exist")
    endif()
endif()

# add test targets
if(ENABLE_TEST)
    enable_testing()
    add_subdirectory("test")
endif()