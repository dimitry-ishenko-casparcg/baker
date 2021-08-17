////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020-2021 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "device.hpp"

#include <cerrno>
#include <climits> // CHAR_BIT
#include <stdexcept>
#include <system_error>

#include <fcntl.h> // open

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
device::device(asio::io_context& io, const fs::path& path) :
    fd_{ io }
{
    auto fd = ::open(path.c_str(), O_RDWR);
    if(fd == -1) throw std::system_error{
        std::error_code{ errno, std::generic_category() }
    };
    fd_.assign(fd);

    request_descriptor(fd_);

    recv data;
    auto n = fd_.read_some(asio::buffer(data));
    if(n < sizeof(descriptor_data)) throw std::runtime_error{
        "Short read - descriptor_data"
    };

    auto dd = data.as<descriptor_data>();
    uid_ = dd->uid;
    columns_ = dd->columns;
    rows_ = dd->rows;
    buttons_.resize(columns_ * CHAR_BIT);

    leds_on(fd_, leds::none);

    light_on(fd_, light::bank_1, all_rows);
    light_on(fd_, light::bank_2, no_rows);

    level(fd_, 255, 255);
    period(fd_, 10);

    request_data(fd_);
    sched_read();
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
    if(ec) return;

    if(n < sizeof(general_data)) throw std::runtime_error{
        "Short read - general_data"
    };
    auto [ pressed, released ] = decode_buttons();

    // handle PS separately as it's not part of buttons_
    if(pressed.count(ps))
    {
        if(pressed_once_ != none)
        {
            un_blink(pressed_once_);
            pressed_once_ = none;
        }
        press(ps);
        pressed.erase(ps);
    }
    else if(released.count(ps))
    {
        release(ps);
        released.erase(ps);

        toggle_locked();
    }

    if(!locked_) for(auto idx : pressed)
    {
        auto& btn = buttons_[idx];
        if(btn.double_press)
        {
            if(idx == pressed_once_) // 2nd press
            {
                pressed_once_ = none;

                if(!pressed_.count(idx))
                {
                    if(btn.group) for(auto p_idx : pressed_)
                        if(buttons_[p_idx].group == btn.group)
                        {
                            release(p_idx);
                            break;
                        }
                    press(idx);
                }
                else if(btn.toggle) release(idx);
            }
            else // different button
            {
                if(pressed_once_ != none) un_blink(pressed_once_);

                if(btn.toggle || !pressed_.count(idx))
                {
                    pressed_once_ = idx;
                    blink(idx);
                }
            }
        }
        else // !btn.double_press
        {
            if(pressed_once_ != none)
            {
                un_blink(pressed_once_);
                pressed_once_ = none;
            }

            if(!pressed_.count(idx))
            {
                if(btn.group) for(auto p_idx : pressed_)
                    if(buttons_[p_idx].group == btn.group)
                    {
                        release(p_idx);
                        break;
                    }
                press(idx);
            }
            else if(btn.toggle) release(idx);
        }
    }

    for(auto idx : released)
        if(pressed_.count(idx))
        {
            auto& btn = buttons_[idx];
            // toggle and group buttons are released separately
            if(!btn.toggle && !btn.group) release(idx);
        }

    sched_read();
}

////////////////////////////////////////////////////////////////////////////////
auto device::decode_buttons() -> std::tuple<indices, indices>
{
    indices pressed, released;

    auto data = data_.as<general_data>();
    auto prev = prev_.as<general_data>();

    if(data->ps != prev->ps)
    {
        if(data->ps) pressed.insert(ps);
        else released.insert(ps);

        prev->ps = data->ps;
    }

    index idx = 0;
    for(auto col = 0; col < columns_; ++col)
    {
        auto on =  data->buttons[col] & ~prev->buttons[col];
        auto off= ~data->buttons[col] &  prev->buttons[col];
        prev->buttons[col] = data->buttons[col];

        for(auto row = 0; row < rows_; ++row)
        {
            if(on & 1) pressed.insert(idx);
            on >>= 1;

            if(off & 1) released.insert(idx);
            off >>= 1;

            ++idx;
        }
        idx += CHAR_BIT - rows_;
    }

    return { std::move(pressed), std::move(released) };
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

        for(auto idx : pressed_) activate(idx);
    }
}

////////////////////////////////////////////////////////////////////////////////
void device::blink(index idx)
{
    light_state(fd_, columns_, idx, light::bank_1, off);
    light_state(fd_, columns_, idx, light::bank_2, flash);
}

////////////////////////////////////////////////////////////////////////////////
void device::un_blink(index idx)
{
    if(pressed_.count(idx))
        activate(idx);
    else deactivate(idx);
}

////////////////////////////////////////////////////////////////////////////////
void device::activate(index idx)
{
    light_state(fd_, columns_, idx, light::bank_1, off);
    light_state(fd_, columns_, idx, light::bank_2, on);
}

////////////////////////////////////////////////////////////////////////////////
void device::deactivate(index idx)
{
    light_state(fd_, columns_, idx, light::bank_1, on);
    light_state(fd_, columns_, idx, light::bank_2, off);
}

////////////////////////////////////////////////////////////////////////////////
void device::press(index idx)
{
    if(idx != ps)
        activate(idx);
    else led_state(fd_, led::red, on);

    pressed_.insert(idx);
    if(pcall_) pcall_(idx);
}

////////////////////////////////////////////////////////////////////////////////
void device::release(index idx)
{
    if(idx != ps)
    {
        // when locked, leave the button red (bank_2)
        if(!locked_) deactivate(idx);
    }
    else led_state(fd_, led::red, off);

    pressed_.erase(idx);
    if(rcall_) rcall_(idx);
}

////////////////////////////////////////////////////////////////////////////////
}
