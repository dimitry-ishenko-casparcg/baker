////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "device.hpp"

#include <cerrno>
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

    read_descriptor();

    request_data();
    sched_read();
}

////////////////////////////////////////////////////////////////////////////////
void device::read_descriptor()
{
    request_descriptor rd;
    asio::write(fd_, asio::buffer(&rd, sizeof(rd)));

    buffer data;
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
void device::request_data()
{
    generate_data gd;
    asio::write(fd_, asio::buffer(&gd, sizeof(gd)));
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

        //

        sched_read();
    }
}

////////////////////////////////////////////////////////////////////////////////
}
