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
    virtual void mode(gpio::mode, gpio::flags, gpio::value) = 0;
    virtual void mode(gpio::mode, gpio::flags) = 0;
    virtual void mode(gpio::mode, gpio::value) = 0;
    virtual void mode(gpio::mode) = 0 ;

    virtual bool digital() const noexcept = 0;
    virtual bool analog() const noexcept = 0;

    virtual bool input() const noexcept = 0;
    virtual bool output() const noexcept = 0;

    virtual bool supports(gpio::mode) const noexcept = 0;

    virtual bool is(gpio::flag) const noexcept = 0;
    virtual bool supports(gpio::flag) const noexcept = 0;

    ////////////////////
    virtual void detach() = 0;
    virtual bool detached() const noexcept = 0;

    // pin is in-use eg by the kernel
    virtual bool used() const noexcept = 0;

    ////////////////////
    virtual void value(gpio::value) = 0;
    virtual gpio::value value() = 0;

    // pwm
    virtual void period(gpio::usec) = 0;
    virtual gpio::usec period() const noexcept = 0;

    virtual void pulse(gpio::usec) = 0;
    virtual gpio::usec pulse() const noexcept = 0;

    virtual void pulse(gpio::percent) = 0;
    virtual gpio::percent duty_cycle() const noexcept = 0;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
