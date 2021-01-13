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
#include <functional>
#include <map>
#include <set>
#include <tuple>

namespace fs = std::filesystem;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
using callback = std::function<void (button)>;

////////////////////////////////////////////////////////////////////////////////
class device
{
public:
    explicit device(asio::io_context&, const fs::path&);

    // mark button as double-press
    void double_press(button);

    // add button to a group
    void group(button, int id);

    void pressed_callback(callback cb) { pcall_ = std::move(cb); }
    void released_callback(callback cb) { rcall_ = std::move(cb); }

private:
    fd fd_;

    byte uid_;
    byte columns_, rows_;
    void read_descriptor();

    recv data_;
    void sched_read();

    void read_data(const asio::error_code&, std::size_t n);

    using buttons = std::set<button>;

    bool locked_ = false;
    void toggle_locked();

    buttons double_press_;
    button pending_ = none; // pending double-press button (was pressed once)

    std::map<button, int> group_;
    std::map<int, button> active_;

    buttons pressed_;

    std::tuple<buttons, buttons> decode_buttons(byte[]);

    void pend(button);
    void un_pend();

    void press(button);
    void release(button);

    callback pcall_, rcall_;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
