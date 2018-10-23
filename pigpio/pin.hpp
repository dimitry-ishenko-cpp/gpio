////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_PIGPIO_PIN_HPP
#define GPIO_PIGPIO_PIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include "pin_base.hpp"

#include <asio/io_service.hpp>
#include <asio/posix/stream_descriptor.hpp>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{
namespace pigpio
{

////////////////////////////////////////////////////////////////////////////////
class chip;

////////////////////////////////////////////////////////////////////////////////
class pin : public pin_base
{
public:
    ////////////////////
    pin(asio::io_service&, pigpio::chip*, gpio::pos);
    virtual ~pin() override;

    ////////////////////
    virtual void mode(gpio::mode, gpio::flag, gpio::state) override;
    virtual void mode(gpio::mode, gpio::flag) override;
    virtual gpio::mode mode() const noexcept override;

    virtual void detach() override;
    virtual bool is_detached() const noexcept override { return !fd_.is_open(); }

    ////////////////////
    // digital
    virtual void set(gpio::state = on) override;
    virtual gpio::state state() override;

    // pwm
    virtual void period(nsec) override;
    virtual void pulse(nsec) override;

private:
    ////////////////////
    int handle_ = -1;
    asio::posix::stream_descriptor fd_;

    void attach();

    std::vector<char> buffer_;
    void sched_read();

    ////////////////////
    auto to_gpio() const noexcept { return static_cast<unsigned>(pos_); }
    void get_pwm();
};

////////////////////////////////////////////////////////////////////////////////
}
}

////////////////////////////////////////////////////////////////////////////////
#endif
