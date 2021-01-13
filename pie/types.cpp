////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "types.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
using send = std::array<byte, 36>;

////////////////////////////////////////////////////////////////////////////////
void request_data(pie::fd& fd)
{
    send data{ };
    data[1] = 177;
    asio::write(fd, asio::buffer(data));
}

////////////////////////////////////////////////////////////////////////////////
void led_state(pie::fd& fd, led::color c, state s)
{
    send data{ };
    data[1] = 179;
    data[2] = c;
    data[3] = s;
    asio::write(fd, asio::buffer(data));
}

////////////////////////////////////////////////////////////////////////////////
void period(pie::fd& fd, byte period)
{
    send data{ };
    data[1] = 180;
    data[2] = period;
    asio::write(fd, asio::buffer(data));
}

////////////////////////////////////////////////////////////////////////////////
void light_state(fd& fd, byte columns, button b, light::bank k, state s)
{
    send data{ };
    data[1] = 181;
    data[2] = b + (k * columns * CHAR_BIT);
    data[3] = s;
    asio::write(fd, asio::buffer(data));
}

////////////////////////////////////////////////////////////////////////////////
void light_on(pie::fd& fd, light::bank k, rows rs)
{
    send data{ };
    data[1] = 182;
    data[2] = k;
    data[3] = rs;
    asio::write(fd, asio::buffer(data));
}

////////////////////////////////////////////////////////////////////////////////
void leds_on(pie::fd& fd, leds::color c)
{
    send data{ };
    data[1] = 186;
    data[2] = c;
    asio::write(fd, asio::buffer(data));
}

////////////////////////////////////////////////////////////////////////////////
void level(pie::fd& fd, byte bank_1, byte bank_2)
{
    send data{ };
    data[1] = 187;
    data[2] = bank_1;
    data[3] = bank_2;
    asio::write(fd, asio::buffer(data));
}

////////////////////////////////////////////////////////////////////////////////
void request_descriptor(pie::fd& fd)
{
    send data{ };
    data[1] = 214;
    asio::write(fd, asio::buffer(data));
}

////////////////////////////////////////////////////////////////////////////////
}
