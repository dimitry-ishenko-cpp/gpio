////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "command.hpp"
#include "gpiod_chip.hpp"
#include "gpiod_pin.hpp"
#include "type_id.hpp"

#include <stdexcept>
#include <utility>

#include <linux/gpio.h>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
gpiod_chip::gpiod_chip(asio::io_context& io, std::string id) :
    chip_base("gpiod"), fd_(io)
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

    gpio::command<
        gpiochip_info,
        GPIO_GET_CHIPINFO_IOCTL
    > cmd = { };

    fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Error getting chip info - " + ec.message()
    );

    name_ = cmd.get().label;

    for(gpio::pos n = 0; n < cmd.get().lines; ++n)
        pins_.emplace_back(new gpiod_pin(this, n));
}

gpiod_chip::~gpiod_chip()
{
    pins_.clear();

    asio::error_code ec;
    fd_.close(ec);
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
extern "C" gpio::chip* create_chip(asio::io_context& io, std::string id)
{ return new gpio::gpiod_chip(io, std::move(id)); }
extern "C" void delete_chip(gpio::chip* chip) { if(chip) delete chip; }
