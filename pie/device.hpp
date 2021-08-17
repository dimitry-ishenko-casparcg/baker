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
    device(asio::io_context&, const fs::path&);

    auto uid() const { return uid_; }

    // mark button(s) as double-press
    void set_double_press(index idx) { buttons_.at(idx).double_press = true; }
    template<typename It>
    void set_double_press(It begin, It end) { for(auto it = begin; it != end; ++it) set_double_press(*it); }
    void set_double_press(index_list il) { set_double_press(il.begin(), il.end()); }

    // mark button(s) as toggle
    void set_toggle(index idx) { buttons_.at(idx).toggle = true; }
    template<typename It>
    void set_toggle(It begin, It end) { for(auto it = begin; it != end; ++it) set_toggle(*it); }
    void set_toggle(index_list il) { set_toggle(il.begin(), il.end()); }

    // add button(s) to a group
    void set_group(index idx, int id) { buttons_.at(idx).group = id; }
    template<typename It>
    void set_group(It begin, It end, int id) { for(auto it = begin; it != end; ++it) set_group(*it, id); }
    void set_group(index_list il, int id) { set_group(il.begin(), il.end(), id); }

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
