////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "gpiod_pin.hpp"
#include "posix/error.hpp"

#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#include <linux/gpio.h>
#include <sys/ioctl.h>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
gpiod_pin::gpiod_pin(std::string type, gpio::pos pos, posix::resource chip) :
    pin(std::move(type), pos), chip_(std::move(chip))
{
    update();
}

////////////////////////////////////////////////////////////////////////////////
gpiod_pin::~gpiod_pin() { detach(); }

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::mode(gpio::mode mode, gpio::flag in, gpio::value value)
{
    ////////////////////
    auto add_from = [&](gpio::flag in, std::initializer_list<gpio::flag> valid)
    {
        __u32 out = 0;
        for(auto one: valid)
            if(in & one)
            {
                switch(one)
                {
                case gpio::active_low : out |= GPIOHANDLE_REQUEST_ACTIVE_LOW ; break;
                case gpio::open_drain : out |= GPIOHANDLE_REQUEST_OPEN_DRAIN ; break;
                case gpio::open_source: out |= GPIOHANDLE_REQUEST_OPEN_SOURCE; break;
                default:;
                }
                in = in & ~one;
            }

        if(in) throw std::invalid_argument(
            type_id() + ": Invalid pin mode flag(s) " + std::to_string(in)
        );

        return out;
    };

    ////////////////////
    detach();

    __u32 flags = 0;
    switch(mode)
    {
    case gpio::digital_in:
        flags = GPIOHANDLE_REQUEST_INPUT
            | add_from(in, { gpio::active_low });
        break;

    case gpio::digital_out:
        flags = GPIOHANDLE_REQUEST_OUTPUT
            | add_from(in, { gpio::active_low, gpio::open_drain, gpio::open_source });
        break;

    default:
        throw std::invalid_argument(
            type_id() + ": Invalid pin mode " + std::to_string(mode)
        );
    }

    gpiohandle_request req = { };
    req.lineoffsets[0]    = static_cast<__u32>(pos_);
    req.flags             = flags;
    req.default_values[0] = static_cast<__u8>(!!value);
    std::strcpy(req.consumer_label, type_id().data());
    req.lines = 1;

    auto status = ::ioctl(chip_, GPIO_GET_LINEHANDLE_IOCTL, &req);
    if(status == -1 || req.fd <= 0) throw posix::errno_error(
        type_id() + ": Error getting pin handle"
    );
    res_.adopt(req.fd);

    update();
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::detach()
{
    if(res_)
    {
        res_.close();
        update();
    }
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::value(int value)
{
    throw_detached();

    gpiohandle_data data = { };
    data.values[0] = static_cast<__u8>(!!value);

    auto status = ::ioctl(res_, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
    if(status == -1) throw posix::errno_error(
        type_id() + ": Error setting pin value " + std::to_string(value)
    );
}

////////////////////////////////////////////////////////////////////////////////
int gpiod_pin::value()
{
    throw_detached();

    gpiohandle_data data = { };

    auto status = ::ioctl(res_, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
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

    auto status = ::ioctl(chip_, GPIO_GET_LINEINFO_IOCTL, &info);
    if(status == -1) throw posix::errno_error(
        type_id() + ": Error getting pin info"
    );

    name_ = info.name;
    modes_ = { gpio::digital_in, gpio::digital_out };

    mode_ = info.flags & GPIOLINE_FLAG_IS_OUT ? gpio::digital_out : gpio::digital_in;

    flags_ = static_cast<gpio::flag>(0);
    if(info.flags & GPIOLINE_FLAG_ACTIVE_LOW ) flags_ = flags_ | gpio::active_low;
    if(info.flags & GPIOLINE_FLAG_OPEN_DRAIN ) flags_ = flags_ | gpio::open_drain;
    if(info.flags & GPIOLINE_FLAG_OPEN_SOURCE) flags_ = flags_ | gpio::open_source;

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
