////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "io_cmd.hpp"
#include "chip.hpp"
#include "pin.hpp"
#include "type_id.hpp"

#include <stdexcept>
#include <utility>

#include <fcntl.h>
#include <linux/gpio.h>
#include <sys/types.h>
#include <sys/stat.h>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{
namespace generic
{

////////////////////////////////////////////////////////////////////////////////
chip::chip(asio::io_service& io, std::string id) :
    chip_base("chip"), fd_(io)
{
    if(id.find_first_not_of("0123456789") != std::string::npos
        || id.size() < 1 || id.size() > 3)
    throw std::invalid_argument(
        type_id(this) + ": Missing or invalid id " + id
    );

    id_ = std::move(id);

    ////////////////////
    std::string path = "/dev/gpiochip" + id_;
    asio::error_code ec;

    fd_.assign(::open(path.data(), O_RDWR | O_CLOEXEC), ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Error opening file " + path + " - " + ec.message()
    );

    io_cmd<
        gpiochip_info,
        GPIO_GET_CHIPINFO_IOCTL
    > cmd = { };

    fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Error getting chip info - " + ec.message()
    );

    name_ = cmd.data_.label;

    for(gpio::pos n = 0; n < cmd.data_.lines; ++n)
        pins_.emplace_back(new generic::pin(io, this, n));
}

////////////////////////////////////////////////////////////////////////////////
chip::~chip()
{
    pins_.clear();

    asio::error_code ec;
    fd_.close(ec);
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
unique_chip get_chip(asio::io_service& io, std::string param)
{
    return std::make_unique<generic::chip>(io, std::move(param));
}

////////////////////////////////////////////////////////////////////////////////
}
