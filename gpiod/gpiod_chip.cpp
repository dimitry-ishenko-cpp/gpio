////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "gpiod_chip.hpp"
#include "gpiod_pin.hpp"
#include "posix/error.hpp"
#include "type_id.hpp"

#include <stdexcept>
#include <utility>

#include <fcntl.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
gpiod_chip::gpiod_chip(std::string id) : chip_base("gpiod")
{
    if(id.find_first_not_of("0123456789") != std::string::npos
        || id.size() < 1 || id.size() > 3)
    throw std::invalid_argument(
        type_id(this) + ": Missing or invalid id " + id
    );

    id_ = std::move(id);

    ////////////////////
    std::string path = "/dev/gpiochip" + id_;
    gpiochip_info info = { };

    fd_ = ::open(path.data(), O_RDWR | O_CLOEXEC);
    if(!fd_) throw posix::errno_error(
        type_id(this) + ": Error opening file " + path
    );

    auto status = ::ioctl(fd_, GPIO_GET_CHIPINFO_IOCTL, &info);
    if(status == -1) throw posix::errno_error(
        type_id(this) + ": Error getting chip info"
    );

    name_ = info.label;

    for(gpio::pos n = 0; n < info.lines; ++n)
        pins_.emplace_back(new gpiod_pin(this, n));
}

gpiod_chip::~gpiod_chip()
{
    pins_.clear();
    ::close(fd_);
}

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
extern "C" gpio::chip* create_chip(std::string id)
{ return new gpio::gpiod_chip(std::move(id)); }
extern "C" void delete_chip(gpio::chip* chip) { if(chip) delete chip; }
