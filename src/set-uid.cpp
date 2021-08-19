////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020-2021 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "pie/device.hpp"
#include "pgm/args.hpp"

#include <exception>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

#if !defined(VERSION)
#  define VERSION "0"
#endif

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
try
{
    auto name = fs::path{ argv[0] }.filename();

    pgm::args args
    {{
        { "-s", "--set-to", "uid", "Change unit ID to <uid>."         },
        { "-h", "--help",          "Print this help screen and exit." },
        { "-v", "--version",       "Show version number and exit."    },

        { "path",                  "Path to an X-Keys device."        },
    }};

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
        auto path = fs::path{ args["path"].value() };

        asio::io_context io;
        pie::device device{ io, path };

        std::cout << "Current device uid: " << static_cast<int>(device.uid()) << std::endl;

        if(args["--set-to"])
        {
            auto s = args["--set-to"].value();
            char* end;
            auto ul = std::strtoul(s.data(), &end, 0);

            if(ul <= 255 && end == (s.data() + s.size()))
            {
                std::cout << "Changing uid to " << ul << std::endl;
                device.set_uid(ul);
                std::cout << "New device uid: " << static_cast<int>(device.uid()) << std::endl;
            }
            else throw pgm::invalid_argument{ "Invalid uid", s };
        }
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
