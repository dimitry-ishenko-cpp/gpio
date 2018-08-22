////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "gpiod_chip.hpp"
#include "gpiod_pin.hpp"
#include "posix/error.hpp"

#include <cstring>
#include <stdexcept>

#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
gpiod_pin::gpiod_pin(gpiod_chip* chip, gpio::pos n) :
    pin_base(chip, n)
{
    modes_ = { gpio::digital_in, gpio::digital_out };
    can_flags_ = { gpio::active_low, gpio::open_drain, gpio::open_source };

    update();
}

gpiod_pin::~gpiod_pin() { detach(); }

////////////////////////////////////////////////////////////////////////////////
namespace
{

auto add_from(gpio::flags flags, gpio::flags valid)
{
    __u32 fl = 0;
    for(auto flag: valid)
        if(flags.count(flag))
        {
            switch(flag)
            {
            case gpio::active_low : fl |= GPIOHANDLE_REQUEST_ACTIVE_LOW ; break;
            case gpio::open_drain : fl |= GPIOHANDLE_REQUEST_OPEN_DRAIN ; break;
            case gpio::open_source: fl |= GPIOHANDLE_REQUEST_OPEN_SOURCE; break;
            default:;
            }
            flags.erase(flag);
        }

    if(flags.size()) throw std::string(
        "Invalid pin mode flag " + std::to_string(*flags.begin())
    );

    return fl;
};

}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::mode(gpio::mode mode, gpio::flags flags, gpio::value value)
{
    detach();

    __u32 fl = 0;
    try
    {
        switch(mode)
        {
        case gpio::digital_in:
            fl = GPIOHANDLE_REQUEST_INPUT | add_from(flags, { gpio::active_low });
            break;

        case gpio::digital_out:
            fl = GPIOHANDLE_REQUEST_OUTPUT | add_from(flags,
                { gpio::active_low, gpio::open_drain, gpio::open_source }
            );
            break;

        default: throw std::string("Invalid pin mode " + std::to_string(mode));
        }
    }
    catch(std::string& msg)
    {
        throw std::invalid_argument(type_id() + ": " + msg);
    }

    gpiohandle_request req = { };
    req.lineoffsets[0]    = static_cast<__u32>(pos_);
    req.flags             = fl;
    req.default_values[0] = static_cast<__u8>(!!value);
    std::strcpy(req.consumer_label, type_id().data());
    req.lines = 1;

    auto status = ::ioctl(
        static_cast<gpiod_chip*>(chip_)->fd_, GPIO_GET_LINEHANDLE_IOCTL, &req
    );
    if(status == -1 || req.fd <= 0) throw posix::errno_error(
        type_id() + ": Error getting pin handle"
    );
    fd_ = req.fd;

    update();
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::detach()
{
    if(fd_ != invalid)
    {
        ::close(fd_);
        fd_ = invalid;

        update();
    }
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::value(gpio::value value)
{
    throw_detached();

    gpiohandle_data data = { };
    data.values[0] = static_cast<__u8>(!!value);

    auto status = ::ioctl(fd_, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
    if(status == -1) throw posix::errno_error(
        type_id() + ": Error setting pin value " + std::to_string(value)
    );
}

////////////////////////////////////////////////////////////////////////////////
gpio::value gpiod_pin::value()
{
    throw_detached();

    gpiohandle_data data = { };

    auto status = ::ioctl(fd_, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
    if(status == -1) throw posix::errno_error(
        type_id() + ": Error getting pin value"
    );

    return data.values[0];
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::update()
{
    gpioline_info info = { };
    info.line_offset = static_cast<__u32>(pos_);

    auto status = ::ioctl(
        static_cast<gpiod_chip*>(chip_)->fd_, GPIO_GET_LINEINFO_IOCTL, &info
    );
    if(status == -1) throw posix::errno_error(
        type_id() + ": Error getting pin info"
    );

    name_ = info.name;
    mode_ = info.flags & GPIOLINE_FLAG_IS_OUT ? gpio::digital_out : gpio::digital_in;

    flags_.clear();
    if(info.flags & GPIOLINE_FLAG_ACTIVE_LOW ) flags_.insert(gpio::active_low);
    if(info.flags & GPIOLINE_FLAG_OPEN_DRAIN ) flags_.insert(gpio::open_drain);
    if(info.flags & GPIOLINE_FLAG_OPEN_SOURCE) flags_.insert(gpio::open_source);

    used_ = info.flags & GPIOLINE_FLAG_KERNEL;
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::throw_detached() const
{
    if(detached()) throw std::logic_error(
        type_id() + ": Using detached instance"
    );
}

////////////////////////////////////////////////////////////////////////////////
}
