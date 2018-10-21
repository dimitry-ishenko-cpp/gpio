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

#include <asio.hpp>
#include <chrono>
#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include <string>

#include <linux/gpio.h>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{
namespace generic
{

////////////////////////////////////////////////////////////////////////////////
pin::pin(asio::io_service& io, generic::chip* chip, gpio::pos n) :
    pin_base(chip, n), fd_(io), buffer_(sizeof(gpioevent_data))
{
    valid_modes_ = { in, out };
    valid_flags_ = { active_low, open_drain, open_source };

    get_info();
}

////////////////////////////////////////////////////////////////////////////////
pin::~pin() { detach(); }

////////////////////////////////////////////////////////////////////////////////
void pin::mode(gpio::mode mode, gpio::flag flags, gpio::state state)
{
    detach();

    std::uint32_t value = 0;
    for(auto flag: valid_flags_)
        if(flag & flags)
        {
            switch(flag)
            {
            case active_low : value |= GPIOHANDLE_REQUEST_ACTIVE_LOW ; break;
            case open_drain : value |= GPIOHANDLE_REQUEST_OPEN_DRAIN ; break;
            case open_source: value |= GPIOHANDLE_REQUEST_OPEN_SOURCE; break;
            default:;
            }
            flags &= ~flag;
        }

    if(flags) throw std::invalid_argument(
        type_id(this) + ": Cannot set pin mode - Invalid flag(s): " + std::to_string(flags)
    );

    switch(mode)
    {
    case in:
        mode_in(value);
        break;

    case out:
        mode_out(value, state);
        pin_base::set(state);
        break;

    default:
        throw std::invalid_argument(
            type_id(this) + ": Cannot set pin mode - Invalid mode: " + std::to_string(mode)
        );
    }

    get_info();
}

////////////////////////////////////////////////////////////////////////////////
void pin::detach()
{
    if(!is_detached())
    {
        pwm_stop();

        fd_.close();
        get_info();
    }
}

////////////////////////////////////////////////////////////////////////////////
void pin::set(gpio::state state)
{
    if(is_detached()) throw std::logic_error(
        type_id(this) + ": Cannot set pin state - Detached instance"
    );

    pin_base::set(state);
    sync_state();
}

////////////////////////////////////////////////////////////////////////////////
gpio::state pin::state()
{
    if(is_detached()) throw std::logic_error(
        type_id(this) + ": Cannot get pin state - Detached instance"
    );

    io_cmd<gpiohandle_data, GPIOHANDLE_GET_LINE_VALUES_IOCTL> cmd = { };
    asio::error_code ec;

    fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Cannot get pin state - " + ec.message()
    );

    return cmd.data_.values[0] ? on : off;
}

////////////////////////////////////////////////////////////////////////////////
void pin::period(nsec period)
{
    if(is_detached()) throw std::logic_error(
        type_id(this) + ": Cannot set pin period - Detached instance"
    );

    pin_base::period(period);
    sync_state();
}

void pin::pulse(nsec pulse)
{
    if(is_detached()) throw std::logic_error(
        type_id(this) + ": Cannot set pin pulse - Detached instance"
    );

    pin_base::pulse(pulse);
    sync_state();
}

////////////////////////////////////////////////////////////////////////////////
void pin::get_info()
{
    io_cmd<gpioline_info, GPIO_GET_LINEINFO_IOCTL> cmd = { };
    asio::error_code ec;

    cmd.data_.line_offset = static_cast<__u32>(pos_);

    static_cast<generic::chip*>(chip_)->fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Cannot get pin info - " + ec.message()
    );

    name_ = cmd.data_.name;
    mode_ = cmd.data_.flags & GPIOLINE_FLAG_IS_OUT ? out : in;

    flags_ = { };
    if(cmd.data_.flags & GPIOLINE_FLAG_ACTIVE_LOW ) flags_ |= active_low;
    if(cmd.data_.flags & GPIOLINE_FLAG_OPEN_DRAIN ) flags_ |= open_drain;
    if(cmd.data_.flags & GPIOLINE_FLAG_OPEN_SOURCE) flags_ |= open_source;

    used_ = cmd.data_.flags & GPIOLINE_FLAG_KERNEL;
}

