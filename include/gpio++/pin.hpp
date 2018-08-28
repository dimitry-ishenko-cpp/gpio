////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_PIN_HPP
#define GPIO_PIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include <gpio++/types.hpp>

#include <set>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace gpio
{

////////////////////////////////////////////////////////////////////////////////
struct chip;

////////////////////////////////////////////////////////////////////////////////
struct pin
{
    virtual ~pin() { }

    ////////////////////
    virtual const gpio::chip* chip() const noexcept = 0;

    virtual gpio::pos pos() const noexcept = 0;
    virtual const std::string& name() const noexcept = 0;

    ////////////////////
    virtual gpio::mode mode() const noexcept = 0;
    virtual void mode(gpio::mode, gpio::flag, gpio::state) = 0;
    virtual void mode(gpio::mode, gpio::flag) = 0;
    virtual void mode(gpio::mode, gpio::state) = 0;
    virtual void mode(gpio::mode) = 0 ;

    virtual bool digital() const noexcept = 0;
    virtual bool analog() const noexcept = 0;

    virtual bool input() const noexcept = 0;
    virtual bool output() const noexcept = 0;

    virtual bool is(gpio::flag) const noexcept = 0;

    virtual bool supports(gpio::mode) const noexcept = 0;
    virtual bool supports(gpio::flag) const noexcept = 0;

    ////////////////////
    virtual void detach() = 0;
    virtual bool detached() const noexcept = 0;

    // pin is in-use eg by the kernel
    virtual bool used() const noexcept = 0;

    ////////////////////
    // digital
    virtual void set(gpio::state = gpio::on) = 0;
    virtual void reset() = 0;
    virtual gpio::state state() = 0;

    // pwm
    virtual void period(gpio::nsec) = 0;
    virtual gpio::nsec period() const noexcept = 0;

    virtual void set(gpio::nsec) = 0;
    virtual gpio::nsec pulse() const noexcept = 0;

    virtual void set(gpio::percent) = 0;
    virtual gpio::percent duty_cycle() const noexcept = 0;

    ////////////////////
    // digital callback
    virtual void on_state_changed(gpio::state_changed) = 0;
    virtual void on_state_on(gpio::state_on) = 0;
    virtual void on_state_off(gpio::state_off) = 0;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
