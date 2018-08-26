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

#include <chrono>
#include <cstring>
#include <initializer_list>
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
    modes_ = { gpio::digital_in, gpio::digital_out, gpio::pwm };
    valid_ = gpio::active_low | gpio::open_drain | gpio::open_source;

    update();
}

gpiod_pin::~gpiod_pin() { detach(); }

////////////////////////////////////////////////////////////////////////////////
gpio::mode gpiod_pin::mode() const noexcept
{ return thread_.joinable() ? gpio::pwm : mode_; }

////////////////////////////////////////////////////////////////////////////////
namespace
{

auto convert(gpio::flag flags, std::initializer_list<gpio::flag> valid)
{
    __u32 fl = 0;
    for(auto flag: valid)
        if(flags & flag)
        {
            switch(flag)
            {
            case gpio::active_low : fl |= GPIOHANDLE_REQUEST_ACTIVE_LOW ; break;
            case gpio::open_drain : fl |= GPIOHANDLE_REQUEST_OPEN_DRAIN ; break;
            case gpio::open_source: fl |= GPIOHANDLE_REQUEST_OPEN_SOURCE; break;
            default:;
            }
            flags &= ~flag;
        }

    if(flags) throw std::string(
        "Invalid pin mode flag(s) " + std::to_string(flags)
    );

    return fl;
};

}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::mode(gpio::mode mode, gpio::flag flags, gpio::state state)
{
    detach();

    __u32 fl = 0;
    try
    {
        switch(mode)
        {
        case gpio::digital_in:
            fl = GPIOHANDLE_REQUEST_INPUT | convert(flags, { gpio::active_low });
            break;

        case gpio::digital_out:
        case gpio::pwm:
            fl = GPIOHANDLE_REQUEST_OUTPUT | convert(flags,
                { gpio::active_low, gpio::open_drain, gpio::open_source }
            );
            break;

        default: throw std::string("Invalid pin mode " + std::to_string(mode));
        }
    }
    catch(std::string& msg)
    {
        throw std::invalid_argument(type_id(this) + ": " + msg);
    }

    gpiohandle_request req = { };
    req.lineoffsets[0]    = static_cast<__u32>(pos_);
    req.flags             = fl;
    req.default_values[0] = state;
    std::strcpy(req.consumer_label, type_id(this).data());
    req.lines = 1;

    auto status = ::ioctl(
        static_cast<gpiod_chip*>(chip_)->fd_, GPIO_GET_LINEHANDLE_IOCTL, &req
    );
    if(status == -1 || req.fd <= 0) throw posix::errno_error(
        type_id(this) + ": Error getting pin handle"
    );
    fd_ = req.fd;

    update();
    if(mode == gpio::pwm) start_pwm();
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::detach()
{
    if(fd_ != invalid)
    {
        stop_pwm();

        ::close(fd_);
        fd_ = invalid;

        update();
    }
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::set(gpio::state state)
{
    throw_detached();

    gpiohandle_data data = { };
    data.values[0] = state;

    auto status = ::ioctl(fd_, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
    if(status == -1) throw posix::errno_error(
        type_id(this) + ": Error setting pin value " + std::to_string(state)
    );
}

////////////////////////////////////////////////////////////////////////////////
gpio::state gpiod_pin::state()
{
    throw_detached();

    gpiohandle_data data = { };

    auto status = ::ioctl(fd_, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
    if(status == -1) throw posix::errno_error(
        type_id(this) + ": Error getting pin value"
    );

    return data.values[0];
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::period(gpio::nsec period)
{
    pin_base::period(period);
    high_ticks_ = pulse_.count();
    low_ticks_ = (period_ - pulse_).count();
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::pulse(gpio::nsec pulse)
{
    pin_base::pulse(pulse);
    high_ticks_= pulse.count();
    low_ticks_ = (period_ - pulse_).count();
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
        type_id(this) + ": Error getting pin info"
    );

    name_ = info.name;
    mode_ = info.flags & GPIOLINE_FLAG_IS_OUT ? gpio::digital_out : gpio::digital_in;

    flags_ = { };
    if(info.flags & GPIOLINE_FLAG_ACTIVE_LOW ) flags_ |= gpio::active_low;
    if(info.flags & GPIOLINE_FLAG_OPEN_DRAIN ) flags_ |= gpio::open_drain;
    if(info.flags & GPIOLINE_FLAG_OPEN_SOURCE) flags_ |= gpio::open_source;

    used_ = info.flags & GPIOLINE_FLAG_KERNEL;
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::throw_detached() const
{
    if(detached()) throw std::logic_error(
        type_id(this) + ": Using detached instance"
    );
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::start_pwm()
{
    stop_ = false;
    thread_ = std::thread([&]
    {
        for(auto tp = std::chrono::high_resolution_clock::now();;)
        {
            if(high_ticks_)
            {
                set();
                tp += gpio::nsec(high_ticks_);
                std::this_thread::sleep_until(tp);
            }
            if(stop_) break;

            if(low_ticks_)
            {
                reset();
                tp += gpio::nsec(low_ticks_);
                std::this_thread::sleep_until(tp);
            }
            if(stop_) break;
        }
    });
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::stop_pwm()
{
    if(thread_.joinable())
    {
        stop_ = true;
        thread_.join();
    }
}

////////////////////////////////////////////////////////////////////////////////
}