////////////////////////////////////////////////////////////////////////////////
void pin::mode_in(uint32_t flags)
{
    io_cmd<gpioevent_request, GPIO_GET_LINEEVENT_IOCTL> cmd = { };
    asio::error_code ec;

    cmd.data_.lineoffset  = static_cast<__u32>(pos_);
    cmd.data_.handleflags = GPIOHANDLE_REQUEST_INPUT | flags;
    cmd.data_.eventflags  = GPIOEVENT_REQUEST_BOTH_EDGES;
    std::strcpy(cmd.data_.consumer_label, type_id(this).data());

    static_cast<generic::chip*>(chip_)->fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Cannot set pin mode - " + ec.message()
    );

    fd_.assign(cmd.data_.fd);
    sched_read();
}

////////////////////////////////////////////////////////////////////////////////
void pin::mode_out(uint32_t flags, gpio::state state)
{
    io_cmd<gpiohandle_request, GPIO_GET_LINEHANDLE_IOCTL> cmd = { };
    asio::error_code ec;

    cmd.data_.lineoffsets[0]    = static_cast<__u32>(pos_);
    cmd.data_.flags             = GPIOHANDLE_REQUEST_OUTPUT | flags;
    cmd.data_.default_values[0] = state;
    std::strcpy(cmd.data_.consumer_label, type_id(this).data());
    cmd.data_.lines = 1;

    static_cast<generic::chip*>(chip_)->fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Cannot set pin mode - " + ec.message()
    );

    fd_.assign(cmd.data_.fd);
}

////////////////////////////////////////////////////////////////////////////////
void pin::state(gpio::state state)
{
    io_cmd<gpiohandle_data, GPIOHANDLE_SET_LINE_VALUES_IOCTL> cmd = { };
    asio::error_code ec;

    cmd.data_.values[0] = state;
    fd_.io_control(cmd, ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Cannot set pin state - " + ec.message()
    );
}

////////////////////////////////////////////////////////////////////////////////
void pin::sched_read()
{
    asio::async_read(fd_, asio::buffer(buffer_),
        [&](const asio::error_code& ec, std::size_t)
        {
            if(ec) return;

            auto ev = reinterpret_cast<gpioevent_data*>(buffer_.data());
            auto state = ev->id == GPIOEVENT_EVENT_RISING_EDGE ? on : off;

            state_changed_(state);
            sched_read();
        }
    );
}

////////////////////////////////////////////////////////////////////////////////
void pin::sync_state()
{
    if(pulse_ == period_)
    {
        // no need for pwm - set state directly
        pwm_stop();
        state(on);
    }
    else if(pulse_ == 0ns)
    {
        // no need for pwm - set state directly
        pwm_stop();
        state(off);
    }
    else
    {
        // start/update pwm
        high_ticks_= pulse_.count();
        low_ticks_ = (period_ - pulse_).count();

        if(!pwm_started()) pwm_start();
    }
}

////////////////////////////////////////////////////////////////////////////////
void pin::pwm_start()
{
    stop_ = false;
    pwm_ = std::async(std::launch::async, [&]()
    {
        for(auto tp = std::chrono::high_resolution_clock::now();;)
        {
            state(on);
            tp += nsec(high_ticks_);
            std::this_thread::sleep_until(tp);
            if(stop_) break;

            state(off);
            tp += nsec(low_ticks_);
            std::this_thread::sleep_until(tp);
            if(stop_) break;
        }
    });
}

////////////////////////////////////////////////////////////////////////////////
void pin::pwm_stop()
{
    if(pwm_started())
    {
        stop_ = true;
        pwm_.get();
    }
}

////////////////////////////////////////////////////////////////////////////////
}
}
