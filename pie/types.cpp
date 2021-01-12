////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "types.hpp"
#include <climits>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
template<typename T>
auto buffer_from(T& v) { return asio::buffer(&v, sizeof(v)); }

////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)

template<byte Cmd>
struct send_cmd_base
{
    const byte id = 0;
    const byte cmd = Cmd;
};

constexpr auto pad_size = 36 - sizeof(send_cmd_base<0>);

////////////////////////////////////////////////////////////////////////////////
void request_data(pie::fd& fd)
{
    struct : send_cmd_base<177>
    {
        const byte _pad[pad_size] { };
    }
    data;

    asio::write(fd, buffer_from(data));
}

////////////////////////////////////////////////////////////////////////////////
void set(pie::fd& fd, led::color c, state s)
{
    struct : send_cmd_base<179>
    {
        led::color color;
        pie::state state;
        const byte _pad[pad_size - 2] { };
    }
    data;
    data.color = c; data.state = s;

    asio::write(fd, buffer_from(data));
}

////////////////////////////////////////////////////////////////////////////////
void period(pie::fd& fd, byte period)
{
    struct : send_cmd_base<180>
    {
        byte period;
        const byte _pad[pad_size - 1] { };
    }
    data;
    data.period = period;

    asio::write(fd, buffer_from(data));
}

////////////////////////////////////////////////////////////////////////////////
void set(fd& fd, byte columns, button b, light::bank k, state s)
{
    struct : send_cmd_base<181>
    {
        byte index;
        pie::state state;
        const byte _pad[pad_size - 2] { };
    }
    data;
    data.index = b + (k * columns * CHAR_BIT);
    data.state = s;

    asio::write(fd, buffer_from(data));
}

////////////////////////////////////////////////////////////////////////////////
void set_on(pie::fd& fd, light::bank k, rows rs)
{
    struct : send_cmd_base<182>
    {
        light::bank bank;
        pie::rows rows;
        const byte _pad[pad_size - 2] { };
    }
    data;
    data.bank = k; data.rows = rs;

    asio::write(fd, buffer_from(data));
}

////////////////////////////////////////////////////////////////////////////////
void set_on(pie::fd& fd, leds::color c)
{
    struct : send_cmd_base<186>
    {
        leds::color color;
        const byte _pad[pad_size - 1] { };
    }
    data;
    data.color = c;

    asio::write(fd, buffer_from(data));
}

////////////////////////////////////////////////////////////////////////////////
void level(pie::fd& fd, byte bank_1, byte bank_2)
{
    struct : send_cmd_base<187>
    {
        byte bank_1;
        byte bank_2;
        const byte _pad[pad_size - 2] { };
    }
    data;
    data.bank_1 = bank_1; data.bank_2 = bank_2;

    asio::write(fd, buffer_from(data));
}

////////////////////////////////////////////////////////////////////////////////
void request_descriptor(pie::fd& fd)
{
    struct : send_cmd_base<214>
    {
        const byte _pad[pad_size] { };
    }
    data;

    asio::write(fd, buffer_from(data));
}

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////
}
