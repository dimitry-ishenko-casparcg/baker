////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2021 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "remote.hpp"

#include <functional>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace std::placeholders;
using std::begin;
using std::end;

////////////////////////////////////////////////////////////////////////////////
namespace src
{

////////////////////////////////////////////////////////////////////////////////
namespace
{

auto parse_word(std::stringstream& ss)
{
    std::string word;
    ss >> word >> std::ws;
    return word;
}

auto parse_num(std::stringstream& ss)
{
    int num = -1;
    ss >> num >> std::ws;
    return num;
}

bool parse_equal_sign(std::stringstream& ss)
{
    char c{ };
    ss >> c >> std::ws;
    return c == '=';
}

}

////////////////////////////////////////////////////////////////////////////////
void remote::conf_from(const fs::path& path)
{
    std::fstream fs{ path, std::ios::in };
    if(!fs.good()) throw std::invalid_argument{ "Can't open file." };

    std::string read;
    for(int n = 1; std::getline(fs, read); ++n)
    {
        std::stringstream ss{ read };
        ss >> std::ws;

        auto cmd = parse_word(ss);
        if(cmd.empty() || cmd[0] == '#') continue;

        std::function<void(int)> call;

        if(cmd == "double-press")
            call = [=](int idx) { set_double_press(idx); };

        else if(cmd == "toggle")
            call = [=](int idx) { set_toggle(idx); };

        else if(cmd == "group")
        {
            auto id = parse_num(ss);
            if(id < 0) throw invalid_line{ n, "Invalid group id" };

            call = [=](int idx) { set_group(idx, id); };
        }
        else throw invalid_line{ n, "Invalid command" };

        if(!parse_equal_sign(ss)) throw invalid_line{ n, "Missing '=' sign" };

        while(!ss.eof())
        {
            auto idx = parse_num(ss);
            if(idx >= 0 && idx < buttons()) call(idx);
            else throw invalid_line{ n, "Invalid button index" };
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
}