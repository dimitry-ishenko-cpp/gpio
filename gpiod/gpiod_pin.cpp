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

#include <chrono>
#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include <string>

#include <linux/gpio.h>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
gpiod_pin::gpiod_pin(gpiod_chip* chip, gpio::pos n) :
    pin_base(chip, n), fd_(chip->fd_.get_executor().context())
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

    gpio::command<
        gpiohandle_request,
        GPIO_GET_LINEHANDLE_IOCTL
    > cmd = { };
    asio::error_code ec;

    cmd.get().lineoffsets[0]    = static_cast<__u32>(pos_);
    cmd.get().flags             = fl;
    cmd.get().default_values[0] = state;
    std::strcpy(cmd.get().consumer_label, type_id(this).data());
    cmd.get().lines = 1;

    static_cast<gpiod_chip*>(chip_)->fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Error getting pin handle - " + ec.message()
    );
    fd_.assign(cmd.get().fd);

    update();
    if(mode == gpio::pwm) start_pwm();
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::detach()
{
    if(fd_.is_open())
    {
        stop_pwm();

        fd_.close();
        update();
    }
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::set(gpio::state state)
{
    throw_detached();
    throw_pwm();

    gpio::command<
        gpiohandle_data,
        GPIOHANDLE_SET_LINE_VALUES_IOCTL
    > cmd = { };
    asio::error_code ec;

    cmd.get().values[0] = state;

    fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Error setting pin value - " + ec.message()
    );
}

////////////////////////////////////////////////////////////////////////////////
gpio::state gpiod_pin::state()
{
    throw_detached();
    throw_pwm();

    gpio::command<
        gpiohandle_data,
        GPIOHANDLE_GET_LINE_VALUES_IOCTL
    > cmd = { };
    asio::error_code ec;

    fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Error getting pin value - " + ec.message()
    );

    return cmd.get().values[0];
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::period(gpio::nsec period)
{
    pin_base::period(period);
    high_ticks_ = pulse_.count();
    low_ticks_ = (period_ - pulse_).count();
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::set(gpio::nsec pulse)
{
    pin_base::set(pulse);
    high_ticks_= pulse_.count();
    low_ticks_ = (period_ - pulse_).count();
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::update()
{
    gpio::command<
        gpioline_info,
        GPIO_GET_LINEINFO_IOCTL
    > cmd = { };
    asio::error_code ec;

    cmd.get().line_offset = static_cast<__u32>(pos_);

    static_cast<gpiod_chip*>(chip_)->fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Error getting pin info - " + ec.message()
    );

    name_ = cmd.get().name;
    mode_ = cmd.get().flags & GPIOLINE_FLAG_IS_OUT ? gpio::digital_out : gpio::digital_in;

    flags_ = { };
    if(cmd.get().flags & GPIOLINE_FLAG_ACTIVE_LOW ) flags_ |= gpio::active_low;
    if(cmd.get().flags & GPIOLINE_FLAG_OPEN_DRAIN ) flags_ |= gpio::open_drain;
    if(cmd.get().flags & GPIOLINE_FLAG_OPEN_SOURCE) flags_ |= gpio::open_source;

    used_ = cmd.get().flags & GPIOLINE_FLAG_KERNEL;
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::throw_detached() const
{
    if(detached()) throw std::logic_error(
        type_id(this) + ": Cannot get/set pin state - Detached instance"
    );
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::throw_pwm() const
{
    if(mode() == gpio::pwm) throw std::logic_error(
        type_id(this) + ": Cannot get/set pin state - PWM in-progress"
    );
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::start_pwm()
{
    stop_ = false;
    thread_ = std::thread([&]
    {
        gpio::command<
            gpiohandle_data,
            GPIOHANDLE_SET_LINE_VALUES_IOCTL
        > cmd = { };
        asio::error_code ec;

        for(auto tp = std::chrono::high_resolution_clock::now();;)
        {
            if(high_ticks_)
            {
                cmd.get().values[0] = true;
                fd_.io_control(cmd, ec);

                tp += gpio::nsec(high_ticks_);
                std::this_thread::sleep_until(tp);
            }
            if(stop_) break;

            if(low_ticks_)
            {
                cmd.get().values[0] = false;
                fd_.io_control(cmd, ec);

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
