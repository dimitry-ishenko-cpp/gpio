////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "gpiod_chip.hpp"
#include "posix/error.hpp"

#include <stdexcept>

#include <fcntl.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
gpiod_chip::gpiod_chip(std::string param)
{
    constexpr auto npos = std::string::npos;

    // discard any extra parameters
    auto pos = param.find(':');
    if(pos != npos) param.erase(pos);

    if(param.find_first_not_of("0123456789") != npos
    || param.size() < 1 || param.size() > 3)
    throw std::invalid_argument("Missing or invalid gpiod chip id " + param);

    id_ = std::move(param);

    ////////////////////
    std::string path = "/dev/gpiochip" + id_;
    gpiochip_info info = { };

    res_.adopt(::open(path.data(), O_RDWR | O_CLOEXEC));
    if(!res_) throw posix::errno_error("Error opening chip " + path);

    auto status = ::ioctl(res_, GPIO_GET_CHIPINFO_IOCTL, &info);
    if(status == -1) throw posix::errno_error("Error getting chip info");

    name_ = info.label;
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
extern "C" gpio::chip* create_chip(const char* param) { return new gpio::gpiod_chip(param); }
extern "C" void delete_chip(gpio::chip* chip) { if(chip) delete chip; }