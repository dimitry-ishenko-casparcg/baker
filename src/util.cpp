////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "util.hpp"

#include <csignal>
#include <cstdlib>

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
void set_interrupt_callback(interrupt_callback cb)
{
    static interrupt_callback cb_;

    if(!cb_)
    {
        std::signal(SIGINT, [](int signal) { cb_(signal); });
        std::signal(SIGTERM, [](int signal) { cb_(signal); });
    }

    cb_ = std::move(cb);
    if(!cb_) std::signal(SIGINT, SIG_DFL), std::signal(SIGTERM, SIG_DFL);
}

////////////////////////////////////////////////////////////////////////////////
fs::path data_path()
{
#if defined(_WIN32)
    return fs::path{ std::getenv("APPDATA") };
#elif defined(__APPLE__)
    return fs::path{ std::getenv("HOME") } / "Library" / "Application Support";
#elif defined(__unix__)
    return fs::path{ std::getenv("HOME") } / ".local" / "share";
#else
    #error "Unsupported platform"
#endif
}

////////////////////////////////////////////////////////////////////////////////
}
