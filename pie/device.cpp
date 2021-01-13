////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "device.hpp"

#include <cerrno>
#include <climits> // CHAR_BIT
#include <functional>
#include <stdexcept>
#include <system_error>

#include <fcntl.h> // open

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
device::device(asio::io_context& io, const fs::path& path) :
    fd_(io)
{
    auto fd = ::open(path.c_str(), O_RDWR);
    if(fd == -1) throw std::system_error{
        std::error_code{ errno, std::generic_category() }
    };
    fd_.assign(fd);

    request_descriptor(fd_);
    read_descriptor();

    leds_on(fd_, leds::none);

    light_on(fd_, light::bank_1, all_rows);
    light_on(fd_, light::bank_2, no_rows);

    level(fd_, 255, 255);
    period(fd_, 10);

    request_data(fd_);
    sched_read();
}

////////////////////////////////////////////////////////////////////////////////
void device::double_press(button btn)
{
    double_press_.insert(btn);
}

////////////////////////////////////////////////////////////////////////////////
void device::group(button btn, int id)
{
    group_[btn] = id;
}

////////////////////////////////////////////////////////////////////////////////
void device::read_descriptor()
{
    recv data;
    auto n = fd_.read_some(asio::buffer(data));
    if(n < sizeof(descriptor_data)) throw std::runtime_error{
        "Short read - descriptor_data"
    };
    auto dd = data.as<descriptor_data>();

    uid_ = dd->uid;
    columns_ = dd->columns;
    rows_ = dd->rows;
}

////////////////////////////////////////////////////////////////////////////////
void device::sched_read()
{
    using namespace std::placeholders;
    fd_.async_read_some(asio::buffer(data_), std::bind(&device::read_data, this, _1, _2));
}

////////////////////////////////////////////////////////////////////////////////
void device::read_data(const asio::error_code& ec, std::size_t n)
{
    if(!ec)
    {
        if(n < sizeof(general_data)) throw std::runtime_error{
            "Short read - general_data"
        };
        auto gd = data_.as<general_data>();

        if(!gd->ps)
        {
            if(pressed_.count(ps))
            {
                release(ps);
                toggle_locked();
            }
        }
        else if(!pressed_.count(ps)) press(ps);

        auto [ pressed, released ] = decode_buttons(gd->buttons);

        for(auto btn : pressed)
        {
            // reset pending double-press button
            // if a different one was pressed
            if(btn != pending_ && pending_ != none) un_pend();

            if(btn == pending_ || !double_press_.count(btn))
            {
                // if this button is part of a group
                if(auto gi = group_.find(btn); gi != group_.end())
                {
                    auto grp = gi->second;

                    // find which button is currently active in this group
                    // and release it
                    if(auto bi = active_.find(grp); bi != active_.end()) release(bi->second);

                    active_[grp] = btn;
                }

                press(btn);
            }
            else pend(btn);
        }

        for(auto btn : released)
        {
            // only release un-groupped buttons
            // group buttons are released when another in the same group is pressed
            if(group_.find(btn) == group_.end()) release(btn);
        }

        sched_read();
    }
}

////////////////////////////////////////////////////////////////////////////////
void device::toggle_locked()
{
    locked_ = !locked_;
    if(locked_)
    {
        light_on(fd_, light::bank_1, no_rows);
        light_on(fd_, light::bank_2, all_rows);
    }
    else
    {
        light_on(fd_, light::bank_1, all_rows);
        light_on(fd_, light::bank_2, no_rows);

        // light up all active group buttons
        for(auto [_, btn] : active_)
        {
            light_state(fd_, columns_, btn, light::bank_1, off);
            light_state(fd_, columns_, btn, light::bank_2, on);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
auto device::decode_buttons(byte buttons_[]) -> std::tuple<buttons, buttons>
{
    buttons pressed, released;

    button b = 0;
    for(auto col = 0; col < columns_; ++col)
    {
        auto on = buttons_[col];
        for(auto row = 0; row < rows_; ++row)
        {
            if(on & 1)
            {
                // don't allow button presses when locked,
                // only allow releases
                if(!locked_ && !pressed_.count(b)) pressed.insert(b);
            }
            else if(pressed_.count(b)) released.insert(b);
            on >>= 1;

            ++b;
        }
        b += CHAR_BIT - rows_;
    }

    return { std::move(pressed), std::move(released) };
}

////////////////////////////////////////////////////////////////////////////////
void device::pend(button b)
{
    light_state(fd_, columns_, b, light::bank_1, off);
    light_state(fd_, columns_, b, light::bank_2, flash);
    pending_ = b;
}

////////////////////////////////////////////////////////////////////////////////
void device::un_pend()
{
    light_state(fd_, columns_, pending_, light::bank_1, on);
    light_state(fd_, columns_, pending_, light::bank_2, off);
    pending_ = none;
}

////////////////////////////////////////////////////////////////////////////////
void device::press(button btn)
{
    if(btn != ps)
    {
        light_state(fd_, columns_, btn, light::bank_1, off);
        light_state(fd_, columns_, btn, light::bank_2, on);
    }
    else led_state(fd_, led::red, on);

    pressed_.insert(btn);
    pending_ = none;
}

////////////////////////////////////////////////////////////////////////////////
void device::release(button btn)
{
    if(btn != ps)
    {
        // when locked, leave the button red (bank_2)
        if(!locked_)
        {
            light_state(fd_, columns_, btn, light::bank_1, on);
            light_state(fd_, columns_, btn, light::bank_2, off);
        }
    }
    else led_state(fd_, led::red, off);

    pressed_.erase(btn);
}

////////////////////////////////////////////////////////////////////////////////
}
