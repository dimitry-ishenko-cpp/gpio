////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_GPIOD_PIN_HPP
#define GPIO_GPIOD_PIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include "pin_base.hpp"

#include <atomic>
#include <thread>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
class gpiod_chip;

////////////////////////////////////////////////////////////////////////////////
class gpiod_pin : public pin_base
{
public:
    ////////////////////
    gpiod_pin(gpiod_chip*, gpio::pos);
    virtual ~gpiod_pin() override;

    ////////////////////
    virtual gpio::mode mode() const noexcept override;
    virtual void mode(gpio::mode, gpio::flag, gpio::value) override;

    virtual void detach() override;
    virtual bool detached() const noexcept override { return fd_ == invalid; }

    virtual void value(gpio::value) override;
    virtual gpio::value value() override;

    virtual void period(gpio::nsec) override;
    virtual void pulse(gpio::nsec) override;

private:
    ////////////////////
    static constexpr int invalid = -1;
    int fd_ = invalid;

    void update();
    void throw_detached() const;

    ////////////////////
    using ticks = gpio::nsec::rep;
    std::atomic<ticks> high_ticks_ { pulse_.count() };
    std::atomic<ticks> low_ticks_ { period_.count() - high_ticks_ };

    std::thread thread_;
    std::atomic<bool> stop_ { false };

    void start_pwm();
    void stop_pwm();
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
