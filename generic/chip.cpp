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

#include <linux/gpio.h>

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

    name_ = cmd.get().label;

    for(gpio::pos n = 0; n < cmd.get().lines; ++n)
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
}

////////////////////////////////////////////////////////////////////////////////
extern "C" gpio::chip* create_chip(asio::io_service& io, std::string id)
{ return new gpio::generic::chip(io, std::move(id)); }
extern "C" void delete_chip(gpio::chip* chip) { if(chip) delete chip; }
