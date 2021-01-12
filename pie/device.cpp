////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "device.hpp"

#include <cerrno>
#include <climits>
#include <functional>
#include <stdexcept>
#include <system_error>

#include <fcntl.h>

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

    set_on(fd_, leds::none);

    set_on(fd_, light::bank_1, all_rows);
    set_on(fd_, light::bank_2, no_rows);

    level(fd_, 255, 255);
    period(fd_, 10);

    request_data(fd_);
    sched_read();
}

////////////////////////////////////////////////////////////////////////////////
void device::read_descriptor()
{
    pie::data data;
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

        if(gd->ps)
        {
            if(!pressed_.count(ps))
            {
                set(fd_, led::red, on);
                pressed_.insert(ps);
            }
        }
        else if(pressed_.count(ps))
        {
            pressed_.erase(ps);
            set(fd_, led::red, off);

            toggle_locked();
        }

        auto [ press, release ] = decode_buttons(gd->buttons);

        for(auto b : press)
        {
            // reset pending double-press button if a different one was pressed
            if(b != pending_ && pending_ != none)
            {
                set(fd_, columns_, pending_, light::bank_1, on);
                set(fd_, columns_, pending_, light::bank_2, off);
                pending_ = none;
            }

            if(b == pending_ || !double_press_.count(b))
            {
                set(fd_, columns_, b, light::bank_1, off);
                set(fd_, columns_, b, light::bank_2, on);

                pressed_.insert(b);
                pending_ = none;
            }
            else
            {
                set(fd_, columns_, b, light::bank_1, off);
                set(fd_, columns_, b, light::bank_2, flash);
                pending_ = b;
            }
        }

        for(auto b : release)
        {
            // when locked, leave the button red (bank_2)
            if(!locked_)
            {
                set(fd_, columns_, b, light::bank_1, on);
                set(fd_, columns_, b, light::bank_2, off);
            }

            pressed_.erase(b);
        }

        sched_read();
    }
}

////////////////////////////////////////////////////////////////////////////////
void device::toggle_locked()
{
    locked_ = !locked_;
    set_on(fd_, light::bank_1, locked_ ? no_rows : all_rows);
    set_on(fd_, light::bank_2, locked_ ? all_rows : no_rows);
}

////////////////////////////////////////////////////////////////////////////////
auto device::decode_buttons(byte buttons_[]) -> std::tuple<buttons, buttons>
{
    buttons press, release;

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
                if(!locked_ && !pressed_.count(b)) press.insert(b);
            }
            else if(pressed_.count(b)) release.insert(b);
            on >>= 1;

            ++b;
        }
        b += CHAR_BIT - rows_;
    }

    return { std::move(press), std::move(release) };
}

////////////////////////////////////////////////////////////////////////////////
}
