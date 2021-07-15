////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020-2021 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "pgm/args.hpp"
#include "pie/device.hpp"
#include "util.hpp"

#include <asio.hpp>
#include <exception>
#include <filesystem>
#include <iostream>

using namespace asio::ip;
namespace fs = std::filesystem;

#if !defined(VERSION)
#  define VERSION "0"
#endif

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
try
{
    auto name{ fs::path(argv[0]).filename() };

    pgm::args args
    {
        { "-h", "--help",            "Print this help screen and exit." },
        { "-v", "--version",         "Show version number and exit."    },
        { "path",                    "Path to Logitech R800 device."    },
    };

    // delay exception handling to process --help and --version
    std::exception_ptr ep;
    try { args.parse(argc, argv); }
    catch(...) { ep = std::current_exception(); }

    if(args["--help"])
    {
        std::cout << "\n" << args.usage(name) << "\n" << std::endl;
    }
    else if(args["--version"])
    {
        std::cout << name.string() << " version " << VERSION << std::endl;
    }
    else if(ep)
    {
        std::rethrow_exception(ep);
    }
    else
    {
        fs::path path{ args["path"].value() };

        asio::io_context io;

        src::on_interrupt([&](int signal)
        {
            std::cout << "Received signal " << signal << " - exiting." << std::endl;
            io.stop();
        });

        std::cout << "Starting event loop." << std::endl;
        io.run();
    }

    return 0;
}
catch(std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
}
catch(...)
{
    std::cerr << "???" << std::endl;
    return 1;
}
