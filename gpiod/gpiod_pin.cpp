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
    valid_ = { gpio::active_low, gpio::open_drain, gpio::open_source };

    update();
}

gpiod_pin::~gpiod_pin() { detach(); }

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::mode(gpio::mode mode, gpio::flag flags, gpio::state state)
{
    detach();

    std::uint32_t value = 0;
    for(auto flag: valid_) if(flag & flags)
    {
        switch(flag)
        {
        case gpio::active_low : value |= GPIOHANDLE_REQUEST_ACTIVE_LOW ; break;
        case gpio::open_drain : value |= GPIOHANDLE_REQUEST_OPEN_DRAIN ; break;
        case gpio::open_source: value |= GPIOHANDLE_REQUEST_OPEN_SOURCE; break;
        default:;
        }
        flags &= ~flag;
    }

    if(flags) throw std::invalid_argument(
        type_id(this) + ": Cannot set pin mode - Invalid flag(s) " + std::to_string(flags)
    );

    switch(mode)
    {
    case gpio::digital_in:
        mode_digital_in(value);
        break;

    case gpio::digital_out:
    case gpio::pwm:
        mode_digital_out(value, state);
        break;

    default:
        throw std::invalid_argument(
            type_id(this) + ": Cannot set pin mode - Invalid mode " + std::to_string(mode)
        );
    }

    update();
    if(mode == gpio::pwm) pwm_start();
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::detach()
{
    if(!detached())
    {
        if(pwm_started()) pwm_stop();

        fd_.close();
        update();
    }
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::set(gpio::state state)
{
    if(detached()) throw std::logic_error(
        type_id(this) + ": Cannot set pin state - Detached instance"
    );

    gpio::command<
        gpiohandle_data,
        GPIOHANDLE_SET_LINE_VALUES_IOCTL
    > cmd = { };
    asio::error_code ec;

    cmd.get().values[0] = state;

    fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Cannot set pin state - " + ec.message()
    );
}

////////////////////////////////////////////////////////////////////////////////
gpio::state gpiod_pin::state()
{
    if(detached()) throw std::logic_error(
        type_id(this) + ": Cannot get pin state - Detached instance"
    );

    gpio::command<
        gpiohandle_data,
        GPIOHANDLE_GET_LINE_VALUES_IOCTL
    > cmd = { };
    asio::error_code ec;

    fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Cannot get pin state - " + ec.message()
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
        type_id(this) + ": Cannot get pin info - " + ec.message()
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
void gpiod_pin::mode_digital_in(uint32_t flags)
{
    gpio::command<
        gpiohandle_request,
        GPIO_GET_LINEHANDLE_IOCTL
    > cmd = { };
    asio::error_code ec;

    cmd.get().lineoffsets[0]    = static_cast<__u32>(pos_);
    cmd.get().flags             = GPIOHANDLE_REQUEST_INPUT | flags;
    std::strcpy(cmd.get().consumer_label, type_id(this).data());
    cmd.get().lines = 1;

    static_cast<gpiod_chip*>(chip_)->fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Cannot set pin mode - " + ec.message()
    );

    fd_.assign(cmd.get().fd);
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::mode_digital_out(uint32_t flags, gpio::state state)
{
    gpio::command<
        gpiohandle_request,
        GPIO_GET_LINEHANDLE_IOCTL
    > cmd = { };
    asio::error_code ec;

    cmd.get().lineoffsets[0]    = static_cast<__u32>(pos_);
    cmd.get().flags             = GPIOHANDLE_REQUEST_OUTPUT | flags;
    cmd.get().default_values[0] = state;
    std::strcpy(cmd.get().consumer_label, type_id(this).data());
    cmd.get().lines = 1;

    static_cast<gpiod_chip*>(chip_)->fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Cannot set pin mode - " + ec.message()
    );

    fd_.assign(cmd.get().fd);
}

////////////////////////////////////////////////////////////////////////////////
void gpiod_pin::pwm_start()
{
    stop_ = false;
    pwm_ = std::async(std::launch::async, [&]()
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
void gpiod_pin::pwm_stop()
{
    stop_ = true;
    pwm_.get();
}

////////////////////////////////////////////////////////////////////////////////
}
