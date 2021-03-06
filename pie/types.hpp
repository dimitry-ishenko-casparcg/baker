////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020-2021 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_TYPES_HPP
#define PIE_TYPES_HPP

////////////////////////////////////////////////////////////////////////////////
#include <array>
#include <asio.hpp>
#include <climits>
#include <cstdint>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
using byte = std::uint8_t;
using word = std::uint16_t;

////////////////////////////////////////////////////////////////////////////////
using index = byte;
constexpr index ps = -1;
constexpr index none = -2;

////////////////////////////////////////////////////////////////////////////////
namespace leds
{
enum color : byte { green = 0x40, red = 0x80, none = 0, all = 0xc0 };
}

////////////////////////////////////////////////////////////////////////////////
namespace led
{
enum color : byte { green = 6, red = 7 };
}

////////////////////////////////////////////////////////////////////////////////
namespace light
{
enum bank : byte { bank_1 = 0, bank_2 = 1 };
}

////////////////////////////////////////////////////////////////////////////////
enum state : byte { off = 0, on = 1, flash = 2 };

////////////////////////////////////////////////////////////////////////////////
enum rows : byte
{
    row_1 = 0x01, row_2 = 0x02, row_3 = 0x04, row_4 = 0x08,
    row_5 = 0x10, row_6 = 0x20, row_7 = 0x40, row_8 = 0x80,

    no_rows = 0, all_rows = 0xff
};

////////////////////////////////////////////////////////////////////////////////
struct recv : std::array<byte, 42>
{
    template<typename T>
    T* as() { return reinterpret_cast<T*>(data()); }

    template<typename T>
    const T* as() const { return reinterpret_cast<const T*>(data()); }
};

////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)

struct general_data
{
    byte uid;

    bool ps :1;
    bool reset :1;
    byte _pad :6;

    byte buttons[16];
};

struct descriptor_data
{
    byte uid;
    byte cmd;

    byte mode;
    byte _pad[4];

    byte columns;
    byte rows;

    byte _pad2 :6;
    bool green_led_on :1;
    bool red_led_on :1;

    byte ver;
    word pid;
};

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////
using fd = asio::posix::stream_descriptor;

// request current state data
void request_data(fd&);

// set PS LED state
void led_state(fd&, led::color, state);

// set backlight/LED flash period
void period(fd&, byte);

// set backlight state
void light_state(fd&, byte columns, index, light::bank, state);

// turn on/off rows of backlights
void light_on(fd&, light::bank, rows);

// turn on/off PS LEDs
void leds_on(fd&, leds::color);

// set backlight intensity
void level(fd&, byte bank_1, byte bank_2);

// set uid
void uid(fd&, byte uid);

// request descriptor data
void request_descriptor(fd&);

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
