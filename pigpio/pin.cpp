////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "chip.hpp"
#include "pin.hpp"
#include "type_id.hpp"

#include <algorithm>
#include <asio.hpp>
#include <limits>

#include <fcntl.h>
#include <pigpio.h>
#include <sys/types.h>
#include <sys/stat.h>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{
namespace pigpio
{

////////////////////////////////////////////////////////////////////////////////
pin::pin(asio::io_service& io, pigpio::chip* chip, gpio::pos n) :
    pin_base(chip, n), fd_(io), buffer_(sizeof(gpioReport_t))
{
    valid_modes_ = { in, out };
    valid_flags_ = { pull_up, pull_down };
}

////////////////////////////////////////////////////////////////////////////////
pin::~pin() { detach(); }

////////////////////////////////////////////////////////////////////////////////
void pin::mode(gpio::mode mode, gpio::flag flag, gpio::state state)
{
    this->mode(mode, flag);
    if(mode == out) set(state);
}

////////////////////////////////////////////////////////////////////////////////
void pin::mode(gpio::mode mode, gpio::flag flag)
{
    detach();

    switch(mode)
    {
    case in:
        if(gpioSetMode(to_gpio(), PI_INPUT) < 0) throw std::runtime_error(
            type_id(this) + ": Cannot set pin as input"
        );
        break;

    case out:
        if(gpioSetMode(to_gpio(), PI_OUTPUT) < 0) throw std::runtime_error(
            type_id(this) + ": Cannot set pin as output"
        );
        break;

    default:
        throw std::invalid_argument(
            type_id(this) + ": Cannot set pin mode - Invalid mode: " + std::to_string(mode)
        );
    }

    ////////////////////
    switch(flag)
    {
    case pull_up:
        if(gpioSetPullUpDown(to_gpio(), PI_PUD_UP) < 0) throw std::runtime_error(
            type_id(this) + ": Cannot enable pull-up resistor"
        );
        break;

    case pull_down:
        if(gpioSetPullUpDown(to_gpio(), PI_PUD_DOWN) < 0) throw std::runtime_error(
            type_id(this) + ": Cannot enable pull-down resistor"
        );
        break;

    default:
        if(flag) throw std::invalid_argument(
            type_id(this) + ": Cannot set pin mode - Invalid flag: " + std::to_string(flag)
        );
    }
    flags_ = flag;

    if(mode == in) attach();
}

////////////////////////////////////////////////////////////////////////////////
gpio::mode pin::mode() const noexcept
{
    auto value = gpioGetMode(to_gpio());
    return value == PI_INPUT ? in :
           value == PI_OUTPUT ? out : detached;
}

////////////////////////////////////////////////////////////////////////////////
void pin::detach()
{
    if(!is_detached()) fd_.close();

    if(handle_ >= 0)
    {
        if(gpioNotifyClose(static_cast<unsigned>(handle_)) < 0)
            throw std::runtime_error(
                type_id(this) + ": Cannot close notification handle"
            );
        handle_ = -1;
    }
}

////////////////////////////////////////////////////////////////////////////////
void pin::set(gpio::state state)
{
    if(gpioWrite(to_gpio(), state) < 0) throw std::runtime_error(
        type_id(this) + ": Cannot set pin state"
    );
}

////////////////////////////////////////////////////////////////////////////////
gpio::state pin::state()
{
    auto value = gpioRead(to_gpio());
    if(value < 0) throw std::runtime_error(
        type_id(this) + ": Cannot get pin state"
    );

    return value ? on : off;
}

////////////////////////////////////////////////////////////////////////////////
void pin::period(nsec period)
{
    // prevent frequency overflow
    period = std::max(period, 10000ns);

    auto freq = nsec::period::den / (nsec::period::num * period.count());
    if(gpioSetPWMfrequency(to_gpio(), static_cast<unsigned>(freq)) < 0)
        throw std::runtime_error(
            type_id(this) + ": Cannot set PWM frequency"
        );

    if(gpioSetPWMrange(to_gpio(), PI_MAX_DUTYCYCLE_RANGE) < 0)
        throw std::runtime_error(
            type_id(this) + ": Cannot set PWM range"
        );

    get_pwm();
}

////////////////////////////////////////////////////////////////////////////////
void pin::pulse(nsec pulse)
{
    pulse = std::min(std::max(pulse, 0ns), period_);

    auto cycle = pulse * PI_MAX_DUTYCYCLE_RANGE / period_;
    if(gpioPWM(to_gpio(), static_cast<unsigned>(cycle)) < 0)
        throw std::runtime_error(
            type_id(this) + ": Cannot set PWM duty cycle"
        );

    pulse_ = period_ * cycle / PI_MAX_DUTYCYCLE_RANGE;
}

////////////////////////////////////////////////////////////////////////////////
void pin::attach()
{
    handle_ = gpioNotifyOpen();
    if(handle_ < 0) throw std::invalid_argument(
        type_id(this) + ": Cannot get notification handle"
    );

    ////////////////////
    std::string path = "/dev/pigpio" + std::to_string(handle_);
    asio::error_code ec;

    fd_.assign(::open(path.data(), O_RDONLY | O_CLOEXEC), ec);
    if(ec) throw std::runtime_error(
        type_id(this) + ": Error opening file " + path + " - " + ec.message()
    );

    if(gpioNotifyBegin(static_cast<unsigned>(handle_), 1 << pos_) < 0)
        throw std::runtime_error(
            type_id(this) + ": Cannot start notification"
        );
    sched_read();
}

////////////////////////////////////////////////////////////////////////////////
void pin::sched_read()
{
    asio::async_read(fd_, asio::buffer(buffer_),
        [&](const asio::error_code& ec, std::size_t)
        {
            if(ec) return;

            auto ev = reinterpret_cast<gpioReport_t*>(buffer_.data());
            auto state = ev->level ? on : off;

            state_changed_(state);
            sched_read();
        }
    );
}

////////////////////////////////////////////////////////////////////////////////
void pin::get_pwm()
{
    auto freq = gpioGetPWMfrequency(to_gpio());
    if(freq < 0) throw std::runtime_error(
        type_id(this) + ": Cannot get PWM frequency"
    );
    period_ = nsec(nsec::period::den / (nsec::period::num * freq));

    auto cycle = gpioGetPWMdutycycle(to_gpio());
    if(cycle < 0) throw std::runtime_error(
        type_id(this) + ": Cannot get PWM duty cycle"
    );
    pulse_ = period_ * cycle / PI_MAX_DUTYCYCLE_RANGE;
}

////////////////////////////////////////////////////////////////////////////////
}
}
