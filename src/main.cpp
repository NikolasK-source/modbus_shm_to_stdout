/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "CyclicOut.hpp"

#include "EventOut.hpp"
#include "cxxitimer.hpp"
#include "cxxopts.hpp"
#include "cxxsignal.hpp"
#include "license.hpp"
#include <filesystem>
#include <iostream>
#include <memory>
#include <sysexits.h>

constexpr std::size_t DEFAULT_CYCLE = 1000;  // 1s
constexpr std::size_t DEFAULT_POLL  = 10;    // 10 ms

class TerminateHandler final : public cxxsignal::SignalHandler {
private:
    static volatile bool _terminate;

public:
    explicit TerminateHandler(int signal_number) : cxxsignal::SignalHandler(signal_number) {}
    void                        handler(int signal_number, siginfo_t *, ucontext_t *) override { _terminate = true; }
    static inline volatile bool terminate() { return _terminate; }
};

volatile bool TerminateHandler::_terminate = false;

class CycleTimeWarning final : public cxxsignal::SignalHandler {
private:
    volatile bool waiting = false;

public:
    explicit CycleTimeWarning(int signal_number) : cxxsignal::SignalHandler(signal_number), waiting(false) {}

    void handler(int signal_number, siginfo_t *, ucontext_t *context) override {
        if (!waiting) std::cerr << "WARNING: cycle time exceeded" << std::endl;
    }

    bool _wait() {
        waiting  = true;
        auto ret = wait();
        waiting  = false;
        return ret;
    }
};

int main(int argc, char **argv) {
    TerminateHandler int_handler(SIGINT);
    TerminateHandler term_handler(SIGTERM);
    TerminateHandler quit_handler(SIGQUIT);
    CycleTimeWarning timer_handler(SIGALRM);

    const std::string exe_name = std::filesystem::path(argv[0]).filename().string();
    cxxopts::Options  options(PROJECT_NAME, "Print Modbus shared memory data to stdout");

    auto exit_usage = [&exe_name]() {
        std::cerr << "Use '" << exe_name << " --help' for more information." << std::endl;
        return EX_USAGE;
    };

    options.add_options()("c,cycle",
                          "cycle time in milliseconds (if --event is used, it sets the memory poll time)",
                          cxxopts::value<std::size_t>());
    options.add_options()("e,event", "enable event mode (output only changed signals)");
    options.add_options()("s,single", "enable single mode (output only once)");
    options.add_options()("h,help", "Show usage information");
    options.add_options()("version", "print version information");
    options.add_options()("license", "show licences");
    options.add_options()("file", "list of signals to output", cxxopts::value<std::string>());

    options.parse_positional({"file"});
    options.positional_help("SIGNAL_LIST");

    cxxopts::ParseResult opts;
    try {
        opts = options.parse(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return exit_usage();
    }

    // print usage
    if (opts.count("help")) {
        options.set_width(120);
        std::cout << options.help() << std::endl;
        std::cout << std::endl;
        std::cout << "This application uses the following libraries:" << std::endl;
        std::cout << "  - cxxopts by jarro2783 (https://github.com/jarro2783/cxxopts)" << std::endl;
        std::cout << "  - cxxshm (https://github.com/NikolasK-source/cxxshm)" << std::endl;
        std::cout << "  - cxxendian (https://github.com/NikolasK-source/cxxendian)" << std::endl;
        std::cout << "  - cxxsignal (https://github.com/NikolasK-source/cxxsignal)" << std::endl;
        std::cout << "  - cxxitimer (https://github.com/NikolasK-source/cxxitimer)" << std::endl;
        return EX_OK;
    }

    // print usage
    if (opts.count("version")) {
        std::cout << PROJECT_NAME << ' ' << PROJECT_VERSION << " (compiled with " << COMPILER_INFO << " on "
                  << SYSTEM_INFO << ')' << std::endl;
        return EX_OK;
    }

    // print licenses
    if (opts.count("license")) {
        print_licenses(std::cout);
        return EX_OK;
    }

    const bool  EVENT_MODE  = opts.count("event") != 0;
    const bool  SINGLE_MODE = opts.count("single") != 0;
    std::size_t cycle_ms    = EVENT_MODE ? DEFAULT_POLL : DEFAULT_CYCLE;
    if (opts.count("cycle")) {
        try {
            cycle_ms = opts["cycle"].as<std::size_t>();
        } catch (const std::exception &e) {
            std::cout << "failed to parse cycle time: " << e.what();
            return EX_USAGE;
        }

        if (cycle_ms == 0) {
            std::cerr << "invalid cycle time" << std::endl;
            return EX_USAGE;
        }
    }

    try {
        int_handler.establish();
        term_handler.establish();
        quit_handler.establish();
        timer_handler.establish();
    } catch (const std::system_error &e) {
        std::cerr << "Failed to establish signal handler: " << e.what() << std::endl;
        return EX_OSERR;
    }

    std::string file;
    if (opts["file"].count()) file = opts["file"].as<std::string>();

    std::shared_ptr<MbOut> init_out;
    std::shared_ptr<MbOut> mb_out;
    try {
        init_out = std::make_shared<CyclicOut>(file, std::cout);
        if (EVENT_MODE) {
            mb_out = std::make_shared<EventOut>(file, std::cout);
        } else {
            mb_out = init_out;
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EX_DATAERR;
    }

    cxxitimer::ITimer_Real timer(static_cast<double>(cycle_ms) / 1000.0);
    timer.start();

    if (!TerminateHandler::terminate()) {
        init_out->cycle();

        do {
            try {
                timer_handler._wait();
            } catch (const std::exception &e) {
                if (TerminateHandler::terminate()) break;
                std::cerr << "ERROR: " << e.what() << std::endl;
                return EX_OSERR;
            }

            mb_out->cycle();
        } while (!TerminateHandler::terminate() && !SINGLE_MODE);
    }
}
