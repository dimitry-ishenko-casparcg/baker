////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_DEVICE_HPP
#define PIE_DEVICE_HPP

////////////////////////////////////////////////////////////////////////////////
#include "types.hpp"

#include <asio.hpp>
#include <filesystem>
#include <set>
#include <tuple>

namespace fs = std::filesystem;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
class device
{
public:
    explicit device(asio::io_context&, const fs::path&);

    auto uid() const { return uid_; }

    auto columns() const { return columns_; }
    auto rows() const { return rows_; }

private:
    fd fd_;

    byte uid_;
    byte columns_, rows_;
    void read_descriptor();

    data data_;
    void sched_read();

    void read_data(const asio::error_code&, std::size_t n);

    using buttons = std::set<button>;

    bool locked_ = false;
    void toggle_locked();

    buttons pressed_;

    std::tuple<buttons, buttons> decode_buttons(byte[]);
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
