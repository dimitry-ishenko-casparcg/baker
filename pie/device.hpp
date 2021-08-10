////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020-2021 Dimitry Ishenko
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
#include <initializer_list>
#include <optional>
#include <set>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
using index_list = std::initializer_list<index>;
using callback = std::function<void (index)>;

////////////////////////////////////////////////////////////////////////////////
class device
{
public:
    explicit device(asio::io_context&, const fs::path&);

    auto uid() const { return uid_; }

    // mark button(s) as double-press
    void set_double_press(index idx) { buttons_.at(idx).double_press = true; }
    void set_double_press(index_list il) { for(auto idx : il) set_double_press(idx); }

    // mark button(s) as toggle
    void set_toggle(index idx) { buttons_.at(idx).toggle = true; }
    void set_toggle(index_list il) { for(auto idx : il) set_toggle(idx); }

    // add button(s) to a group
    void set_group(index idx, int id) { buttons_.at(idx).group = id; }
    void set_group(index_list il, int id) { for(auto idx : il) set_group(idx, id); }

    void on_press(callback cb) { pcall_ = std::move(cb); }
    void on_release(callback cb) { rcall_ = std::move(cb); }

private:
    fd fd_;
    byte uid_;
    byte columns_, rows_;

    struct button
    {
        bool double_press = false;
        bool toggle = false;
        std::optional<int> group;
    };
    std::vector<button> buttons_;

    callback pcall_, rcall_;

    recv data_{ }, prev_{ };
    void sched_read();

    using indices = std::set<index>;

    void read_data(const asio::error_code&, std::size_t n);
    std::tuple<indices, indices> decode_buttons();

    index pressed_once_ = none;
    indices pressed_;

    void blink(index);
    void un_blink(index);
    void activate(index);
    void deactivate(index);

    void press(index);
    void release(index);

    bool locked_ = false;
    void toggle_locked();
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
