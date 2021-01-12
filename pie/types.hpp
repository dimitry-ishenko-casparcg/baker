////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef PIE_TYPES_HPP
#define PIE_TYPES_HPP

////////////////////////////////////////////////////////////////////////////////
#include <asio.hpp>
#include <cstdint>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace pie
{

////////////////////////////////////////////////////////////////////////////////
using byte = std::uint8_t;
using word = std::uint16_t;

////////////////////////////////////////////////////////////////////////////////
using button = int;
constexpr button ps = -1;
constexpr button none = -2;

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
class data : public std::vector<byte>
{
    using base = std::vector<byte>;

public:
    explicit data(std::size_t n = 42) : base(n, 0) { }

    template<typename T>
    T* as() { return reinterpret_cast<T*>(base::data()); }

    template<typename T>
    const T* as() const { return reinterpret_cast<T*>(base::data()); }
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

void request_data(fd&);
void set(fd&, led::color, state);
void period(fd&, byte);
void set(fd&, byte columns, button, light::bank, state);
void set_on(fd&, light::bank, rows);
void set_on(fd&, leds::color);
void level(fd&, byte bank_1, byte bank_2);
void request_descriptor(fd&);

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif